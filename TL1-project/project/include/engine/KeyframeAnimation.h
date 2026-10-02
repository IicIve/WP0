#pragma once

#include <vector>
#include <map>
#include <string>

#include "Vector.h"
#include "Quaternion.h"
#include "ModelManager.h"
#include "MathFunc.h"
#include "Model.h"

using namespace Vector;
using namespace QuaternionMath;

struct KeyframeVector3 {
	Vector3 value;
	float time;
};

struct KeyframeQuaternion {
	Quaternion value;
	float time;
};

template <typename tValue>

struct Keyframe {
	float time;
	tValue value;
};

//using KeyframeVector3 = Keyframe<Vector3>;
//using KeyframeQuaternion = Keyframe<Quaternion>;

struct NodeAnimation {
	std::vector<KeyframeVector3> translate;
	std::vector<KeyframeQuaternion> rotate;
	std::vector<KeyframeVector3> scale;
};

template <typename tValue>

struct AnimationCurve {
	std::vector<Keyframe<tValue>> keyframes;
};

struct Animation {
	float duration;//アニメーション全体の尺(単位は秒)
	//NodeAnimationの集合、Node名でひけるようにしておく
	std::map<std::string, NodeAnimation> nodeAnimations;

};

class KeyframeAnimation {
public:
	struct VertexData {
		Vector4 position;
		Vector2 texcoord;
		Vector3 normal;
	};

	struct MaterialData {
		std::string textureFilePath;
		uint32_t textureIndex = 0;
	};

	struct ModelData {
		std::vector<VertexData> vertices;
		MaterialData material;
	};

	Animation LoadAnimationFile(const std::string& directoryPath, const std::string& filename);
	void Update(float deltaTime);
	void ApplyAnimation(Model::Skeleton& skeleton, const Animation& animation, float animationTime);
	Vector3 CalculateValue(const std::vector<KeyframeVector3>& keyframes, float time);
	Quaternion CalculateValue(const std::vector<KeyframeQuaternion>& keyframes, float time);
	const Model::ModelData& GetModelData() const { return model; }
	const Animation& GetAnimation() const { return animation; }
	float GetAnimationTime() const { return animationTime; }

private:
	Model::ModelData model = Model::LoadModelFile("resources", "AnimatedCube.gltf");
	Animation animation = LoadAnimationFile("resources", "walk.gltf");

	float animationTime = 0.0f;
	


};

