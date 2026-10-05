#define NOMINMAX
#include "Enemy.h"

#include <algorithm>
#include <cassert>
#include <cmath>
#include <numbers>

#include "Camera.h"
#include "MapChipField.h"
#include "Model.h"
#include "Object3d.h"
#include "Object3dCommon.h"

Enemy::Enemy() = default;

Enemy::~Enemy() = default;

void Enemy::Initialize(Model *model, Camera *camera,
                       MapChipField *mapChipField,
                       const Vector::Vector3 &position) {
  assert(model);
  assert(camera);
  assert(mapChipField);

  this->model = model;
  this->camera = camera;
  this->mapChipField = mapChipField;
  this->position = position;

  velocity = {-0.02f, 0.0f, 0.0f};
  rotation = {};
  onGround = false;
  walkTimer = 0.0f;

  object = std::make_unique<Object3d>();
  object->Initialize(Object3dCommon::GetInstance());
  object->SetModel(model);
  object->SetCamera(camera);
  UpdateRenderObject();
}

bool Enemy::OverlapsBlock(const EnemyAABB &aabb, uint32_t x,
                          uint32_t y) const {
  if (mapChipField->GetMapChipTypeByIndex(x, y) != MapChipType::Block) {
    return false;
  }

  const MapChipRect block = mapChipField->GetRectByIndex(x, y);
  return aabb.max.x > block.left && aabb.min.x < block.right &&
         aabb.max.y > block.bottom && aabb.min.y < block.top;
}

bool Enemy::IsStandingOnBlock() const {
  EnemyAABB probe = GetAABB();
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

bool Enemy::HasGroundAhead(float nextX) const {
  const float direction = velocity.x >= 0.0f ? 1.0f : -1.0f;
  const float probeX =
      nextX + direction * (kHalfWidth + kGroundProbeDistance);
  const float probeY =
      position.y - kHalfHeight - kGroundProbeDistance;

  for (uint32_t y = 0; y < mapChipField->GetNumBlockVertical(); ++y) {
    for (uint32_t x = 0; x < mapChipField->GetNumBlockHorizontal(); ++x) {
      if (mapChipField->GetMapChipTypeByIndex(x, y) !=
          MapChipType::Block) {
        continue;
      }

      const MapChipRect block = mapChipField->GetRectByIndex(x, y);
      if (probeX >= block.left && probeX <= block.right &&
          probeY >= block.bottom && probeY <= block.top) {
        return true;
      }
    }
  }
  return false;
}

void Enemy::MoveWithMapCollision() {
  onGround = IsStandingOnBlock();

  const float nextX = position.x + velocity.x;
  const EnemyAABB horizontalAABB = {
      {nextX - kHalfWidth, position.y - kHalfHeight,
       position.z - kHalfWidth},
      {nextX + kHalfWidth, position.y + kHalfHeight,
       position.z + kHalfWidth},
  };

  bool hitWall = false;
  for (uint32_t y = 0;
       y < mapChipField->GetNumBlockVertical() && !hitWall; ++y) {
    for (uint32_t x = 0; x < mapChipField->GetNumBlockHorizontal(); ++x) {
      if (OverlapsBlock(horizontalAABB, x, y)) {
        hitWall = true;
        break;
      }
    }
  }

  const bool reachedEdge = onGround && !hitWall && !HasGroundAhead(nextX);
  if (hitWall || reachedEdge) {
    velocity.x = -velocity.x;
  } else {
    position.x = nextX;
  }

  velocity.y = std::max(velocity.y - kGravity, -kMaxFallSpeed);
  float resolvedY = position.y + velocity.y;
  EnemyAABB verticalAABB = {
      {position.x - kHalfWidth, resolvedY - kHalfHeight,
       position.z - kHalfWidth},
      {position.x + kHalfWidth, resolvedY + kHalfHeight,
       position.z + kHalfWidth},
  };

  onGround = false;
  for (uint32_t y = 0; y < mapChipField->GetNumBlockVertical(); ++y) {
    for (uint32_t x = 0; x < mapChipField->GetNumBlockHorizontal(); ++x) {
      if (mapChipField->GetMapChipTypeByIndex(x, y) !=
          MapChipType::Block) {
        continue;
      }

      const MapChipRect block = mapChipField->GetRectByIndex(x, y);
      const bool overlapsHorizontally =
          verticalAABB.max.x > block.left &&
          verticalAABB.min.x < block.right;
      const float currentBottom = position.y - kHalfHeight;
      const float currentTop = position.y + kHalfHeight;
      const bool crossedFloor =
          velocity.y < 0.0f && currentBottom >= block.top &&
          verticalAABB.min.y <= block.top;
      const bool crossedCeiling =
          velocity.y > 0.0f && currentTop <= block.bottom &&
          verticalAABB.max.y >= block.bottom;

      if (!overlapsHorizontally ||
          (!OverlapsBlock(verticalAABB, x, y) && !crossedFloor &&
           !crossedCeiling)) {
        continue;
      }

      if (velocity.y < 0.0f) {
        resolvedY = std::max(
            resolvedY, block.top + kHalfHeight + kCollisionSkin);
        onGround = true;
      } else if (velocity.y > 0.0f) {
        resolvedY = std::min(
            resolvedY, block.bottom - kHalfHeight - kCollisionSkin);
      }
      velocity.y = 0.0f;
      verticalAABB.min.y = resolvedY - kHalfHeight;
      verticalAABB.max.y = resolvedY + kHalfHeight;
    }
  }

  position.y = resolvedY;
}

void Enemy::Update() {
  walkTimer += 1.0f / 60.0f;
  const float walkWave =
      std::sin(walkTimer * std::numbers::pi_v<float> * 2.0f);
  rotation.z = kWalkTiltDegrees * walkWave *
               (std::numbers::pi_v<float> / 180.0f);
  rotation.y = velocity.x >= 0.0f
                   ? std::numbers::pi_v<float> / 2.0f
                   : std::numbers::pi_v<float> * 3.0f / 2.0f;

  MoveWithMapCollision();
  UpdateRenderObject();
}

void Enemy::UpdateRenderObject() {
  object->SetTranslate(position);
  object->SetRotate(rotation);
  object->Update();
}

void Enemy::Draw() {
  assert(object);
  object->Draw();
}

EnemyAABB Enemy::GetAABB() const {
  return {
      {position.x - kHalfWidth, position.y - kHalfHeight,
       position.z - kHalfWidth},
      {position.x + kHalfWidth, position.y + kHalfHeight,
       position.z + kHalfWidth},
  };
}
