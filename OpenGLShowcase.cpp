#include <GL/gl3w.h>

#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <iostream>
#include <vector>
#include <string>
#include <math.h>
#include <cmath>

using namespace std;
#define _USE_MATH_DEFINES
#include "error.h"
#include "obj.h"
#include "shader.h"
#include "texture.h"
#include "test.h"
#include "camera.h"
#include "Model.h"
#include "shadow.h"
#include "point.h"
#include "casteljau.h"

// Width, height of the window and of the shadow cubemap texture
const int width = 1920;
const int height = 1080;
const int sh_width = 2048;
const int sh_height = 2048;

// The currect vertex of the curve the jet is on
int currentPosition = 0;

// The last frames mouse position, starts in the center of the screen
int lastMouseX = width / 2;
int lastMouseY = height / 2;

// vectors for storing curve info
std::vector<point> ctrl_points;
std::vector<point> curve;

// VAOs and VBOs for the curve
#define NUM_BUFFERS 2
#define NUM_VAOS 2
GLuint CurveBuffers[NUM_BUFFERS];
GLuint CurveVAOs[NUM_VAOS];

// Curve info variables
int num_ctrl_verts = 0;
int num_ctrl_floats = 0;

int num_curve_verts = 0;
int num_curve_floats = 0;

// The current camera the user is on
int CurrentCameraIndex = 0;

// Time since last frame
float deltaTime;

// Texture for the rain particle
GLuint RainDropTexture;





void SizeCallback(GLFWwindow* window, int w, int h)
{
	glViewport(0, 0, w, h);
}

// Particle Struct used to store data for the rain particles
struct Particle
{
	glm::vec3 position, speed;
	unsigned char r, g, b, a;
	float size, angle, weight;
	float life;
	float cameraDistance;

	bool operator<(const Particle& that) const {
		// Overwrites the < operation allowing particles to be sorted using std::sort based on distance to camera
		return this->cameraDistance > that.cameraDistance;
	}
};

// Max number of particles in the scene at once
const int MaxParticles = 10000;

// Array of all particles int the scene
Particle ParticleArray[MaxParticles];

// Function to sort the particles back to front
void SortParticles()
{
	std::sort(&ParticleArray[0], &ParticleArray[MaxParticles]);
}

// The last particle that was used
int LastParticle = 0;


// Finds a particle which is dead so it can be replaced
int FindUnusedParticle()
{
	for (int i = LastParticle; i < MaxParticles; i++)
	{
		if (ParticleArray[i].life < 0)
		{
			LastParticle = i;
			return i;
		}
	}

	for (int i = 0; i < LastParticle; i++)
	{
		if (ParticleArray[i].life < 0)
		{
			LastParticle = i;
			return i;
		}
	}

	return 0;
}

// Returns a random float from min to max
float random_float(float min, float max) 
{
	return ((float)rand() / RAND_MAX) * (max - min) + min;
}

// Texture file paths for the skybox
vector<std::string> skybox_faces
{
	"skybox/px.png",
	"skybox/nx.png",
	"skybox/py.png",
	"skybox/ny.png",
	"skybox/pz.png",
	"skybox/nz.png"
};

// Vertex positions for the sykbox
float skyboxVertices[] = {        
	-100.0f,  100.0f, -100.0f,
	-100.0f, -100.0f, -100.0f,
	 100.0f, -100.0f, -100.0f,
	 100.0f, -100.0f, -100.0f,
	 100.0f,  100.0f, -100.0f,
	-100.0f,  100.0f, -100.0f,

	-100.0f, -100.0f,  100.0f,
	-100.0f, -100.0f, -100.0f,
	-100.0f,  100.0f, -100.0f,
	-100.0f,  100.0f, -100.0f,
	-100.0f,  100.0f,  100.0f,
	-100.0f, -100.0f,  100.0f,

	 100.0f, -100.0f, -100.0f,
	 100.0f, -100.0f,  100.0f,
	 100.0f,  100.0f,  100.0f,
	 100.0f,  100.0f,  100.0f,
	 100.0f,  100.0f, -100.0f,
	 100.0f, -100.0f, -100.0f,

	-100.0f, -100.0f,  100.0f,
	-100.0f,  100.0f,  100.0f,
	 100.0f,  100.0f,  100.0f,
	 100.0f,  100.0f,  100.0f,
	 100.0f, -100.0f,  100.0f,
	-100.0f, -100.0f,  100.0f,

	-100.0f,  100.0f, -100.0f,
	 100.0f,  100.0f, -100.0f,
	 100.0f,  100.0f,  100.0f,
	 100.0f,  100.0f,  100.0f,
	-100.0f,  100.0f,  100.0f,
	-100.0f,  100.0f, -100.0f,

	-100.0f, -100.0f, -100.0f,
	-100.0f, -100.0f,  100.0f,
	 100.0f, -100.0f, -100.0f,
	 100.0f, -100.0f, -100.0f,
	-100.0f, -100.0f,  100.0f,
	 100.0f, -100.0f,  100.0f
};

// Creation of the 3 cameras
SCamera Camera;
SCamera CameraJet;
SCamera CameraJetFollow;

// The current camera the user is on
SCamera* CurrentCamera;

// Storage for controllable jet
Jet2 jet2;

// the position of the main light source
glm::vec3 lightPos;

// The current numer of particles in the scene
int NumOfParticle = 0;

// The current mode the user is in (camera)
int CurrentCameraMode = 0;

// The current rotation of the controllable jet
float currentRot = 0;


void processKeyboard(GLFWwindow* window)
{
	bool cam_changed = false;
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
	{
		if (CurrentCameraMode == 2)
		{
			jet2.position += CameraJetFollow.Front * 0.01f;
		}
		else if (CurrentCameraMode == 0)
		{
			Camera.Position += Camera.Front * Camera.MovementSpeed;
		}
		cam_changed = true;
	}
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
	{
		if (CurrentCameraMode == 2)
		{
			CameraJetFollow.Yaw -= 0.1;
			currentRot -= 0.1;
		}
		else if (CurrentCameraMode == 0)
		{
			Camera.Position -= Camera.Right * Camera.MovementSpeed;
		}
		cam_changed = true;
	}
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
	{
		if (CurrentCameraMode == 0)
		{
			Camera.Position -= Camera.Front * Camera.MovementSpeed;
		}
		
		cam_changed = true;
	}
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
	{
		if (CurrentCameraMode == 2)
		{
			CameraJetFollow.Yaw += 0.1;
			currentRot += 0.1;
		}
		else if (CurrentCameraMode == 0)
		{
			Camera.Position += Camera.Right * Camera.MovementSpeed;
		}
		
		
		cam_changed = true;
	}
	if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
	{
		CameraJetFollow.Pitch += 0.1;
		cam_changed = true;
	}
	if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
	{
		CameraJetFollow.Pitch -= 0.1;
		cam_changed = true;
	}
	if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
	{
		lightPos = Camera.Position;
	}
	if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS)
	{
		CurrentCamera = &Camera;
		CurrentCameraMode = 0;
	}
	if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS)
	{
		CurrentCamera = &CameraJet;
		CurrentCameraMode = 1;
	}
	if (glfwGetKey(window, GLFW_KEY_3) == GLFW_PRESS)
	{
		CurrentCamera = &CameraJetFollow;
		CurrentCameraMode = 2;
	}
	if (cam_changed)
	{
		MoveAndOrientCamera(Camera);
	}
}

void processMouse(GLFWwindow* window, double xpos, double ypos)
{
	if (CurrentCameraMode != 0)
	{
		return;
	}
	// Use the difference in mouse position to alter the pitch and yaw of the camera
	float xOffset = xpos - lastMouseX;
	float yOffset = lastMouseY - ypos;
	lastMouseX = xpos;
	lastMouseY = ypos;

	const float sensitivity = 0.1f;
	xOffset *= sensitivity;
	yOffset *= sensitivity;

	// Use these mouse inputs to change the camera rotation
	ProcessMouseInputs(Camera, xOffset, yOffset);
	MoveAndOrientCamera(Camera);

}


// Add a new control point to the curve and then recalculate it
void AddNewPoint(float x, float y, float z)
{
	ctrl_points.push_back(point(x, y, z));
	curve = EvaluateBezierCurve(ctrl_points, 64);

	num_curve_verts = 0;
	num_curve_floats = 0;
	float* curve_vertices = MakeFloatsFromVector(curve, num_curve_verts, num_curve_floats, 0.f, 0.f, 0.f);

	num_ctrl_verts = 0;
	num_ctrl_floats = 0;
	float* ctrl_vertices = MakeFloatsFromVector(ctrl_points, num_ctrl_verts, num_ctrl_floats, 0.f, 0.f, 0.f);


	glCreateBuffers(NUM_BUFFERS, CurveBuffers);
	glGenVertexArrays(NUM_VAOS, CurveVAOs);

	glBindVertexArray(CurveVAOs[0]);
	glNamedBufferStorage(CurveBuffers[0], sizeof(float) * num_curve_floats, curve_vertices, 0);
	glBindBuffer(GL_ARRAY_BUFFER, CurveBuffers[0]);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, (6 * sizeof(float)), (void*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, (6 * sizeof(float)), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);

	glBindVertexArray(CurveVAOs[1]);
	glNamedBufferStorage(CurveBuffers[1], sizeof(float) * num_ctrl_floats, ctrl_vertices, 0);
	glBindBuffer(GL_ARRAY_BUFFER, CurveBuffers[1]);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, (6 * sizeof(float)), (void*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, (6 * sizeof(float)), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);
}

// Render a series of objcts
void render(GLuint currentShader, vector<ModelClass*> objectsToRender, unsigned int depthCubemap, glm::vec3 lightPos, glm::vec3 camPos)
{
	// near and far plane of the camrea
	float near_plane = 1.0, far_plane = 100.5f;
	// switch to required shader
	glUseProgram(currentShader);
	// Calculate the view and projection matrix
	glm::mat4 view = glm::mat4(1.f);
	view = glm::lookAt(CurrentCamera->Position, CurrentCamera->Position + CurrentCamera->Front, CurrentCamera->Up);
	glUniformMatrix4fv(glGetUniformLocation(currentShader, "view"), 1, GL_FALSE, glm::value_ptr(view));

	glm::mat4 projection = glm::mat4(1.f);
	projection = glm::perspective(glm::radians(45.f), (float)width / (float)height, .01f, 10000.f);
	glUniformMatrix4fv(glGetUniformLocation(currentShader, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

	// Pass in all the required uniforms
	glUniform3fv(glGetUniformLocation(currentShader, "lightPos"), 1, value_ptr(lightPos));
	glUniform3fv(glGetUniformLocation(currentShader, "camPos"), 1, value_ptr(CurrentCamera->Position));
	glUniform1f(glGetUniformLocation(currentShader, "far_plane"), far_plane);
	glUniform1f(glGetUniformLocation(currentShader, "time"), (float)glfwGetTime() / 2);

	// Pass in the cubemap texture used for the shadows
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_CUBE_MAP, depthCubemap);
	// For each object in the list get its texture, bind it then get the model transform and pass that in aswell
	for (int i = 0; i < objectsToRender.size(); i++)
	{
		for (int j = 0; j < objectsToRender[i]->currentObjs.size(); j++)
		{
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, objectsToRender[i]->currentObjs[j].texture);
			glBindVertexArray(objectsToRender[i]->currentObjs[j].VAO);

			glm::mat4 model = glm::mat4(1.f);
			objectsToRender[i]->Update();
			objectsToRender[i]->Model(&model);

			glUniformMatrix4fv(glGetUniformLocation(currentShader, "model"), 1, GL_FALSE, glm::value_ptr(model));
			glDrawArrays(GL_TRIANGLES, 0, (objectsToRender[i]->currentObjs[j].tris.size() * 3));
		}
	}

	glBindVertexArray(0);
}

// A simpler model drawing function designed to be used with the shadow shaders used to generate the shadow cubemap
void drawModelsBare(GLuint shadowShaderProgram, vector<ModelClass*> objectsToRender, vector<ModelClass*> refractiveObjects)
{
	for (int i = 0; i < objectsToRender.size(); i++)
	{
		for (int j = 0; j < objectsToRender[i]->currentObjs.size(); j++)
		{
			glBindVertexArray(objectsToRender[i]->currentObjs[j].VAO);

			glm::mat4 model = glm::mat4(1.f);
			objectsToRender[i]->Model(&model);

			glUniformMatrix4fv(glGetUniformLocation(shadowShaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
			glDrawArrays(GL_TRIANGLES, 0, (objectsToRender[i]->currentObjs[j].tris.size() * 3));
		}
	}

	for (int i = 0; i < refractiveObjects.size(); i++)
	{
		for (int j = 0; j < refractiveObjects[i]->currentObjs.size(); j++)
		{
			glBindVertexArray(refractiveObjects[i]->currentObjs[j].VAO);

			glm::mat4 model = glm::mat4(1.f);
			refractiveObjects[i]->Model(&model);

			glUniformMatrix4fv(glGetUniformLocation(shadowShaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
			glDrawArrays(GL_TRIANGLES, 0, (refractiveObjects[i]->currentObjs[j].tris.size() * 3));
		}
	}
}

// Function used to smoothly transition between one value to another
float lint1(float a, float b, float f) {
	return (a * (1.0f - f)) + (b * f);
}

// Function to get the distance between two points in 3 dimensional space
float EuclidianDistance(int x1, int x2, int y1, int y2, int z1, int z2)
{
	return sqrt(pow((x2 - x1), 2) + pow((y2 - y1), 2) + pow((z2 - z1), 2));
}

int main(int argc, char** argv)
{
	// Set the random seed for the scene 
	srand((unsigned int)time(NULL));

	// Set the default camera
	CurrentCamera = &Camera;

	// Set up opengl and the window
	glfwInit();
	GLFWwindow* window = glfwCreateWindow(width, height, "Assessment 2", NULL, NULL);
	glfwMakeContextCurrent(window);
	glfwSetWindowSizeCallback(window, SizeCallback);

	gl3wInit();

	glEnable(GL_DEBUG_OUTPUT);
	glDebugMessageCallback(DebguMessageCallback, 0);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// Compile all the shaders 
	GLuint shaderProgram = CompileShader("textured.vert", "textured.frag");
	GLuint shadowProgram = CompileShaderWithGeometry("shadow.vert", "shadow.frag", "shadow.geo");
	GLuint skyboxProgram = CompileShader("skybox.vert", "skybox.frag");
	GLuint refractionProgram = CompileShader("refraction.vert", "refraction.frag");
	GLuint lineProgram = CompileShader("mvp.vert", "col.frag");
	GLuint particleProgram = CompileShader("particle.vert", "particle.frag");

	// Setup CubeMaps

	// Cubemap for the shadow using the depth buffer
	unsigned int depthFrameBuffer;
	glGenFramebuffers(1, &depthFrameBuffer);
	unsigned int depthCubemap;
	glGenTextures(1, &depthCubemap);
	glBindTexture(GL_TEXTURE_CUBE_MAP, depthCubemap);

	for (unsigned int i = 0; i < 6; ++i)
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_DEPTH_COMPONENT, sh_width, sh_height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);

	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	glBindFramebuffer(GL_FRAMEBUFFER, depthFrameBuffer);
	glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, depthCubemap, 0);
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	
	// Setup the texture IDS for the main shader

	glUseProgram(shaderProgram);
	glUniform1i(glGetUniformLocation(shaderProgram, "Texture"), 0);
	glUniform1i(glGetUniformLocation(shaderProgram, "depthMap"), 1);
	glUniform1i(glGetUniformLocation(shaderProgram, "skybox"), 2);

	// Load Skybox

	glUseProgram(skyboxProgram);
	glUniform1i(glGetUniformLocation(skyboxProgram, "skybox"), 0);
	unsigned int skyboxTextureID;
	glGenTextures(1, &skyboxTextureID);
	glBindTexture(GL_TEXTURE_CUBE_MAP, skyboxTextureID);

	// Load each texture into the frame buffer
	int widthSkybox, heightSkybox, nrChannels;
	for (unsigned int i = 0; i < skybox_faces.size(); i++)
	{
		unsigned char* data = stbi_load(skybox_faces[i].c_str(), &widthSkybox, &heightSkybox, &nrChannels, 0);
		if (data)
		{
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, widthSkybox, heightSkybox, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
			stbi_image_free(data);
		}
		else
		{
			stbi_image_free(data);
		}
	}
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	// Setup VAO and VBO for the skybox verticies
	unsigned int skyboxVAO, skyboxVBO;
	glGenVertexArrays(1, &skyboxVAO);
	glGenBuffers(1, &skyboxVBO);
	glBindVertexArray(skyboxVAO);
	glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);




	// Bind the skybox to the main shader
	glUseProgram(shaderProgram);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_CUBE_MAP, skyboxTextureID);


	// Add all the control points for the bezier curve
	ctrl_points.push_back(point(29.5589,26.6515,-42.9413));
	ctrl_points.push_back(point(-3.89368,23.6132,-51.7317));
	ctrl_points.push_back(point(-42.6319,15.3011,-48.2232));
	ctrl_points.push_back(point(-51.7069,12.8396,-25.1201));
	ctrl_points.push_back(point(-47.2062,11.4175,17.6717));
	ctrl_points.push_back(point(-21.0258,11.7184,29.5942));
	ctrl_points.push_back(point(26.27,11.0934,30.3668));
	ctrl_points.push_back(point(32.2284,11.0458,6.46358));
	ctrl_points.push_back(point(37.7514,10.7317,-36.4149));
	ctrl_points.push_back(point(21.9158,10.8013,-49.1179));
	ctrl_points.push_back(point(1.96139,14.6398,-31.09));
	ctrl_points.push_back(point(-21.1128,19.7135,-8.995498));
	ctrl_points.push_back(point(-43.1165,16.0796,18.109));
	ctrl_points.push_back(point(-37.2635,12.4721,32.1324));
	ctrl_points.push_back(point(-7.45327,9.69435,30.3958));
	ctrl_points.push_back(point(18.7696,4.40373,8.99205));
	ctrl_points.push_back(point(16.2903,6.95968,-2.83717));
	ctrl_points.push_back(point(-2.32754,22.9729,-20.7175));
	ctrl_points.push_back(point(-15.6068,26.1019,-25.035));
	ctrl_points.push_back(point(-50.8069,31.5542,-21.8425));
	ctrl_points.push_back(point(-46.9894,34.5714,22.6104));
	ctrl_points.push_back(point(-17.4026,31.3315,35.5461));
	ctrl_points.push_back(point(18.6105,32.955,36.9977));
	ctrl_points.push_back(point(31.5485,29.6746,13.3615));
	ctrl_points.push_back(point(37.333,28.9693,-13.2803));
	ctrl_points.push_back(point(30.8981,26.825,-42.58691));
	
	// Generate bezier curve from the control points
	curve = EvaluateBezierCurve(ctrl_points, 128);

	num_curve_verts = 0;
	num_curve_floats = 0;
	float* curve_vertices = MakeFloatsFromVector(curve, num_curve_verts, num_curve_floats, 0.f, 0.f, 0.f);

	num_ctrl_verts = 0;
	num_ctrl_floats = 0;
	float* ctrl_vertices = MakeFloatsFromVector(ctrl_points, num_ctrl_verts, num_ctrl_floats, 0.f, 0.f, 0.f);

	// Generate VAO and VBO to store the verticies for the curve
	glCreateBuffers(NUM_BUFFERS, CurveBuffers);
	glGenVertexArrays(NUM_VAOS, CurveVAOs);

	glBindVertexArray(CurveVAOs[0]);
	glNamedBufferStorage(CurveBuffers[0], sizeof(float) * num_curve_floats, curve_vertices, 0);
	glBindBuffer(GL_ARRAY_BUFFER, CurveBuffers[0]);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, (6 * sizeof(float)), (void*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, (6 * sizeof(float)), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);

	glBindVertexArray(CurveVAOs[1]);
	glNamedBufferStorage(CurveBuffers[1], sizeof(float) * num_ctrl_floats, ctrl_vertices, 0);
	glBindBuffer(GL_ARRAY_BUFFER, CurveBuffers[1]);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, (6 * sizeof(float)), (void*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, (6 * sizeof(float)), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);

	// Setup Cameras

	InitCamera(Camera);
	MoveAndOrientCamera(Camera);
	InitCamera(CameraJet);
	InitCamera(CameraJetFollow);
	CameraJetFollow.Yaw = 90;
	
	CameraJetFollow.Front = glm::vec3(0, 0, 1);

	// Setup Particles

	RainDropTexture = CreateTexture("objs/raindrop.png");

	GLfloat* particlePositionSizeData = new GLfloat[MaxParticles * 4];
	GLubyte* particleColourData = new GLubyte[MaxParticles * 4];

	// Instantiate each particle to be dead initially
	for (int i = 0; i < MaxParticles; i++)
	{
		ParticleArray[i].life = -1.f;
		ParticleArray[i].cameraDistance = -1.f;
	}

	// Vertex data for the particle billboard
	static const GLfloat particleVertexData[] = {
		 -0.5f, -0.5f, 0.0f,
		 0.5f, -0.5f, 0.0f,
		 -0.5f, 0.5f, 0.0f,
		 0.5f, 0.5f, 0.0f,
	};

	// Generate all the buffers which will be passed to the GPU
	GLuint particleVertexBuffer;
	glGenBuffers(1, &particleVertexBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, particleVertexBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(particleVertexData), particleVertexData, GL_STATIC_DRAW);

	GLuint particlePositions;
	glGenBuffers(1, &particlePositions);
	glBindBuffer(GL_ARRAY_BUFFER, particlePositions);
	glBufferData(GL_ARRAY_BUFFER, MaxParticles * 4 * sizeof(GLfloat), NULL, GL_STREAM_DRAW);

	GLuint particleColours;
	glGenBuffers(1, &particleColours);
	glBindBuffer(GL_ARRAY_BUFFER, particleColours);
	glBufferData(GL_ARRAY_BUFFER, MaxParticles * 4 * sizeof(GLubyte), NULL, GL_STREAM_DRAW);



	// Create all objects and the lists to hold them
	vector<ModelClass*> objectsToRender;
	vector<ModelClass*> refractiveObjects;
	vector<ModelClass*> shinyObjects;
	Sphere sphere;
	BumpyPlane plane;
	Carrier boat;
	Ground ground;
	Jet jet;
	Buoy buoy1;
	Buoy buoy2;
	Border border;

	shinyObjects.push_back(&sphere);
	objectsToRender.push_back(&boat);
	objectsToRender.push_back(&ground);
	objectsToRender.push_back(&jet);
	objectsToRender.push_back(&jet2);
	objectsToRender.push_back(&buoy1);
	objectsToRender.push_back(&buoy2);
	objectsToRender.push_back(&border);
	buoy2.position = glm::vec3(8.55814, -1.85559, 28.6543);
	jet2.position = glm::vec3(-0.55959, -1.50188, -7.25146);
	refractiveObjects.push_back(&plane);

	// Parse and generate textures and buffers for main objects
	for (int i = 0; i < objectsToRender.size(); i++)
	{
		objectsToRender[i]->ParseObj();
	}

	for (int i = 0; i < objectsToRender.size(); i++)
	{
		for (int j = 0; j < objectsToRender[i]->currentObjs.size(); j++)
		{
			objectsToRender[i]->currentObjs[j].texture = CreateTexture(objectsToRender[i]->currentObjs[j].mtl.fil_name);

			glGenVertexArrays(1, &objectsToRender[i]->currentObjs[j].VAO);
			glGenBuffers(1, &objectsToRender[i]->currentObjs[j].VBO);

			glBindVertexArray(objectsToRender[i]->currentObjs[j].VAO);
			glBindBuffer(GL_ARRAY_BUFFER, objectsToRender[i]->currentObjs[j].VBO);
			glBufferData(GL_ARRAY_BUFFER, sizeof(float) * (objectsToRender[i]->currentObjs[j].tris.size() * 24), objectsToRender[i]->currentObjs[j].tris.data(), GL_STATIC_DRAW);
			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
			glEnableVertexAttribArray(0);
			glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
			glEnableVertexAttribArray(1);
			glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(5 * sizeof(float)));
			glEnableVertexAttribArray(2);
		}
	}

	// Parse and generate textures and buffers for reflective objects (water)

	for (int i = 0; i < refractiveObjects.size(); i++)
	{
		refractiveObjects[i]->ParseObj();
	}

	for (int i = 0; i < refractiveObjects.size(); i++)
	{
		for (int j = 0; j < refractiveObjects[i]->currentObjs.size(); j++)
		{
			refractiveObjects[i]->currentObjs[j].texture = CreateTexture(refractiveObjects[i]->currentObjs[j].mtl.fil_name);

			glGenVertexArrays(1, &refractiveObjects[i]->currentObjs[j].VAO);
			glGenBuffers(1, &refractiveObjects[i]->currentObjs[j].VBO);

			glBindVertexArray(refractiveObjects[i]->currentObjs[j].VAO);
			glBindBuffer(GL_ARRAY_BUFFER, refractiveObjects[i]->currentObjs[j].VBO);
			glBufferData(GL_ARRAY_BUFFER, sizeof(float) * (refractiveObjects[i]->currentObjs[j].tris.size() * 24), refractiveObjects[i]->currentObjs[j].tris.data(), GL_STATIC_DRAW);
			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
			glEnableVertexAttribArray(0);
			glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
			glEnableVertexAttribArray(1);
			glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(5 * sizeof(float)));
			glEnableVertexAttribArray(2);
		}
	}

	// Parse and generate textures and buffers for shiny objects (procedurally generated sphere)

	for (int i = 0; i < shinyObjects.size(); i++)
	{
		shinyObjects[i]->ParseObj();
	}

	for (int i = 0; i < shinyObjects.size(); i++)
	{
		for (int j = 0; j < shinyObjects[i]->currentObjs.size(); j++)
		{
			shinyObjects[i]->currentObjs[j].texture = CreateTexture(shinyObjects[i]->currentObjs[j].mtl.fil_name);

			glGenVertexArrays(1, &shinyObjects[i]->currentObjs[j].VAO);
			glGenBuffers(1, &shinyObjects[i]->currentObjs[j].VBO);

			glBindVertexArray(shinyObjects[i]->currentObjs[j].VAO);
			glBindBuffer(GL_ARRAY_BUFFER, shinyObjects[i]->currentObjs[j].VBO);
			glBufferData(GL_ARRAY_BUFFER, sizeof(float) * (shinyObjects[i]->currentObjs[j].tris.size() * 24), shinyObjects[i]->currentObjs[j].tris.data(), GL_STATIC_DRAW);
			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
			glEnableVertexAttribArray(0);
			glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
			glEnableVertexAttribArray(1);
			glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(5 * sizeof(float)));
			glEnableVertexAttribArray(2);
		}
	}
	
	// Setup input methods for opengl
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	glfwSetCursorPosCallback(window, processMouse);

	// Set the initial position of the light
	lightPos = glm::vec3(0.f, 2.f, 0.f);

	jet.position = glm::vec3(ctrl_points[0].x, ctrl_points[0].y, ctrl_points[0].z);
	float oldX = jet.position.x;
	float oldY = jet.position.y;
	float oldZ = jet.position.z;

	float prevTime = glfwGetTime();
	float lastTime = glfwGetTime();

	float dist = EuclidianDistance(oldX, curve[currentPosition + 1].x, oldY, curve[currentPosition + 1].y, oldZ, curve[currentPosition + 1].z);
	float speed = 1;
	float duration = speed / dist;
	float t = 0;
	float currentTime = 0;
	float nextPosIndex = 1;

	while (!glfwWindowShouldClose(window))
	{
		processKeyboard(window);

		float currentTime = glfwGetTime();
		deltaTime = currentTime - lastTime;
		lastTime = currentTime;

		if (currentRot > 360)
		{
			currentRot = 0;
		}
		if (currentRot < 0)
		{
			currentRot = 360;
		}

		CameraJetFollow.Position = jet2.position - (CameraJetFollow.Front * 5.f) + glm::vec3(0, 1, 0);
		MoveAndOrientCamera(CameraJetFollow);
		jet2.nextPos = CameraJetFollow.Front;
		jet2.up = CameraJetFollow.Up;
		jet2.pitch = CameraJetFollow.Pitch;
		jet2.yaw = currentRot;
		CameraJet.Position = jet.position - glm::vec3(0,2,0);
		glm::vec3 directionToCarrier = boat.position - jet.position;
		CameraJet.Front = glm::normalize(directionToCarrier);
		
		//MoveAndOrientCamera(CameraJet);
		

		glClearColor(1.f, 1.f, 1.f, 1.f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


		float near_plane = 1.0, far_plane = 100.5f;
		glm::mat4 lightProjection = glm::perspective(glm::radians(90.0f), (float)sh_width / (float)sh_height, near_plane, far_plane);
		vector<glm::mat4> lightDirections;
		lightDirections.push_back(lightProjection * glm::lookAt(lightPos, lightPos + glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)));
		lightDirections.push_back(lightProjection * glm::lookAt(lightPos, lightPos + glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)));
		lightDirections.push_back(lightProjection * glm::lookAt(lightPos, lightPos + glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f)));
		lightDirections.push_back(lightProjection * glm::lookAt(lightPos, lightPos + glm::vec3(0.0f, -1.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f)));
		lightDirections.push_back(lightProjection * glm::lookAt(lightPos, lightPos + glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, -1.0f, 0.0f)));
		lightDirections.push_back(lightProjection * glm::lookAt(lightPos, lightPos + glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, -1.0f, 0.0f)));

		// Render to cubemap
		glViewport(0, 0, sh_width, sh_height);
		glBindFramebuffer(GL_FRAMEBUFFER, depthFrameBuffer);
		glClear(GL_DEPTH_BUFFER_BIT);
		glUseProgram(shadowProgram);
		for (int i = 0; i < 6; ++i)
		{
			std::string temp = "lightMatrix[" + std::to_string(i) + "]";
			glUniformMatrix4fv(glGetUniformLocation(shadowProgram, temp.c_str()), 1, GL_FALSE, glm::value_ptr(lightDirections[i]));
		}
		glUniform1f(glGetUniformLocation(shadowProgram, "far_plane"), far_plane);
		glUniform3fv(glGetUniformLocation(shadowProgram, "lightPos"), 1, value_ptr(lightPos));

		drawModelsBare(shadowProgram, objectsToRender, refractiveObjects);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);


		// Do Particle Calculations

		int newparticles = (int)(deltaTime * 10000.0);;
		if (newparticles > (int)(0.016f * 1000.0))
			newparticles = (int)(0.016f * 1000.0);

		//std::cout << "New Particles " << newparticles << std::endl;

		for (int i = 0; i < newparticles; i++) {
			int particleIndex = FindUnusedParticle();
			//std::cout << particleIndex << std::endl;
			ParticleArray[particleIndex].life = 3.5f; // This particle will live 5 seconds.
			ParticleArray[particleIndex].position = glm::vec3(CurrentCamera->Position.x + random_float(-50, 50), 5, CurrentCamera->Position.z + random_float(-50, 50));


			ParticleArray[particleIndex].speed = glm::vec3(0,-3,0);


			// Very bad way to generate a random color
			ParticleArray[particleIndex].r = 256;
			ParticleArray[particleIndex].g = 256;
			ParticleArray[particleIndex].b = 256;
			ParticleArray[particleIndex].a = 256 / 3;

			ParticleArray[particleIndex].size = (rand() % 1000) / 2000.0f + 0.1f;
		}

		NumOfParticle = 0;
		for (int i = 0; i < MaxParticles; i++) {

			Particle& p = ParticleArray[i]; // shortcut

			if (p.life > 0.0f) {

				// Decrease life
				p.life -= deltaTime;
				if (p.life > 0.0f) {
	
					
					// Simulate simple physics : gravity only, no collisions
					//p.speed += glm::vec3(0.0f, -9.81f, 0.0f) * (float)deltaTime * 0.5f;
					p.position += p.speed * (float)deltaTime;
					p.cameraDistance = glm::pow(glm::length(p.position - CurrentCamera->Position),2);
					//ParticlesContainer[i].pos += glm::vec3(0.0f,10.0f, 0.0f) * (float)delta;

					// Fill the GPU buffer
					particlePositionSizeData[4 * NumOfParticle + 0] = p.position.x;
					particlePositionSizeData[4 * NumOfParticle + 1] = p.position.y;
					particlePositionSizeData[4 * NumOfParticle + 2] = p.position.z;

					if (i == 300)
					{
						//std::cout << "Pos x " << p.position.x << std::endl;
						//std::cout << "Pos y " << p.position.y << std::endl;
						//std::cout << "Pos z " << p.position.z << std::endl;
						//std::cout << "life " << p.life << std::endl;

					}
					particlePositionSizeData[4 * NumOfParticle + 3] = p.size;

					particleColourData[4 * NumOfParticle + 0] = p.r;
					particleColourData[4 * NumOfParticle + 1] = p.g;
					particleColourData[4 * NumOfParticle + 2] = p.b;
					particleColourData[4 * NumOfParticle + 3] = p.a;

				}
				else {
					// Particles that just died will be put at the end of the buffer in SortParticles();
					p.cameraDistance = -1.0f;
				}

				NumOfParticle++;
			}
		}

		SortParticles();
		//std::cout << "NumOfParticle " << NumOfParticle << std::endl;
		// Plane Movement


		// Get the position of the next curve point
		glm::vec3 nextPos = glm::vec3(curve[nextPosIndex].x, curve[nextPosIndex].y, curve[nextPosIndex].z);
		//// Increment the timer
		t += glfwGetTime() - prevTime;


		////std::cout << jet.position.x << " " << jet.position.y << " " << jet.position.z << std::endl;
		////std::cout << "Current Pos " << t << std::endl;

		//// Find new posiitons using lerp
		float newX = lint1(oldX, nextPos.x, t / duration);
		float newY = lint1(oldY, nextPos.y, t / duration);
		float newZ = lint1(oldZ, nextPos.z, t / duration);
		//// Update jet position
		jet.position = glm::vec3(newX, newY, newZ);
		vector<point> ctrl_points2 = ctrl_points;
		if (curve[curve.size() - 1].x == 0 && curve[curve.size() - 1].y == 0 && curve[curve.size() - 1].z == 0)
		
			curve.pop_back();
		
		if (t >= duration)
		{
		
			// if we have reached the final point
			if (nextPosIndex == curve.size() - 1)
			{
				currentPosition = curve.size() - 1;
				nextPosIndex = 0;
			}
			else if (currentPosition == curve.size() - 1)
			{
				currentPosition = 0;
				nextPosIndex++;
			}
			else
			{
				// If the lerp is finished update the position
				currentPosition++;
				nextPosIndex++;
			}
			
			
			jet.nextPos = glm::normalize(glm::vec3(newX, newY, newZ) - nextPos);
	
			int currentPosTemp = currentPosition;
			int nextPostTemp = nextPosIndex;
			int sizeOfCurve = curve.size();
			glm::vec3 nextPosTest = glm::vec3(curve[nextPosIndex].x, curve[nextPosIndex].y, curve[nextPosIndex].z);
			//std::cout << nextPosTest.x << " " << nextPosTest.y << " " << nextPosTest.z << std::endl;
			oldX = newX;
			oldY = newY;
			oldZ = newZ;
			dist = EuclidianDistance(oldX, curve[nextPosIndex].x, oldY, curve[nextPosIndex].y, oldZ, curve[nextPosIndex].z);
			//std::cout << dist << std::endl;
			if (dist > 100000)
			{
				dist = 1;
			}
			if (dist == 0)
			{
				dist = 1;
			}
			duration = speed / dist;
			prevTime = glfwGetTime();
			t = 0;
		}
		
		



		glViewport(0, 0, width, height);

		glUseProgram(shaderProgram);
		glUniform1i(glGetUniformLocation(shaderProgram, "water"), 0);
		render(shaderProgram, objectsToRender, depthCubemap, lightPos, CurrentCamera->Position);


		render(refractionProgram, shinyObjects, depthCubemap, lightPos, CurrentCamera->Position);


		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glUseProgram(shaderProgram);
		glUniform1i(glGetUniformLocation(shaderProgram, "water"), 1);
		render(shaderProgram, refractiveObjects, depthCubemap, lightPos, CurrentCamera->Position);


		


		glBindFramebuffer(GL_FRAMEBUFFER, 0);


		// Draw Skybox

		glDepthFunc(GL_LEQUAL);
		glUseProgram(skyboxProgram);
		glm::mat4 viewSkybox = glm::mat4(1.f);
		viewSkybox = glm::lookAt(CurrentCamera->Position, CurrentCamera->Position + CurrentCamera->Front, CurrentCamera->Up);
		viewSkybox = glm::mat4(glm::mat3(viewSkybox));
		glUniformMatrix4fv(glGetUniformLocation(skyboxProgram, "view"), 1, GL_FALSE, glm::value_ptr(viewSkybox));

		glm::mat4 projectionSkybox = glm::mat4(1.f);
		projectionSkybox = glm::perspective(glm::radians(45.f), (float)width / (float)height, .01f, 100000.f);
		glUniformMatrix4fv(glGetUniformLocation(skyboxProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projectionSkybox));

		glBindVertexArray(skyboxVAO);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_CUBE_MAP, skyboxTextureID);
		glDrawArrays(GL_TRIANGLES, 0, 36);
		glBindVertexArray(0);
		glDepthFunc(GL_LESS);


		// Render Particles

		glBindBuffer(GL_ARRAY_BUFFER, particlePositions);
		glBufferData(GL_ARRAY_BUFFER, MaxParticles * 4 * sizeof(GLfloat), NULL, GL_STREAM_DRAW);
		glBufferSubData(GL_ARRAY_BUFFER, 0, NumOfParticle * sizeof(GLfloat) * 4, particlePositionSizeData);

		glBindBuffer(GL_ARRAY_BUFFER, particleColours);
		glBufferData(GL_ARRAY_BUFFER, MaxParticles * 4 * sizeof(GLubyte), NULL, GL_STREAM_DRAW);
		glBufferSubData(GL_ARRAY_BUFFER, 0, NumOfParticle * sizeof(GLubyte) * 4, particleColourData);

		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glUseProgram(particleProgram);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, RainDropTexture);
		glUniform1i(glGetUniformLocation(particleProgram, "myTextureSampler"), 0);

		glm::mat4 viewParticle = glm::mat4(1.f);
		viewParticle = glm::lookAt(CurrentCamera->Position, CurrentCamera->Position + CurrentCamera->Front, CurrentCamera->Up);

		glm::mat4 projectionParticle = glm::mat4(1.f);
		projectionParticle = glm::perspective(glm::radians(45.f), (float)width / (float)height, .01f, 10000.f);

		glm::mat4 viewProjectionCombined = projectionParticle * viewParticle;

		glUniform3f(glGetUniformLocation(particleProgram, "CameraRight_worldspace"), CurrentCamera->Right.x, CurrentCamera->Right.y, CurrentCamera->Right.z);
		glUniform3f(glGetUniformLocation(particleProgram, "CameraUp_worldspace"), CurrentCamera->Up.x, CurrentCamera->Up.y, CurrentCamera->Up.z );
		glUniformMatrix4fv(glGetUniformLocation(particleProgram, "VP"), 1, GL_FALSE, &viewProjectionCombined[0][0]);


		glEnableVertexAttribArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, particleVertexBuffer);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

		glEnableVertexAttribArray(1);
		glBindBuffer(GL_ARRAY_BUFFER, particlePositions);
		glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 0, (void*)0);

		glEnableVertexAttribArray(2);
		glBindBuffer(GL_ARRAY_BUFFER, particleColours);
		glVertexAttribPointer(2, 4, GL_UNSIGNED_BYTE, GL_TRUE, 0, (void*)0);

		glVertexAttribDivisor(0, 0);
		glVertexAttribDivisor(1, 1);
		glVertexAttribDivisor(2, 1);

		//std::cout << NumOfParticle << std::endl;
		glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, NumOfParticle);





		if (ctrl_points.size() >= 2)
		{
			glUseProgram(lineProgram);

			glm::mat4 model = glm::mat4(1.f);
			glUniformMatrix4fv(glGetUniformLocation(lineProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
			glm::mat4 view = glm::mat4(1.f);
			view = glm::lookAt(CurrentCamera->Position, CurrentCamera->Position + CurrentCamera->Front, CurrentCamera->Up);
			glUniformMatrix4fv(glGetUniformLocation(lineProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));
			glm::mat4 projection = glm::mat4(1.f);
			projection = glm::perspective(glm::radians(45.f), (float)width / (float)height, .01f, 10000.f);
			glUniformMatrix4fv(glGetUniformLocation(lineProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

			glBindVertexArray(CurveVAOs[0]);
			glDrawArrays(GL_LINE_STRIP, 0, num_curve_verts);

			glBindVertexArray(CurveVAOs[1]);
			glDrawArrays(GL_POINTS, 0, num_ctrl_verts);
		}


		glfwSwapBuffers(window);

		glfwPollEvents();
		processKeyboard(window);
	}

	glfwTerminate();



	return 0;
}

