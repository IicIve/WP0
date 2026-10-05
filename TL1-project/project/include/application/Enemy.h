#pragma once

#include <cstdint>
#include <memory>

#include "Vector.h"

class Camera;
class MapChipField;
class Model;
class Object3d;

struct EnemyAABB {
  Vector::Vector3 min;
  Vector::Vector3 max;
};

class Enemy {
public:
  Enemy();
  ~Enemy();

  Enemy(const Enemy &) = delete;
  Enemy &operator=(const Enemy &) = delete;

  void Initialize(Model *model, Camera *camera,
                  MapChipField *mapChipField,
                  const Vector::Vector3 &position);
  void Update();
  void Draw();

  const Vector::Vector3 &GetPosition() const { return position; }
  EnemyAABB GetAABB() const;

private:
  void MoveWithMapCollision();
  bool IsStandingOnBlock() const;
  bool HasGroundAhead(float nextX) const;
  bool OverlapsBlock(const EnemyAABB &aabb, uint32_t x,
                     uint32_t y) const;
  void UpdateRenderObject();

  Model *model = nullptr;
  Camera *camera = nullptr;
  MapChipField *mapChipField = nullptr;
  std::unique_ptr<Object3d> object;

  Vector::Vector3 position{};
  Vector::Vector3 velocity{-0.02f, 0.0f, 0.0f};
  Vector::Vector3 rotation{};
  bool onGround = false;
  float walkTimer = 0.0f;

  static constexpr float kHalfWidth = 0.4f;
  static constexpr float kHalfHeight = 0.4f;
  static constexpr float kCollisionSkin = 0.01f;
  static constexpr float kGroundProbeDistance = 0.05f;
  static constexpr float kGravity = 0.1f;
  static constexpr float kMaxFallSpeed = 3.0f;
  static constexpr float kWalkTiltDegrees = 16.0f;
};
