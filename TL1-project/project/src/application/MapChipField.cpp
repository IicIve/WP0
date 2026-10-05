#include "MapChipField.h"

#include <algorithm>
#include <cassert>
#include <fstream>
#include <limits>
#include <sstream>
#include <string_view>

namespace {

MapChipType ParseMapChip(std::string_view value) {
  if (value == "1") {
    return MapChipType::Block;
  }
  if (value == "2") {
    return MapChipType::Goal;
  }
  if (value == "3") {
    return MapChipType::MovingPlatform;
  }
  return MapChipType::Blank;
}

} // namespace

void MapChipField::ResetMapChipData() {
  mapChipData.assign(kNumBlockVertical,
                     std::vector<MapChipType>(kNumBlockHorizontal,
                                              MapChipType::Blank));
}

void MapChipField::LoadMapChipCsv(const std::string &filePath) {
  ResetMapChipData();

  std::ifstream file(filePath);
  assert(file.is_open());

  for (uint32_t y = 0; y < kNumBlockVertical; ++y) {
    std::string line;
    if (!std::getline(file, line)) {
      break;
    }

    std::istringstream lineStream(line);
    for (uint32_t x = 0; x < kNumBlockHorizontal; ++x) {
      std::string value;
      if (!std::getline(lineStream, value, ',')) {
        break;
      }
      mapChipData[y][x] = ParseMapChip(value);
    }
  }
}

MapChipType MapChipField::GetMapChipTypeByIndex(uint32_t xIndex,
                                                uint32_t yIndex) const {
  if (xIndex >= kNumBlockHorizontal || yIndex >= kNumBlockVertical) {
    return MapChipType::Blank;
  }
  return mapChipData[yIndex][xIndex];
}

Vector::Vector3
MapChipField::GetMapChipPositionByIndex(uint32_t xIndex,
                                        uint32_t yIndex) const {
  return {kBlockWidth * static_cast<float>(xIndex),
          kBlockHeight * static_cast<float>(kNumBlockVertical - 1 - yIndex),
          0.0f};
}

MapChipIndex MapChipField::GetMapChipIndexSetByPosition(
    const Vector::Vector3 &position) const {
  constexpr uint32_t kInvalidIndex = std::numeric_limits<uint32_t>::max();
  const float mapLeft = -kBlockWidth / 2.0f;
  const float mapRight =
      kBlockWidth * static_cast<float>(kNumBlockHorizontal - 1) +
      kBlockWidth / 2.0f;
  const float mapTop =
      kBlockHeight * static_cast<float>(kNumBlockVertical - 1) +
      kBlockHeight / 2.0f;
  const float mapBottom = -kBlockHeight / 2.0f;

  if (position.x < mapLeft || position.x >= mapRight ||
      position.y < mapBottom || position.y >= mapTop) {
    return {kInvalidIndex, kInvalidIndex};
  }

  return {
      static_cast<uint32_t>((position.x - mapLeft) / kBlockWidth),
      static_cast<uint32_t>((mapTop - position.y) / kBlockHeight),
  };
}

MapChipRect MapChipField::GetRectByIndex(uint32_t xIndex,
                                         uint32_t yIndex) const {
  const Vector::Vector3 center =
      GetMapChipPositionByIndex(xIndex, yIndex);
  return {
      center.x - kBlockWidth / 2.0f,
      center.x + kBlockWidth / 2.0f,
      center.y - kBlockHeight / 2.0f,
      center.y + kBlockHeight / 2.0f,
  };
}
