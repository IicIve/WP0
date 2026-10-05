#pragma once

#include <cstdint>
#include <memory>

#include "Vector.h"

class Camera;
class Input;
class MapChipField;
struct MapChipRect;
class Model;
class Object3d;

struct PlayerAABB {
  Vector::Vector3 min;
  Vector::Vector3 max;
};

class Player {
public:
  Player();
  ~Player();

  Player(const Player &) = delete;
  Player &operator=(const Player &) = delete;

  void Initialize(Model *model, Camera *camera, Input *input,
                  MapChipField *mapChipField,
                  const Vector::Vector3 &position);
  void Update();
  void Draw();
  void Respawn();
  void ResolveMovingPlatformCollision(
      const MapChipRect &platformRect,
      const Vector::Vector3 &platformMoveDelta);

  const Vector::Vector3 &GetPosition() const { return position; }
  const Vector::Vector3 &GetVelocity() const { return velocity; }
  bool CanBeHit() const { return invincibilityFrames == 0; }
  bool IsAttacking() const;
  PlayerAABB GetAABB() const;

private:
  enum class Direction {
    Right,
    Left,
  };

  enum class AttackPhase {
    None,
    Charge,
    Rush,
    Recovery,
  };

  void UpdateMovement();
  void BeginAttack();
  void UpdateAttackAction();
  void EndAttack();
  void ResolveHorizontalCollision(float moveX);
  void ResolveVerticalCollision(float moveY);
  bool IsStandingOnBlock() const;
  bool OverlapsBlock(const PlayerAABB &aabb, uint32_t x,
                     uint32_t y) const;
  void UpdateRenderObject();

  Model *model = nullptr;
  Camera *camera = nullptr;
  Input *input = nullptr;
  MapChipField *mapChipField = nullptr;
  std::unique_ptr<Object3d> object;

  Vector::Vector3 position{};
  Vector::Vector3 respawnPosition{};
  Vector::Vector3 previousPosition{};
  Vector::Vector3 velocity{};
  Vector::Vector3 rotation{};
  Vector::Vector3 scale{1.0f, 1.0f, 1.0f};
  Direction direction = Direction::Right;
  AttackPhase attackPhase = AttackPhase::None;

  bool onGround = false;
  uint32_t jumpCount = 0;
  uint32_t invincibilityFrames = 0;
  uint32_t attackFrame = 0;

  static constexpr float kHalfWidth = 0.4f;
  static constexpr float kHalfHeight = 0.4f;
  static constexpr float kCollisionSkin = 0.01f;
  static constexpr float kGroundProbeDistance = 0.05f;
  static constexpr float kGroundAcceleration = 0.1f;
  static constexpr float kAirAcceleration = 0.03f;
  static constexpr float kGroundAttenuation = 0.35f;
  static constexpr float kMaxRunSpeed = 0.7f;
  static constexpr float kGravity = 0.1f;
  static constexpr float kMaxFallSpeed = 3.0f;
  static constexpr float kJumpSpeed = 1.3f;
  static constexpr uint32_t kMaxJumpCount = 2;
  static constexpr uint32_t kRespawnInvincibilityFrames = 60;
  static constexpr uint32_t kAttackChargeFrames = 30;
  static constexpr uint32_t kAttackRushFrames = 30;
  static constexpr uint32_t kAttackRecoveryFrames = 30;
  static constexpr float kAttackRushSpeed = 0.8f;
};
