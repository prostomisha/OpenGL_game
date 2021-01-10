#include "glew.h"
#include "freeglut.h"
#include "glm.hpp"
#include "ext.hpp"
#include <iostream>
#include <cmath>
#include <vector>

#include "Shader_Loader.h"
#include "Render_Utils.h"
#include "Camera.h"

#include "Texture.h"
#include "stb_image.h"

//#define STB_IMAGE_IMPLEMENTATION

GLuint programColor;
GLuint programTexture;
GLuint programSkybox;

GLuint cubemapTexture;

Core::Shader_Loader shaderLoader;

obj::Model shipModel;
obj::Model sphereModel;
obj::Model SkyBox;

glm::vec3 cameraPos = glm::vec3(0, 0, 5);
glm::vec3 cameraDir; // camera forward vector
glm::vec3 cameraSide; // camera up vector
float cameraAngle = 0;

glm::mat4 cameraMatrix, perspectiveMatrix;

glm::vec3 lightDir = glm::normalize(glm::vec3(1.0f, -0.9f, -1.0f));

std::vector<std::string> faces =
{
		"textures/right.png",
		"textures/left.png",
		"textures/top.png",
		"textures/bottom.png",
		"textures/front.png",
		"textures/back.png"
};

glm::quat rotation = glm::quat(1, 0, 0, 0);

std::vector<glm::vec3> planPos;

GLuint textureAsteroid;

static const int NUM_PLANETS = 10;
glm::vec3 planetsPositions[NUM_PLANETS];

float lastX;
float lastY;
float offsetX;
float offsetY;
glm::quat rotationChange;
float angleZ;

void keyboard(unsigned char key, int x, int y)
{
	
	float angleSpeed = 0.1f;
	float moveSpeed = 0.1f;
	switch(key)
	{
	case 'z': angleZ -= 0.1f; break;
	case 'x': angleZ += 0.1f; break;
	case 'w': cameraPos += cameraDir * moveSpeed; break;
	case 's': cameraPos -= cameraDir * moveSpeed; break;
	case 'd': cameraPos += cameraSide * moveSpeed; break;
	case 'a': cameraPos -= cameraSide * moveSpeed; break;
	}
}

void mouse(int x, int y)
{
	offsetX = (x - lastX) * 0.1;
	offsetY = (y - lastY) * 0.1;
	lastX = x;
	lastY = y;

	const float sensitivity = 0.1f;
	offsetX *= sensitivity;
	offsetY *= sensitivity;
}

glm::mat4 createCameraMatrix()
{
	glm::quat quatX = glm::quat(glm::angleAxis(offsetX, glm::vec3(0, 1, 0)));
	glm::quat quatY = glm::quat(glm::angleAxis(offsetY, glm::vec3(1, 0, 0)));
	glm::quat quatZ = glm::quat(glm::angleAxis(angleZ, glm::vec3(0, 0, 1)));

	rotationChange = quatX * quatY * quatZ;

	offsetX = 0;
	offsetY = 0;
	angleZ = 0;

	rotation = rotationChange * rotation;
	rotation = glm::normalize(rotation);

	cameraDir = glm::vec3(glm::inverse(rotation) * glm::vec3(0, 0, -1));
	glm::vec3 up = glm::vec3(0, 1, 0);
	cameraSide = glm::cross(cameraDir, up);

	return Core::createViewMatrixQuat(cameraPos, rotation);
}

GLuint loadCubemap(std::vector<std::string> faces)
{
	unsigned int textureID;
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

	int width, height, nrChannels;
	for (unsigned int i = 0; i < faces.size(); i++)
	{
		unsigned char *data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);
		if (data)
		{
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
				0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data
			);
			stbi_image_free(data);
		}
		else
		{
			std::cout << "Cubemap tex failed to load at path: " << faces[i] << std::endl;
			stbi_image_free(data);
		}
	}
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	return textureID;
}

void drawObjectColor(obj::Model * model, glm::mat4 modelMatrix, glm::vec3 color)
{
	GLuint program = programColor;

	glUseProgram(program);

	glUniform3f(glGetUniformLocation(program, "objectColor"), color.x, color.y, color.z);
	glUniform3f(glGetUniformLocation(program, "lightDir"), lightDir.x, lightDir.y, lightDir.z);

	glm::mat4 transformation = perspectiveMatrix * cameraMatrix * modelMatrix;
	glUniformMatrix4fv(glGetUniformLocation(program, "modelViewProjectionMatrix"), 1, GL_FALSE, (float*)&transformation);
	glUniformMatrix4fv(glGetUniformLocation(program, "modelMatrix"), 1, GL_FALSE, (float*)&modelMatrix);

	Core::DrawModel(model);

	glUseProgram(0);
}

void drawObjectTexture(obj::Model * model, glm::mat4 modelMatrix, GLuint textureId)
{
	GLuint program = programTexture;

	glUseProgram(program);

	glUniform3f(glGetUniformLocation(program, "lightDir"), lightDir.x, lightDir.y, lightDir.z);
	Core::SetActiveTexture(textureId, "textureSampler", program, 0);

	glm::mat4 transformation = perspectiveMatrix * cameraMatrix * modelMatrix;
	glUniformMatrix4fv(glGetUniformLocation(program, "modelViewProjectionMatrix"), 1, GL_FALSE, (float*)&transformation);
	glUniformMatrix4fv(glGetUniformLocation(program, "modelMatrix"), 1, GL_FALSE, (float*)&modelMatrix);

	Core::DrawModel(model);

	glUseProgram(0);
}
void drawSkyBox(obj::Model * model, glm::mat4 modelMatrix, GLuint textureId)
{
	GLuint program = programTexture;

	glUseProgram(program);

	//glUniform3f(glGetUniformLocation(program, "lightDir"), lightDir.x, lightDir.y, lightDir.z);
	Core::SetActiveTexture(textureId, "skybox", program, 0);

	glm::mat4 transformation = perspectiveMatrix * cameraMatrix * modelMatrix;
	glUniformMatrix4fv(glGetUniformLocation(program, "projection"), 1, GL_FALSE, (float*)&transformation);
	glUniformMatrix4fv(glGetUniformLocation(program, "model"), 1, GL_FALSE, (float*)&modelMatrix);
	glUniformMatrix4fv(glGetUniformLocation(program, "view"), 1, GL_FALSE, (float*)&cameraMatrix);

	Core::DrawModel(model);

	glUseProgram(0);
}

void renderScene()
{
	// Update of camera and perspective matrices
	cameraMatrix = createCameraMatrix();
	perspectiveMatrix = Core::createPerspectiveMatrix();

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glClearColor(0.0f, 0.1f, 0.3f, 1.0f);

	glm::mat4 shipInitialTransformation = glm::translate(glm::vec3(0,-0.25f,0)) * glm::rotate(glm::radians(180.0f), glm::vec3(0,1,0)) * glm::scale(glm::vec3(0.25f));
	glm::mat4 shipModelMatrix = glm::translate(cameraPos + cameraDir * 0.5f) * glm::mat4_cast(glm::inverse(rotation)) * shipInitialTransformation;

	//glm::mat4 skyBoxMatrix = glm::translate(glm::vec3(0, 0, 0)) * glm::scale(glm::vec3(10.0f));

	drawObjectColor(&shipModel, shipModelMatrix, glm::vec3(0.6f, 0.1f, 1.0f));
	//drawObjectColor(&SkyBox, skyBoxMatrix, glm::vec3(.0f, .5f, .0f)); // SKYBOX

	//drawSkyBox(&SkyBox, glm::translate(glm::vec3(0.0)), cubemapTexture);
	drawObjectTexture(&sphereModel, glm::translate(glm::vec3(0.0)), textureAsteroid);
	
	for each (glm::vec3 planet in planetsPositions)
	{
		drawObjectColor(&sphereModel, glm::translate(planet), glm::vec3(0.5f, 0.8f, 0.3f));
	}

	glutSwapBuffers();
}

void init()
{
	srand(time(0));
	glEnable(GL_DEPTH_TEST);
	programColor = shaderLoader.CreateProgram("shaders/shader_color.vert", "shaders/shader_color.frag");
	programTexture = shaderLoader.CreateProgram("shaders/shader_tex.vert", "shaders/shader_tex.frag");

	sphereModel = obj::loadModelFromFile("models/sphere.obj");
	shipModel = obj::loadModelFromFile("models/spaceship.obj");
	SkyBox = obj::loadModelFromFile("models/skybox.obj");
	
	textureAsteroid = Core::LoadTexture("textures/a.jpg");
	cubemapTexture = loadCubemap(faces);

	for (int i = 0; i < NUM_PLANETS; i++)
	{
		planetsPositions[i] = glm::ballRand(18.0f);
	}
}

void shutdown()
{
	shaderLoader.DeleteProgram(programColor);
	shaderLoader.DeleteProgram(programTexture);
}

void idle()
{
	glutPostRedisplay();
}

int main(int argc, char ** argv)
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA);
	glutInitWindowPosition(200, 200);
	glutInitWindowSize(600, 600);
	glutCreateWindow("OpenGL Pierwszy Program");
	glewInit();

	init();
	glutKeyboardFunc(keyboard);
	glutPassiveMotionFunc(mouse);
	glutDisplayFunc(renderScene);
	glutIdleFunc(idle);

	glutMainLoop();

	shutdown();

	return 0;
}
