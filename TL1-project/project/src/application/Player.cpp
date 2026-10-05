#define NOMINMAX
#include "Player.h"

#include <algorithm>
#include <cassert>
#include <cmath>
#include <numbers>

#include "Camera.h"
#include "Input.h"
#include "MapChipField.h"
#include "Model.h"
#include "Object3d.h"
#include "Object3dCommon.h"

namespace {

float EaseOut(float start, float end, float t) {
  t = std::clamp(t, 0.0f, 1.0f);
  const float easedT = 1.0f - (1.0f - t) * (1.0f - t);
  return start + (end - start) * easedT;
}

} // namespace

Player::Player() = default;

Player::~Player() = default;

void Player::Initialize(Model *model, Camera *camera, Input *input,
                        MapChipField *mapChipField,
                        const Vector::Vector3 &position) {
  assert(model);
  assert(camera);
  assert(input);
  assert(mapChipField);

  this->model = model;
  this->camera = camera;
  this->input = input;
  this->mapChipField = mapChipField;
  this->position = position;
  respawnPosition = position;
  previousPosition = position;

  velocity = {};
  rotation = {0.0f, std::numbers::pi_v<float> / 2.0f, 0.0f};
  scale = {1.0f, 1.0f, 1.0f};
  direction = Direction::Right;
  attackPhase = AttackPhase::None;
  onGround = false;
  jumpCount = 0;
  invincibilityFrames = 0;
  attackFrame = 0;

  object = std::make_unique<Object3d>();
  object->Initialize(Object3dCommon::GetInstance());
  object->SetModel(model);
  object->SetCamera(camera);
  UpdateRenderObject();
}

void Player::UpdateMovement() {
  const bool moveRight = input->PushKey(DIK_RIGHT);
  const bool moveLeft = input->PushKey(DIK_LEFT);

  float acceleration = 0.0f;
  if (moveRight != moveLeft) {
    acceleration = moveRight ? (onGround ? kGroundAcceleration
                                         : kAirAcceleration)
                             : -(onGround ? kGroundAcceleration
                                          : kAirAcceleration);
    direction = moveRight ? Direction::Right : Direction::Left;
  }

  if (acceleration * velocity.x < 0.0f) {
    velocity.x *= 1.0f - kGroundAttenuation;
  }

  velocity.x += acceleration;
  velocity.x = std::clamp(velocity.x, -kMaxRunSpeed, kMaxRunSpeed);

  if (onGround && !moveRight && !moveLeft) {
    velocity.x *= 1.0f - kGroundAttenuation;
    if (std::abs(velocity.x) < 0.001f) {
      velocity.x = 0.0f;
    }
  }

  if (input->TriggerKey(DIK_UP) && jumpCount < kMaxJumpCount) {
    velocity.y = kJumpSpeed;
    ++jumpCount;
    onGround = false;
  }

  if (!onGround) {
    velocity.y = std::max(velocity.y - kGravity, -kMaxFallSpeed);
  } else if (velocity.y < 0.0f) {
    velocity.y = 0.0f;
  }
}

void Player::BeginAttack() {
  attackPhase = AttackPhase::Charge;
  attackFrame = 0;
  velocity.x = 0.0f;
  scale = {1.0f, 1.0f, 1.0f};
}

void Player::UpdateAttackAction() {
  if (!onGround) {
    velocity.y = std::max(velocity.y - kGravity, -kMaxFallSpeed);
  } else if (velocity.y < 0.0f) {
    velocity.y = 0.0f;
  }

  ++attackFrame;
  switch (attackPhase) {
  case AttackPhase::Charge: {
    const float t = static_cast<float>(attackFrame) /
                    static_cast<float>(kAttackChargeFrames);
    scale.y = EaseOut(1.0f, 1.6f, t);
    scale.z = EaseOut(1.0f, 0.3f, t);
    velocity.x = 0.0f;

    if (attackFrame >= kAttackChargeFrames) {
      attackPhase = AttackPhase::Rush;
      attackFrame = 0;
    }
    break;
  }

  case AttackPhase::Rush: {
    const float t = static_cast<float>(attackFrame) /
                    static_cast<float>(kAttackRushFrames);
    scale.y = EaseOut(1.6f, 0.7f, t);
    scale.z = EaseOut(0.3f, 1.3f, t);
    velocity.x = direction == Direction::Right ? kAttackRushSpeed
                                                : -kAttackRushSpeed;

    if (attackFrame >= kAttackRushFrames) {
      attackPhase = AttackPhase::Recovery;
      attackFrame = 0;
    }
    break;
  }

  case AttackPhase::Recovery: {
    const float t = static_cast<float>(attackFrame) /
                    static_cast<float>(kAttackRecoveryFrames);
    scale.y = EaseOut(0.7f, 1.0f, t);
    scale.z = EaseOut(1.3f, 1.0f, t);
    velocity.x *= 1.0f - kGroundAttenuation;

    if (attackFrame >= kAttackRecoveryFrames) {
      EndAttack();
    }
    break;
  }

  case AttackPhase::None:
    break;
  }
}

void Player::EndAttack() {
  attackPhase = AttackPhase::None;
  attackFrame = 0;
  velocity.x = 0.0f;
  scale = {1.0f, 1.0f, 1.0f};
}

bool Player::IsAttacking() const {
  return attackPhase == AttackPhase::Rush;
}

bool Player::OverlapsBlock(const PlayerAABB &aabb, uint32_t x,
                           uint32_t y) const {
  if (mapChipField->GetMapChipTypeByIndex(x, y) != MapChipType::Block) {
    return false;
  }

  const MapChipRect block = mapChipField->GetRectByIndex(x, y);
  return aabb.max.x > block.left && aabb.min.x < block.right &&
         aabb.max.y > block.bottom && aabb.min.y < block.top;
}

void Player::ResolveHorizontalCollision(float moveX) {
  if (moveX == 0.0f) {
    return;
  }

  float resolvedX = position.x + moveX;
  PlayerAABB movedAABB = {
      {resolvedX - kHalfWidth, position.y - kHalfHeight,
       position.z - kHalfWidth},
      {resolvedX + kHalfWidth, position.y + kHalfHeight,
       position.z + kHalfWidth},
  };

  for (uint32_t y = 0; y < mapChipField->GetNumBlockVertical(); ++y) {
    for (uint32_t x = 0; x < mapChipField->GetNumBlockHorizontal(); ++x) {
      if (!OverlapsBlock(movedAABB, x, y)) {
        continue;
      }

      const MapChipRect block = mapChipField->GetRectByIndex(x, y);
      if (moveX > 0.0f) {
        resolvedX = std::min(resolvedX,
                             block.left - kHalfWidth - kCollisionSkin);
      } else {
        resolvedX = std::max(resolvedX,
                             block.right + kHalfWidth + kCollisionSkin);
      }
      velocity.x = 0.0f;

      movedAABB.min.x = resolvedX - kHalfWidth;
      movedAABB.max.x = resolvedX + kHalfWidth;
    }
  }

  position.x = resolvedX;
}

void Player::ResolveVerticalCollision(float moveY) {
  onGround = false;
  if (moveY == 0.0f) {
    onGround = IsStandingOnBlock();
    return;
  }

  float resolvedY = position.y + moveY;
  PlayerAABB movedAABB = {
      {position.x - kHalfWidth, resolvedY - kHalfHeight,
       position.z - kHalfWidth},
      {position.x + kHalfWidth, resolvedY + kHalfHeight,
       position.z + kHalfWidth},
  };

  for (uint32_t y = 0; y < mapChipField->GetNumBlockVertical(); ++y) {
    for (uint32_t x = 0; x < mapChipField->GetNumBlockHorizontal(); ++x) {
      if (!OverlapsBlock(movedAABB, x, y)) {
        continue;
      }

      const MapChipRect block = mapChipField->GetRectByIndex(x, y);
      if (moveY > 0.0f) {
        resolvedY = std::min(resolvedY,
                             block.bottom - kHalfHeight - kCollisionSkin);
      } else {
        resolvedY = std::max(resolvedY,
                             block.top + kHalfHeight + kCollisionSkin);
        onGround = true;
        jumpCount = 0;
      }
      velocity.y = 0.0f;

      movedAABB.min.y = resolvedY - kHalfHeight;
      movedAABB.max.y = resolvedY + kHalfHeight;
    }
  }

  position.y = resolvedY;
}

bool Player::IsStandingOnBlock() const {
  PlayerAABB probe = GetAABB();
  probe.min.y -= kGroundProbeDistance;
  probe.max.y -= kGroundProbeDistance;

  for (uint32_t y = 0; y < mapChipField->GetNumBlockVertical(); ++y) {
    for (uint32_t x = 0; x < mapChipField->GetNumBlockHorizontal(); ++x) {
      if (OverlapsBlock(probe, x, y)) {
        return true;
      }
    }
  }
  return false;
}

void Player::Update() {
  if (invincibilityFrames > 0) {
    --invincibilityFrames;
  }

  previousPosition = position;
  onGround = IsStandingOnBlock();
  if (onGround) {
    jumpCount = 0;
  }

  if (attackPhase == AttackPhase::None && input->TriggerKey(DIK_SPACE)) {
    BeginAttack();
  }

  if (attackPhase == AttackPhase::None) {
    UpdateMovement();
  } else {
    UpdateAttackAction();
  }

  ResolveHorizontalCollision(velocity.x);
  ResolveVerticalCollision(velocity.y);

  if (attackPhase == AttackPhase::Rush && attackFrame > 0 &&
      velocity.x == 0.0f) {
    attackPhase = AttackPhase::Recovery;
    attackFrame = 0;
  }

  rotation.y = direction == Direction::Right
                   ? std::numbers::pi_v<float> / 2.0f
                   : std::numbers::pi_v<float> * 3.0f / 2.0f;
  UpdateRenderObject();
}

void Player::Respawn() {
  position = respawnPosition;
  previousPosition = respawnPosition;
  velocity = {};
  rotation = {0.0f, std::numbers::pi_v<float> / 2.0f, 0.0f};
  scale = {1.0f, 1.0f, 1.0f};
  direction = Direction::Right;
  attackPhase = AttackPhase::None;
  onGround = false;
  jumpCount = 0;
  invincibilityFrames = kRespawnInvincibilityFrames;
  attackFrame = 0;
  UpdateRenderObject();
}

void Player::ResolveMovingPlatformCollision(
    const MapChipRect &platformRect,
    const Vector::Vector3 &platformMoveDelta) {
  const PlayerAABB playerAABB = GetAABB();
  const float previousBottom = previousPosition.y - kHalfHeight;
  const float currentBottom = playerAABB.min.y;
  const bool overlapsHorizontally =
      playerAABB.max.x > platformRect.left &&
      playerAABB.min.x < platformRect.right;
  const bool crossedPlatformTop =
      previousBottom >= platformRect.top - kCollisionSkin &&
      currentBottom <= platformRect.top + kGroundProbeDistance;

  if (velocity.y > 0.0f || !overlapsHorizontally || !crossedPlatformTop) {
    return;
  }

  position.y = platformRect.top + kHalfHeight + kCollisionSkin;
  velocity.y = 0.0f;
  onGround = true;
  jumpCount = 0;

  ResolveHorizontalCollision(platformMoveDelta.x);
  UpdateRenderObject();
}

void Player::UpdateRenderObject() {
  object->SetTranslate(position);
  object->SetRotate(rotation);
  object->SetScale(scale);
  object->Update();
}

void Player::Draw() {
  assert(object);
  object->Draw();
}

PlayerAABB Player::GetAABB() const {
  return {
      {position.x - kHalfWidth, position.y - kHalfHeight,
       position.z - kHalfWidth},
      {position.x + kHalfWidth, position.y + kHalfHeight,
       position.z + kHalfWidth},
  };
}
