#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <SOIL2/SOIL2.h>

#include <iostream>
#include <string>
#include <vector>
#include <map>

#include "cShaderProgram.h"
#include "cCamera.h"
#include "cMesh.h"
#include "cModel.h"
#include "cSkybox.h"
#include "cSkinnedMesh.h"
#include "cSkinnedGameObject.h"
#include "sAnimationState.h"
#include "cScreenQuad.h"
#include "cPlaneObject.h"
#include "cFrameBuffer.h"

//Setting up a camera GLOBAL
cCamera Camera(glm::vec3(0.0f, 15.0f, 25.0f),		//Camera Position
			glm::vec3(0.0f, 1.0f, 0.0f),		//World Up vector
			-30.0f,								//Pitch
			-90.0f);							//Yaw

float deltaTime = 0.0f;
float lastFrame = 0.0f;
bool firstMouse = true;
float lastX = 400, lastY = 300;

unsigned int SCR_WIDTH = 800;
unsigned int SCR_HEIGHT = 800;

//For second pass render filters
int drawType = 1;

//For animation
cSkinnedGameObject* robaxMan;
cSkinnedGameObject* robaxBoy;
cSkinnedGameObject* robaxSelected;
bool boyOrMan = false;
bool shiftLock = false;
bool spaceLock = false;
bool shiftKey = false;
bool ctrlKey = false;
bool jumpHappening = false;
bool actionHappening = false;

std::map<std::string, cModel*> mapModelsToNames;
std::map<std::string, cShaderProgram*> mapShaderToName;

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow* window);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
unsigned int loadCubeMap(std::string directory, std::vector<std::string> faces);

int main()
{
	glfwInit();

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL", NULL, NULL);
	if (window == NULL)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}

	glfwMakeContextCurrent(window);

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialzed GLAD" << std::endl;
		return -1;
	}

	glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);

	//Setting up global openGL state
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);

	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetScrollCallback(window, scroll_callback);

	//Set up all our programs
	cShaderProgram* myProgram = new cShaderProgram();
	myProgram->compileProgram("assets/shaders/", "vertShader.glsl", "fragShader.glsl");
	mapShaderToName["mainProgram"] = myProgram;

	myProgram = new cShaderProgram();
	myProgram->compileProgram("assets/shaders/", "animVert.glsl", "animFrag.glsl");
	mapShaderToName["skinProgram"] = myProgram;

	myProgram = new cShaderProgram();
	myProgram->compileProgram("assets/shaders/", "skyBoxVert.glsl", "skyBoxFrag.glsl");
	mapShaderToName["skyboxProgram"] = myProgram;

	myProgram = new cShaderProgram();
	myProgram->compileProgram("assets/shaders/", "modelVert.glsl", "modelFrag.glsl");
	mapShaderToName["simpleProgram"] = myProgram;

	myProgram = new cShaderProgram();
	myProgram->compileProgram("assets/shaders/", "quadVert.glsl", "quadFrag.glsl");
	mapShaderToName["quadProgram"] = myProgram;

	//Assemble all our models

	std::string path = "assets/models/floor/floor.obj";
	mapModelsToNames["Floor"] = new cModel(path);

	path = "assets/models/fence/fence.obj";
	mapModelsToNames["Fence"] = new cModel(path);

	std::vector<std::string> vecAnims;
	vecAnims.push_back("assets/modelsFBX/IdleRobaxacet.fbx");

	robaxBoy = new cSkinnedGameObject("RobaxacetMan", "assets/modelsFBX/IdleRobaxacet.fbx",
		glm::vec3(0.0f, 0.0f, 0.0f),
		glm::vec3(0.025f),
		glm::vec3(0.0f, 90.0f, 0.0f),
		vecAnims);

	vecAnims.push_back("assets/modelsFBX/WalkingRobaxacet.fbx");
	vecAnims.push_back("assets/modelsFBX/ThrowRobaxacet.fbx");
	vecAnims.push_back("assets/modelsFBX/StrafeLeftRobaxacet.fbx");
	vecAnims.push_back("assets/modelsFBX/StrafeRightRobaxacet.fbx");
	vecAnims.push_back("assets/modelsFBX/RunningBackwardRobaxacet.fbx");
	vecAnims.push_back("assets/modelsFBX/WalkingBackwardRobaxacet.fbx");
	//vecAnims.push_back("assets/modelsFBX/RightTurnRobaxacet.fbx");
	vecAnims.push_back("assets/modelsFBX/LeftTurnRobaxacet.fbx");
	vecAnims.push_back("assets/modelsFBX/RunningJumpRobaxacet.fbx");
	vecAnims.push_back("assets/modelsFBX/StandingJumpRobaxacet.fbx");
	vecAnims.push_back("assets/modelsFBX/RunningRobaxacet.fbx");

	robaxMan = new cSkinnedGameObject("RobaxacetMan", "assets/modelsFBX/IdleRobaxacet.fbx",
		glm::vec3(5.0f, 0.0f, 0.0f),
		glm::vec3(0.025f),
		glm::vec3(0.0f, 0.0f, 0.0f),
		vecAnims);

	robaxBoy->Model = robaxMan->Model;

	robaxSelected = robaxBoy;

	//Creating a frame buffer
	cFrameBuffer mainFrameBuffer(SCR_HEIGHT, SCR_WIDTH);

	//Some simple shapes
	cScreenQuad screenQuad;
	cPlaneObject planeObject;

	//Positions for some of the point light
	glm::vec3 pointLightPositions[] = {
		glm::vec3(5.0f,  0.2f,  5.0f),
		glm::vec3(5.0f, 0.2f, -5.0f),
		glm::vec3(-5.0f,  0.2f, -5.0f),
		glm::vec3(-5.0f,  0.2f, 5.0f)
	};

	//Load the skyboxes
	cSkybox skybox("assets/textures/skybox/");

	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	mapShaderToName["skyboxProgram"]->useProgram();
	mapShaderToName["skyboxProgram"]->setInt("skybox", 0);

	mapShaderToName["mainProgram"]->useProgram();
	mapShaderToName["mainProgram"]->setInt("skybox", 0);

	mapShaderToName["quadProgram"]->useProgram();
	mapShaderToName["quadProgram"]->setInt("screenTexture", 0);

	mapShaderToName["mainProgram"]->useProgram();
	mapShaderToName["mainProgram"]->setInt("reflectRefract", 0);

	while (!glfwWindowShouldClose(window))
	{
		processInput(window);

		float currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		mapShaderToName["mainProgram"]->useProgram();
		//Light settings go in here until I've made a class for them
		{
			//http://devernay.free.fr/cours/opengl/materials.html
			mapShaderToName["mainProgram"]->setVec3("dirLight.direction", -0.2f, -1.0f, -0.3f);
			mapShaderToName["mainProgram"]->setVec3("dirLight.ambient", 0.55f, 0.55f, 0.55f);
			mapShaderToName["mainProgram"]->setVec3("dirLight.diffuse", 0.4f, 0.4f, 0.4f);
			mapShaderToName["mainProgram"]->setVec3("dirLight.specular", 0.5f, 0.5f, 0.5f);

			mapShaderToName["mainProgram"]->setVec3("pointLights[0].position", pointLightPositions[0]);
			mapShaderToName["mainProgram"]->setFloat("pointLights[0].constant", 1.0f);
			mapShaderToName["mainProgram"]->setFloat("pointLights[0].linear", 0.09f);
			mapShaderToName["mainProgram"]->setFloat("pointLights[0].quadratic", 0.032f);
			mapShaderToName["mainProgram"]->setVec3("pointLights[0].ambient", 0.55f, 0.55f, 0.55f);
			mapShaderToName["mainProgram"]->setVec3("pointLights[0].diffuse", 0.8f, 0.8f, 0.8f);
			mapShaderToName["mainProgram"]->setVec3("pointLights[0].specular", 1.0f, 1.0f, 1.0f);

			mapShaderToName["mainProgram"]->setVec3("pointLights[1].position", pointLightPositions[1]);
			mapShaderToName["mainProgram"]->setFloat("pointLights[1].constant", 1.0f);
			mapShaderToName["mainProgram"]->setFloat("pointLights[1].linear", 0.09f);
			mapShaderToName["mainProgram"]->setFloat("pointLights[1].quadratic", 0.032f);
			mapShaderToName["mainProgram"]->setVec3("pointLights[1].ambient", 0.55f, 0.55f, 0.55f);
			mapShaderToName["mainProgram"]->setVec3("pointLights[1].diffuse", 0.8f, 0.8f, 0.8f);
			mapShaderToName["mainProgram"]->setVec3("pointLights[1].specular", 1.0f, 1.0f, 1.0f);

			mapShaderToName["mainProgram"]->setVec3("pointLights[2].position", pointLightPositions[2]);
			mapShaderToName["mainProgram"]->setFloat("pointLights[2].constant", 1.0f);
			mapShaderToName["mainProgram"]->setFloat("pointLights[2].linear", 0.88f);
			mapShaderToName["mainProgram"]->setFloat("pointLights[2].quadratic", 0.032f);
			mapShaderToName["mainProgram"]->setVec3("pointLights[2].ambient", 0.55f, 0.55f, 0.55f);
			mapShaderToName["mainProgram"]->setVec3("pointLights[2].diffuse", 0.8f, 0.8f, 0.8f);
			mapShaderToName["mainProgram"]->setVec3("pointLights[2].specular", 1.0f, 1.0f, 1.0f);

			mapShaderToName["mainProgram"]->setVec3("pointLights[3].position", pointLightPositions[3]);
			mapShaderToName["mainProgram"]->setFloat("pointLights[3].constant", 1.0f);
			mapShaderToName["mainProgram"]->setFloat("pointLights[3].linear", 0.09f);
			mapShaderToName["mainProgram"]->setFloat("pointLights[3].quadratic", 0.032f);
			mapShaderToName["mainProgram"]->setVec3("pointLights[3].ambient", 0.05f, 0.05f, 0.05f);
			mapShaderToName["mainProgram"]->setVec3("pointLights[3].diffuse", 0.8f, 0.8f, 0.8f);
			mapShaderToName["mainProgram"]->setVec3("pointLights[3].specular", 1.0f, 1.0f, 1.0f);

			mapShaderToName["mainProgram"]->setVec3("spotLight.position", Camera.position);
			mapShaderToName["mainProgram"]->setVec3("spotLight.direction", Camera.front);
			mapShaderToName["mainProgram"]->setFloat("spotLight.cutOff", glm::cos(glm::radians(12.5f)));
			mapShaderToName["mainProgram"]->setFloat("spotLight.outerCutOff", glm::cos(glm::radians(15.0f)));
			mapShaderToName["mainProgram"]->setFloat("spotLight.constant", 1.0f);
			mapShaderToName["mainProgram"]->setFloat("spotLight.linear", 0.09f);
			mapShaderToName["mainProgram"]->setFloat("spotLight.quadratic", 0.032f);
			mapShaderToName["mainProgram"]->setVec3("spotLight.ambient", 0.0f, 0.0f, 0.0f);
			mapShaderToName["mainProgram"]->setVec3("spotLight.diffuse", 1.0f, 1.0f, 1.0f);
			mapShaderToName["mainProgram"]->setVec3("spotLight.specular", 1.0f, 1.0f, 1.0f);
			//http://wiki.ogre3d.org/tiki-index.php?page=-Point+Light+Attenuation
		}
		mapShaderToName["mainProgram"]->setVec3("cameraPos", Camera.position);

		glm::mat4 projection = glm::perspective(glm::radians(Camera.zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
		glm::mat4 view = Camera.getViewMatrix();
		glm::mat4 skyboxView = glm::mat4(glm::mat3(Camera.getViewMatrix()));

		//Begin writing to another frame buffer
		glBindFramebuffer(GL_FRAMEBUFFER, mainFrameBuffer.FBO);

		glEnable(GL_DEPTH_TEST);
		glClearColor(0.05f, 0.05f, 0.05f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		mapShaderToName["mainProgram"]->useProgram();
		glm::mat4 model = glm::mat4(1.0);
		model = glm::scale(model, glm::vec3(10.0f, 0.0f, 10.0f));
		mapShaderToName["mainProgram"]->setMat4("model", model);
		mapShaderToName["mainProgram"]->setMat4("projection", projection);
		mapShaderToName["mainProgram"]->setMat4("view", view);
		glBindTexture(GL_TEXTURE_CUBE_MAP, skybox.textureID);
		mapModelsToNames["Floor"]->Draw(*mapShaderToName["mainProgram"]);

		model = glm::mat4(1.0);
		model = glm::translate(model, glm::vec3(0.0f, 0.0f, 10.0f));
		mapShaderToName["mainProgram"]->setMat4("model", model);
		glBindTexture(GL_TEXTURE_CUBE_MAP, skybox.textureID);
		mapModelsToNames["Fence"]->Draw(*mapShaderToName["mainProgram"]);

		model = glm::mat4(1.0);
		model = glm::translate(model, glm::vec3(0.0f, 0.0f, -10.0f));
		mapShaderToName["mainProgram"]->setMat4("model", model);
		glBindTexture(GL_TEXTURE_CUBE_MAP, skybox.textureID);
		mapModelsToNames["Fence"]->Draw(*mapShaderToName["mainProgram"]);

		model = glm::mat4(1.0);
		model = glm::translate(model, glm::vec3(10.0f, 0.0f, 0.0f));
		model = glm::rotate(model, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		mapShaderToName["mainProgram"]->setMat4("model", model);
		glBindTexture(GL_TEXTURE_CUBE_MAP, skybox.textureID);
		mapModelsToNames["Fence"]->Draw(*mapShaderToName["mainProgram"]);

		model = glm::mat4(1.0);
		model = glm::translate(model, glm::vec3(-10.0f, 0.0f, 0.0f));
		model = glm::rotate(model, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		mapShaderToName["mainProgram"]->setMat4("model", model);
		glBindTexture(GL_TEXTURE_CUBE_MAP, skybox.textureID);
		mapModelsToNames["Fence"]->Draw(*mapShaderToName["mainProgram"]);

		//Here am rendering skinned meshes
		mapShaderToName["skinProgram"]->useProgram();
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		mapShaderToName["skinProgram"]->setMat4("projection", projection);
		mapShaderToName["skinProgram"]->setMat4("view", view);
		if (robaxSelected == robaxBoy)
		{
			mapShaderToName["skinProgram"]->setInt("activeBoy", 1);
		}
		robaxBoy->Draw(*mapShaderToName["skinProgram"], deltaTime);

		mapShaderToName["skinProgram"]->setInt("activeBoy", 0);

		mapShaderToName["skinProgram"]->useProgram();
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		mapShaderToName["skinProgram"]->setMat4("projection", projection);
		mapShaderToName["skinProgram"]->setMat4("view", view);
		if (robaxSelected == robaxMan)
		{
			mapShaderToName["skinProgram"]->setInt("activeBoy", 1);
		}
		robaxMan->Draw(*mapShaderToName["skinProgram"], deltaTime);

		mapShaderToName["skinProgram"]->setInt("activeBoy", 0);

		if (jumpHappening)
		{
			jumpHappening = robaxSelected->oneFrameEvent;
		}

		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		
		//Drawing the main scene's skybox
		mapShaderToName["skyboxProgram"]->useProgram();

		glDepthFunc(GL_LEQUAL);  // change depth function so depth test passes when values are equal to depth buffer's content
		mapShaderToName["skyboxProgram"]->setMat4("projection", projection);
		mapShaderToName["skyboxProgram"]->setMat4("view", skyboxView);

		glBindVertexArray(skybox.VAO);
		glBindTexture(GL_TEXTURE_CUBE_MAP, skybox.textureID);
		glDrawArrays(GL_TRIANGLES, 0, 36);
		glDepthFunc(GL_LESS); // set depth function back to default

		//Final pass: Render all of the above on one quad
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glDisable(GL_DEPTH_TEST);
		glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		//Paste the entire scene onto a quad as a single texture
		mapShaderToName["quadProgram"]->useProgram();
		mapShaderToName["quadProgram"]->setInt("drawType", drawType);
		glBindVertexArray(screenQuad.VAO);
		glBindTexture(GL_TEXTURE_2D, mainFrameBuffer.textureID);
		glDrawArrays(GL_TRIANGLES, 0, 6);

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	glfwTerminate();

	return 0;
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	SCR_HEIGHT = height;
	SCR_WIDTH = width;
	glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
	return;
}

void processInput(GLFWwindow* window)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);

	if (!jumpHappening && glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
	{
		jumpHappening = true;
		robaxSelected->oneFrameEvent = true;
		robaxSelected->curAnimState->defaultAnimation.currentTime = 0.0f;
		if (glfwGetKey(window, GLFW_KEY_I))
		{
			if (shiftKey)
			{
				robaxSelected->animToPlay = "assets/modelsFBX/RunningJumpRobaxacet.fbx";
				robaxSelected->oneFrameStandWalkRun = 2;
			}
			else
			{
				robaxSelected->animToPlay = "assets/modelsFBX/StandingJumpRobaxacet.fbx";
				robaxSelected->oneFrameStandWalkRun = 1;
			}
		}
		else
		{
			robaxSelected->animToPlay = "assets/modelsFBX/StandingJumpRobaxacet.fbx";
			robaxSelected->oneFrameStandWalkRun = 0;
		}
	}

	if (!jumpHappening && glfwGetKey(window, GLFW_KEY_ENTER) == GLFW_PRESS)
	{
		jumpHappening = true;
		robaxSelected->oneFrameEvent = true;
		robaxSelected->curAnimState->defaultAnimation.currentTime = 0.0f;
		robaxSelected->oneFrameStandWalkRun = 0;
		robaxSelected->animToPlay = "assets/modelsFBX/ThrowRobaxacet.fbx";
	}

	if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
	{
		shiftKey = true;
	}
	else if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_RELEASE)
	{
		shiftKey = false;
	}

	if (!jumpHappening && glfwGetKey(window, GLFW_KEY_RIGHT_SHIFT) == GLFW_PRESS)
	{
		if (!shiftLock)
		{
			robaxSelected->animToPlay = "assets/modelsFBX/IdleRobaxacet.fbx";
			if (boyOrMan)
			{
				robaxSelected = robaxBoy;
			}
			else
			{
				robaxSelected = robaxMan;
			}
			boyOrMan = !boyOrMan;
		}
		shiftLock = true;
	}
	else if (glfwGetKey(window, GLFW_KEY_RIGHT_SHIFT) == GLFW_RELEASE)
	{
		shiftLock = false;
	}

	if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS)
	{
		ctrlKey = true;
	}
	else if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_RELEASE)
	{
		ctrlKey = false;
	}


	if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS)
		drawType = 1;
	if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS)
		drawType = 2;
	if (glfwGetKey(window, GLFW_KEY_3) == GLFW_PRESS)
		drawType = 3;
	if (glfwGetKey(window, GLFW_KEY_4) == GLFW_PRESS)
		drawType = 4;
	if (glfwGetKey(window, GLFW_KEY_5) == GLFW_PRESS)
		drawType = 5;

	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
	{
		Camera.processKeyboard(Camera_Movement::FORWARD, deltaTime);
	}
	else if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
	{
		Camera.processKeyboard(Camera_Movement::BACKWARD, deltaTime);
	}		
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		Camera.processKeyboard(Camera_Movement::LEFT, deltaTime);
	else if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		Camera.processKeyboard(Camera_Movement::RIGHT, deltaTime);

	if (!jumpHappening && glfwGetKey(window, GLFW_KEY_I) == GLFW_PRESS)
	{
		robaxSelected->Movement(cSkinnedGameObject::FORWARD, deltaTime, shiftKey, ctrlKey);
		if (shiftKey && !ctrlKey)
		{
			robaxSelected->animToPlay = "assets/modelsFBX/RunningRobaxacet.fbx";
		}
		else if (!ctrlKey)
		{
			robaxSelected->animToPlay = "assets/modelsFBX/WalkingRobaxacet.fbx";
		}
		else
		{
			robaxSelected->animToPlay = "assets/modelsFBX/IdleRobaxacet.fbx";
		}
	}
	else if (!jumpHappening && glfwGetKey(window, GLFW_KEY_K) == GLFW_PRESS)
	{
		robaxSelected->Movement(cSkinnedGameObject::BACKWARD, deltaTime, shiftKey, ctrlKey);
		if (shiftKey && !ctrlKey)
		{
			robaxSelected->animToPlay = "assets/modelsFBX/RunningBackwardRobaxacet.fbx";
		}
		else if (!ctrlKey)
		{
			robaxSelected->animToPlay = "assets/modelsFBX/WalkingBackwardRobaxacet.fbx";
		}
		else
		{
			robaxSelected->animToPlay = "assets/modelsFBX/IdleRobaxacet.fbx";
		}
	}
	else if (!jumpHappening)
	{
		robaxSelected->animToPlay = "assets/modelsFBX/IdleRobaxacet.fbx";
	}	
	if (!jumpHappening && glfwGetKey(window, GLFW_KEY_J) == GLFW_PRESS)
	{
		robaxSelected->Movement(cSkinnedGameObject::LEFT, deltaTime, shiftKey, ctrlKey);
		if (ctrlKey)
		{
			robaxSelected->animToPlay = "assets/modelsFBX/StrafeLeftRobaxacet.fbx";
		}
		else if (glfwGetKey(window, GLFW_KEY_I) != GLFW_PRESS && glfwGetKey(window, GLFW_KEY_K) != GLFW_PRESS)
		{
			robaxSelected->animToPlay = "assets/modelsFBX/LeftTurnRobaxacet.fbx";
		}
	}
	else if (!jumpHappening && glfwGetKey(window, GLFW_KEY_L) == GLFW_PRESS)
	{
		robaxSelected->Movement(cSkinnedGameObject::RIGHT, deltaTime, shiftKey, ctrlKey);
		if (ctrlKey)
		{
			robaxSelected->animToPlay = "assets/modelsFBX/StrafeRightRobaxacet.fbx";
		}
		else if (glfwGetKey(window, GLFW_KEY_I) != GLFW_PRESS && glfwGetKey(window, GLFW_KEY_K) != GLFW_PRESS)
		{
			robaxSelected->animToPlay = "assets/modelsFBX/LeftTurnRobaxacet.fbx";
		}
	}
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
	if (firstMouse) // this bool variable is initially set to true
	{
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
	}

	float xOffset = xpos - lastX;
	float yOffset = lastY - ypos;
	lastX = xpos;
	lastY = ypos;

	Camera.processMouseMovement(xOffset, yOffset, true);
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	Camera.processMouseScroll(yoffset);
}

unsigned int loadCubeMap(std::string directory, std::vector<std::string> faces)
{
	unsigned int textureID;
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

	int width, height, nrChannels;
	for (unsigned int i = 0; i < faces.size(); i++)
	{
		std::string fullPath = directory + faces[i];
		unsigned char *data = SOIL_load_image(fullPath.c_str(), &width, &height, &nrChannels, 0);
		if (data)
		{
			GLenum format;
			if (nrChannels == 1)
				format = GL_RED;
			else if (nrChannels == 3)
				format = GL_RGB;
			else if (nrChannels == 4)
				format = GL_RGBA;

			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
				0, GL_RGB, width, height, 0, format, GL_UNSIGNED_BYTE, data
			);
			SOIL_free_image_data(data);
		}
		else
		{
			std::cout << "Cubemap texture failed to load at path: " << faces[i] << std::endl;
			SOIL_free_image_data(data);
		}
	}
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	return textureID;
}