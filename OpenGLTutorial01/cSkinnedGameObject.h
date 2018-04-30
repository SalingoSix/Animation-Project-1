#ifndef _HG_cSkinnedGameObject_
#define _HG_cSkinnedGameObject_

#include <glm\vec3.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm\gtc\quaternion.hpp>
#include <glm\gtx\quaternion.hpp>
#include <string>

#include "cSkinnedMesh.h"
#include "sAnimationState.h"

class cSkinnedGameObject
{
public:

	enum Direction
	{
		FORWARD,
		BACKWARD,
		LEFT,
		RIGHT
	};

	glm::vec3 Position, Scale;
	glm::vec3 Forward;
	glm::quat OrientationQuat;
	glm::vec3 OrientationEuler;

	cSkinnedGameObject(std::string modelName, std::string modelDir, glm::vec3 position, glm::vec3 scale, glm::vec3 orientationEuler, std::vector<std::string> charAnimations);

	void Draw(cShaderProgram Shader, float deltaTime);
	void Movement(Direction direction, float deltaTime, bool shiftKey, bool ctrlKey);

	std::vector<std::string> vecCharacterAnimations;
	sAnimationState* defaultAnimState, *curAnimState;
	std::string animToPlay;
	float Speed;

	cSkinnedMesh * Model;

	bool oneFrameEvent;
	int oneFrameStandWalkRun;
private:
	std::vector<glm::mat4> vecBoneTransformation;
};
#endif