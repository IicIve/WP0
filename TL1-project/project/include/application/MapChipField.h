#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include "Vector.h"

enum class MapChipType {
  Blank,
  Block,
  Goal,
  MovingPlatform,
};

struct MapChipIndex {
  uint32_t x;
  uint32_t y;
};

struct MapChipRect {
  float left;
  float right;
  float bottom;
  float top;
};

class MapChipField {
public:
  static constexpr float kBlockWidth = 2.0f;
  static constexpr float kBlockHeight = 2.0f;
  static constexpr uint32_t kNumBlockVertical = 20;
  static constexpr uint32_t kNumBlockHorizontal = 100;

  void LoadMapChipCsv(const std::string &filePath);

  MapChipType GetMapChipTypeByIndex(uint32_t xIndex,
                                    uint32_t yIndex) const;
  Vector::Vector3 GetMapChipPositionByIndex(uint32_t xIndex,
                                            uint32_t yIndex) const;
  MapChipIndex
  GetMapChipIndexSetByPosition(const Vector::Vector3 &position) const;
  MapChipRect GetRectByIndex(uint32_t xIndex, uint32_t yIndex) const;

  uint32_t GetNumBlockVertical() const { return kNumBlockVertical; }
  uint32_t GetNumBlockHorizontal() const { return kNumBlockHorizontal; }

private:
  void ResetMapChipData();

  std::vector<std::vector<MapChipType>> mapChipData;
};
