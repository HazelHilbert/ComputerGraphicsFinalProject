#include "animation.h"
#include "terrain.h"
#include "sky.h"
#include "box.h"
#include "TerrainManager.h"
#include "model.h"

#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/string_cast.hpp>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <render/shader.h>

#include "CityManager.h"
#include "FoxManager.h"


#define _USE_MATH_DEFINES
#define BUFFER_OFFSET(i) ((char *)NULL + (i))

static GLFWwindow *window;
static int windowWidth = 1024;
static int windowHeight = 768;

static void key_callback(GLFWwindow *window, int key, int scancode, int action, int mode);
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
void cursor_position_callback(GLFWwindow* window, double xpos, double ypos);
void updateLightSpaceMatrix();

// Camera
static glm::vec3 eye_center(0, 100, 100);
static glm::vec3 lookat(0, 0, 0);
static glm::vec3 up(0, 1, 0);

static float FoV = 45.0f;
glm::float32 zNear = 0.1f;
glm::float32 zFar = 10000.0f;

static bool mousePressed = false;
static double lastMouseX, lastMouseY;
static float sensitivity = 0.1f;

// Lighting control
const glm::vec3 wave500(0.0f, 255.0f, 146.0f);
const glm::vec3 wave600(255.0f, 190.0f, 0.0f);
const glm::vec3 wave700(205.0f, 0.0f, 0.0f);
//static glm::vec3 lightIntensity = 5.0f * (8.0f * wave500 + 15.6f * wave600 + 18.4f * wave700);
static glm::vec3 lightIntensity(5e6f, 5e6f, 5e6f);

static glm::vec3 lightDirection = glm::normalize(glm::vec3(-275.0f, -275.0f, -275.0f)); // Example direction

// Shadow mapping
static glm::vec3 lightUp(0.0f, 1.0f, 0.0f);
static int shadowMapWidth = 2048;
static int shadowMapHeight = 1536;

glm::mat4 lightSpaceMatrix;

// Define light's orthographic projection parameters
float near_plane = 0.1f, far_plane = 400.0f;
float orthoSize = 150.0f; // should be slightly larger than half the terrain size
glm::mat4 lightProjection = glm::ortho(-orthoSize, orthoSize, -orthoSize, orthoSize, near_plane, far_plane);

// Animation 
static bool playAnimation = false;
static float playbackSpeed = 2.0f;

struct AxisXYZ {
    // A structure for visualizing the global 3D coordinate system

	GLfloat vertex_buffer_data[18] = {
		// X axis
		0.0, 0.0f, 0.0f,
		100.0f, 0.0f, 0.0f,

		// Y axis
		0.0f, 0.0f, 0.0f,
		0.0f, 100.0f, 0.0f,

		// Z axis
		0.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 100.0f,
	};

	GLfloat color_buffer_data[18] = {
		// X, red
		1.0f, 0.0f, 0.0f,
		1.0f, 0.0f, 0.0f,

		// Y, green
		0.0f, 1.0f, 0.0f,
		0.0f, 1.0f, 0.0f,

		// Z, blue
		0.0f, 0.0f, 1.0f,
		0.0f, 0.0f, 1.0f,
	};

	// OpenGL buffers
	GLuint vertexArrayID;
	GLuint vertexBufferID;
	GLuint colorBufferID;

	// Shader variable IDs
	GLuint mvpMatrixID;
	GLuint programID;

	void initialize() {
		// Create a vertex array object
		glGenVertexArrays(1, &vertexArrayID);
		glBindVertexArray(vertexArrayID);

		// Create a vertex buffer object to store the vertex data
		glGenBuffers(1, &vertexBufferID);
		glBindBuffer(GL_ARRAY_BUFFER, vertexBufferID);
		glBufferData(GL_ARRAY_BUFFER, sizeof(vertex_buffer_data), vertex_buffer_data, GL_STATIC_DRAW);

		// Create a vertex buffer object to store the color data
		glGenBuffers(1, &colorBufferID);
		glBindBuffer(GL_ARRAY_BUFFER, colorBufferID);
		glBufferData(GL_ARRAY_BUFFER, sizeof(color_buffer_data), color_buffer_data, GL_STATIC_DRAW);

		// Create and compile our GLSL program from the shaders
		programID = LoadShadersFromFile("../FinalProject/shader/box.vert", "../FinalProject/shader/box.frag");
		if (programID == 0)
		{
			std::cerr << "Failed to load shaders." << std::endl;
		}

		// Get a handle for our "MVP" uniform
		mvpMatrixID = glGetUniformLocation(programID, "MVP");
	}

	void render(glm::mat4 cameraMatrix) {
		glUseProgram(programID);

		glEnableVertexAttribArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, vertexBufferID);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

		glEnableVertexAttribArray(1);
		glBindBuffer(GL_ARRAY_BUFFER, colorBufferID);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);

		// Set model-view-projection matrix
		glm::mat4 mvp = cameraMatrix;
		glUniformMatrix4fv(mvpMatrixID, 1, GL_FALSE, &mvp[0][0]);

        // Draw the lines
        glDrawArrays(GL_LINES, 0, 6);

		glDisableVertexAttribArray(0);
		glDisableVertexAttribArray(1);
	}

	void cleanup() {
		glDeleteBuffers(1, &vertexBufferID);
		glDeleteBuffers(1, &colorBufferID);
		glDeleteVertexArrays(1, &vertexArrayID);
		glDeleteProgram(programID);
	}
};

int main(void)
{
	// Initialise GLFW
	if (!glfwInit())
	{
		std::cerr << "Failed to initialize GLFW." << std::endl;
		return -1;
	}

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // For MacOS
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	// Open a window and create its OpenGL context
	window = glfwCreateWindow(windowWidth, windowHeight, "Toward a Futuristic Emerald Isle", NULL, NULL);
	if (window == NULL)
	{
		std::cerr << "Failed to open a GLFW window." << std::endl;
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);

	// Ensure we can capture the escape key being pressed below
	glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);
	glfwSetKeyCallback(window, key_callback);

	glfwSetMouseButtonCallback(window, mouse_button_callback);
	glfwSetCursorPosCallback(window, cursor_position_callback);
	// hide cursor
	//glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);


	// Load OpenGL functions, gladLoadGL returns the loaded version, 0 on error.
	int version = gladLoadGL(glfwGetProcAddress);
	if (version == 0)
	{
		std::cerr << "Failed to initialize OpenGL context." << std::endl;
		return -1;
	}

	// Background
	glClearColor(0.2f, 0.2f, 0.25f, 0.0f);

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);

	// -------------------------------------
	// Initialize models
	// -------------------------------------

	//MyBot bot;
	//bot.initialize();

	FoxManager foxManager;
	foxManager.initialize();

	AxisXYZ axis;
	axis.initialize();

	Sky sky;
	sky.initialize(glm::vec3(0,-5,0), glm::vec3(100.f,100.0f,100.0f));

	TerrainManager terrainM;
	terrainM.initialize(eye_center);

	CityManager cityManager;
	cityManager.initialize(30);

	// -------------------------------------
	// -------------------------------------

	glm::mat4 viewMatrix, projectionMatrix;
	projectionMatrix = glm::perspective(glm::radians(FoV), (float)windowWidth / windowHeight, zNear, zFar);

	updateLightSpaceMatrix();

	// Time and frame rate tracking
	static double lastTime = glfwGetTime();
	float time = 0.0f;			// Animation time 
	float fTime = 0.0f;			// Time for measuring fps
	unsigned long frames = 0;

	// Main loop
	do
	{
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// Update states for animation
        double currentTime = glfwGetTime();
        float deltaTime = float(currentTime - lastTime);
		lastTime = currentTime;

		if (playAnimation) {
			time += deltaTime * playbackSpeed;
			foxManager.update(time);
		}

		// Rendering
		viewMatrix = glm::lookAt(eye_center, lookat, up);
		glm::mat4 vp = projectionMatrix * viewMatrix;

		glm::mat4 vp_skybox = projectionMatrix * glm::mat4(glm::mat3(viewMatrix));

		const float lightDistance = 200.0f; // Adjust as needed
		const float fixedLightY = 10.0f; // Example value, adjust based on your scene
		glm::vec3 lightPos = glm::vec3(eye_center.x, fixedLightY, eye_center.z) - lightDirection * lightDistance;

		// View Mesh
		//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

		glDepthMask(GL_FALSE);
		sky.render(vp_skybox);
		glDepthMask(GL_TRUE);

		//axis.render(vp);

		// animations need to be rendered right before terrain because some states are set and not properly reset
		if (playAnimation) {
			foxManager.render(vp, lightDirection, lightIntensity);
		}
		glEnable(GL_DEPTH_TEST);


		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		terrainM.render(vp, lightSpaceMatrix, lightDirection, lightIntensity, eye_center);
		cityManager.render(vp,lightDirection, lightIntensity, eye_center);
		glDisable(GL_BLEND);



		// FPS tracking
		// Count number of frames over a few seconds and take average
		frames++;
		fTime += deltaTime;
		if (fTime > 2.0f) {		
			float fps = frames / fTime;
			frames = 0;
			fTime = 0;
			
			std::stringstream stream;
			stream << std::fixed << std::setprecision(2) << "Toward a Futuristic Emerald Isle | FPS: " << fps;
			glfwSetWindowTitle(window, stream.str().c_str());
		}

		// Swap buffers
		glfwSwapBuffers(window);
		glfwPollEvents();

	} // Check if the ESC key was pressed or the window was closed
	while (!glfwWindowShouldClose(window));

	// Clean up
	axis.cleanup();
	foxManager.cleanup();
	sky.cleanup();
	terrainM.cleanup();
	cityManager.cleanup();

	// Close OpenGL window and terminate GLFW
	glfwTerminate();

	return 0;
}

static void key_callback(GLFWwindow *window, int key, int scancode, int action, int mode)
{
	const float moveSpeed = 20.0f;

	if (key == GLFW_KEY_C && action == GLFW_PRESS) {
		glm::vec3 lightPos = -lightDirection * 200.0f; // Position the light far away in the opposite direction
		eye_center = lightPos;
	}

	if (key == GLFW_KEY_UP && (action == GLFW_REPEAT || action == GLFW_PRESS)) {
		eye_center.y += moveSpeed;
		lookat.y += moveSpeed;
	}

	if (key == GLFW_KEY_DOWN && (action == GLFW_REPEAT || action == GLFW_PRESS)) {
		eye_center.y -= moveSpeed;
		lookat.y -= moveSpeed;
	}

	if (key == GLFW_KEY_W && (action == GLFW_REPEAT || action == GLFW_PRESS)) {
		glm::vec3 viewDirection = glm::normalize(lookat - eye_center);
		eye_center += viewDirection * moveSpeed;
		lookat += viewDirection * moveSpeed;
		updateLightSpaceMatrix();
	}

	if (key == GLFW_KEY_S && (action == GLFW_REPEAT || action == GLFW_PRESS)) {
		glm::vec3 viewDirection = glm::normalize(lookat - eye_center);
		eye_center -= viewDirection * moveSpeed;
		lookat -= viewDirection * moveSpeed;
		updateLightSpaceMatrix();
	}

	if (key == GLFW_KEY_A && (action == GLFW_REPEAT || action == GLFW_PRESS)) {
		glm::vec3 direction = glm::normalize(glm::cross(glm::normalize(lookat - eye_center), up));
		eye_center -= direction * moveSpeed;
		lookat -= direction * moveSpeed;
		updateLightSpaceMatrix();
	}

	if (key == GLFW_KEY_D && (action == GLFW_REPEAT || action == GLFW_PRESS)) {
		glm::vec3 direction = glm::normalize(glm::cross(glm::normalize(lookat - eye_center), up));
		eye_center += direction * moveSpeed;
		lookat += direction * moveSpeed;
		updateLightSpaceMatrix();
	}

	if (key == GLFW_KEY_SPACE && action == GLFW_PRESS) {
		playAnimation = !playAnimation;
	}

	if (key == GLFW_KEY_LEFT && action == GLFW_PRESS) {
		playbackSpeed += 1.0f;
		if (playbackSpeed > 10.0f)
			playbackSpeed = 10.0f;
	}

	if (key == GLFW_KEY_RIGHT && action == GLFW_PRESS) {
		playbackSpeed -= 1.0f;
		if (playbackSpeed < 1.0f)
			playbackSpeed = 1.0f;
	}

	if (key == GLFW_KEY_R && action == GLFW_PRESS) {
		eye_center = glm::vec3(100.0f, 100.0f, 100.0f);
		lookat =  glm::vec3(0,0,0);
		std::cout << "Camera Reset." << std::endl;
		updateLightSpaceMatrix();
	}

	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
		glfwSetWindowShouldClose(window, GL_TRUE);
	}
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
	if (button != GLFW_MOUSE_BUTTON_LEFT) return;

	if (action == GLFW_PRESS) {
		mousePressed = true;
		glfwGetCursorPos(window, &lastMouseX, &lastMouseY);
	} else if (action == GLFW_RELEASE) {
		mousePressed = false;
	}
}

// could use glm/gtc/quaternion to avoid gimbal lock, but did so and rotation was disorientating, maybe implemented wrong
void cursor_position_callback(GLFWwindow* window, double xpos, double ypos) {
	if (!mousePressed) return;

	double deltaX = xpos - lastMouseX;
	double deltaY = ypos - lastMouseY;
	lastMouseX = xpos;
	lastMouseY = ypos;

	glm::vec3 viewDirection = normalize(lookat - eye_center);

	glm::mat4 horizontalRotation = rotate(glm::mat4(1.0f), glm::radians((float)deltaX * sensitivity), up);
	viewDirection = glm::vec3(horizontalRotation * glm::vec4(viewDirection, 0.0f));

	glm::vec3 rightDirection = normalize(cross(viewDirection, up));
	glm::mat4 verticalRotation = rotate(glm::mat4(1.0f), glm::radians((float)deltaY * sensitivity), rightDirection);
	viewDirection = glm::vec3(verticalRotation * glm::vec4(viewDirection, 0.0f));

	lookat = eye_center + viewDirection;
}

void updateLightSpaceMatrix() {
	const float lightDistance = 200.0f; // Adjust as needed
	const float fixedLightY = 10.0f; // Example value, adjust based on your scene
	glm::vec3 lightPos = glm::vec3(eye_center.x, fixedLightY, eye_center.z) - lightDirection * lightDistance;
	glm::mat4 lightView = glm::lookAt(lightPos, glm::vec3(eye_center.x, 0.0f, eye_center.z), lightUp);
	lightSpaceMatrix = lightProjection * lightView;
}