#include "cSkinnedGameObject.h"
#include "cShaderProgram.h"

#define _USE_MATH_DEFINES
#include <stack>
#include <math.h>
#include <glm\gtc\matrix_transform.hpp>

cSkinnedGameObject::cSkinnedGameObject(std::string modelName, std::string modelDir, glm::vec3 position, glm::vec3 scale, glm::vec3 orientationEuler, std::vector<std::string> charAnimations)
{
	this->Model = new cSkinnedMesh(modelDir.c_str());

	this->Position = position;
	this->Scale = scale;
	this->OrientationQuat = glm::quat(orientationEuler);
	this->OrientationEuler.x = glm::radians(orientationEuler.x);
	this->OrientationEuler.y = glm::radians(orientationEuler.y);
	this->OrientationEuler.z = glm::radians(orientationEuler.z);

	this->Forward = glm::vec3(0.0f, 0.0f, 0.0f);
	this->Forward.z = cos(this->OrientationEuler.y);
	this->Forward.x = sin(this->OrientationEuler.y);
	this->Forward = glm::normalize(this->Forward);

	this->defaultAnimState = new sAnimationState();
	this->defaultAnimState->defaultAnimation.name = modelDir;
	this->defaultAnimState->defaultAnimation.frameStepTime = 0.005f;
	this->defaultAnimState->defaultAnimation.totalTime = this->Model->GetDuration();

	this->curAnimState = new sAnimationState();
	this->curAnimState->defaultAnimation.name = modelDir;
	this->curAnimState->defaultAnimation.frameStepTime = 0.004f;
	//this->curAnimState->defaultAnimation.frameStepTime = 0.005f;
	this->curAnimState->defaultAnimation.totalTime = this->Model->GetDuration();

	this->vecCharacterAnimations = charAnimations;

	for (int i = 0; i != this->vecCharacterAnimations.size(); i++)
	{
		this->Model->LoadMeshAnimation(this->vecCharacterAnimations[i]);
	}

	this->Speed = 4.0f;
	this->animToPlay = this->defaultAnimState->defaultAnimation.name;
	oneFrameEvent = false;
	oneFrameStandWalkRun = 0;
}

void cSkinnedGameObject::Draw(cShaderProgram Shader, float deltaTime)
{
	float curFrameTime = 0.0f;

	if (this->defaultAnimState->defaultAnimation.name == this->animToPlay)
	{
		this->defaultAnimState->defaultAnimation.IncrementTime();
		curFrameTime = this->defaultAnimState->defaultAnimation.currentTime;
	}
	else
	{
		this->curAnimState->defaultAnimation.totalTime = this->Model->MapAnimationNameToScene[this->animToPlay]->mAnimations[0]->mDuration /
			this->Model->MapAnimationNameToScene[this->animToPlay]->mAnimations[0]->mTicksPerSecond;
		this->curAnimState->defaultAnimation.name = this->animToPlay;
		this->curAnimState->defaultAnimation.IncrementTime();
		curFrameTime = this->curAnimState->defaultAnimation.currentTime;
	}

	std::vector<glm::mat4> vecFinalTransformation;
	std::vector<glm::mat4> vecOffsets;

	this->Model->BoneTransform(curFrameTime, animToPlay, vecFinalTransformation, this->vecBoneTransformation, vecOffsets);

	GLuint numBonesUsed = static_cast<GLuint>(vecFinalTransformation.size());
	Shader.useProgram();
	Shader.setInt("numBonesUsed", numBonesUsed);
	glm::mat4* boneMatrixArray = &(vecFinalTransformation[0]);
	Shader.setMat4("bones", numBonesUsed, boneMatrixArray);

	glm::mat4 model = glm::mat4(1.0f);
	model = glm::translate(model, this->Position);
	model = glm::rotate(model, this->OrientationEuler.x, glm::vec3(1.0f, 0.0f, 0.0f));
	model = glm::rotate(model, this->OrientationEuler.y, glm::vec3(0.0f, 1.0f, 0.0f));
	model = glm::rotate(model, this->OrientationEuler.z, glm::vec3(0.0f, 0.0f, 1.0f));
	model = glm::scale(model, this->Scale);
	Shader.setMat4("model", model);

	this->Model->Draw(Shader);

	if (oneFrameEvent)
	{
		if (oneFrameStandWalkRun == 0)
		{
			//Animation happens while standing
		}
		else if (oneFrameStandWalkRun == 1)
		{	//Animation was called while walking
			this->Movement(FORWARD, deltaTime, false, false);
		}
		else if (oneFrameStandWalkRun == 2)
		{
			this->Movement(FORWARD, deltaTime, true, false);
		}
		if (curFrameTime == 0)
		{
			this->animToPlay = "assets/modelsFBX/IdleRobaxacet.fbx";
			oneFrameEvent = false;
		}
	}
}

void cSkinnedGameObject::Movement(Direction direction, float deltaTime, bool shiftKey, bool ctrlKey)
{
	float mySpeed = this->Speed;
	if (shiftKey)
		mySpeed += 8.0f;

	if (direction == FORWARD)
	{
		if (ctrlKey)
			return;
		this->Position += this->Forward * deltaTime * mySpeed;
	}

	else if (direction == BACKWARD)
	{
		if (ctrlKey)
			return;
		this->Position -= this->Forward * deltaTime * mySpeed;
	}

	else if (direction == LEFT)
	{
		if (ctrlKey)
		{
			float leftDirection = this->OrientationEuler.y + (M_PI / 2.0f);
			float strafeLeftZ = cos(leftDirection);
			float strafeLeftX = sin(leftDirection);
			glm::vec3 strafeLeft = glm::normalize(glm::vec3(strafeLeftX, 0.0f, strafeLeftZ));
			this->Position += strafeLeft * deltaTime * mySpeed;
		}
		else
		{
			this->OrientationEuler.y += this->Speed * deltaTime;
			this->Forward.z = cos(this->OrientationEuler.y);
			this->Forward.x = sin(this->OrientationEuler.y);
			this->Forward = glm::normalize(this->Forward);
		}
	}

	else if (direction == RIGHT)
	{
		if (ctrlKey)
		{
			float rightDirection = this->OrientationEuler.y - (M_PI / 2.0f);
			float strafeRightZ = cos(rightDirection);
			float strafeRightX = sin(rightDirection);
			glm::vec3 strafeRight = glm::normalize(glm::vec3(strafeRightX, 0.0f, strafeRightZ));
			this->Position += strafeRight * deltaTime * mySpeed;
		}
		else
		{
			this->OrientationEuler.y -= this->Speed * deltaTime;
			this->Forward.z = cos(this->OrientationEuler.y);
			this->Forward.x = sin(this->OrientationEuler.y);
			this->Forward = glm::normalize(this->Forward);
		}
	}

	float boundaries = 9.5f;

	if (Position.x >= boundaries)
	{
		Position.x = boundaries;
	}
	else if (Position.x <= -boundaries)
	{
		Position.x = -boundaries;
	}
	if (Position.z >= boundaries)
	{
		Position.z = boundaries;
	}
	else if (Position.z <= -boundaries)
	{
		Position.z = -boundaries;
	}
}