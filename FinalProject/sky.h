#ifndef SKY_H
#define SKY_H

#include <glad/gl.h>
#include <glm/gtc/type_ptr.hpp>

struct Sky {
	glm::vec3 position;		// Position of the box
	glm::vec3 scale;		// Size of the box in each axis

	GLfloat vertex_buffer_data[72] = {	// Vertex definition for an inner facing box
		// Front face (Inner-facing)
		-1.0f,  1.0f, 1.0f,
		 1.0f,  1.0f, 1.0f,
		 1.0f, -1.0f, 1.0f,
		-1.0f, -1.0f, 1.0f,

		// Back face (Inner-facing)
		 1.0f,  1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f,
		-1.0f, -1.0f, -1.0f,
		 1.0f, -1.0f, -1.0f,

		// Left face (Inner-facing)
		-1.0f,  1.0f, -1.0f,
		-1.0f,  1.0f,  1.0f,
		-1.0f, -1.0f,  1.0f,
		-1.0f, -1.0f, -1.0f,

		// Right face (Inner-facing)
		 1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f, -1.0f,
		 1.0f, -1.0f, -1.0f,
		 1.0f, -1.0f,  1.0f,

		// Top face (Inner-facing)
		-1.0f,  1.0f, -1.0f,
		 1.0f,  1.0f, -1.0f,
		 1.0f,  1.0f,  1.0f,
		-1.0f,  1.0f,  1.0f,

		// Bottom face (Inner-facing)
		-1.0f, -1.0f,  1.0f,
		 1.0f, -1.0f,  1.0f,
		 1.0f, -1.0f, -1.0f,
		-1.0f, -1.0f, -1.0f,
	};


	GLfloat color_buffer_data[72] = {
		// Front, red
		1.0f, 0.0f, 0.0f,
		1.0f, 0.0f, 0.0f,
		1.0f, 0.0f, 0.0f,
		1.0f, 0.0f, 0.0f,

		// Back, yellow
		1.0f, 1.0f, 0.0f,
		1.0f, 1.0f, 0.0f,
		1.0f, 1.0f, 0.0f,
		1.0f, 1.0f, 0.0f,

		// Left, green
		0.0f, 1.0f, 0.0f,
		0.0f, 1.0f, 0.0f,
		0.0f, 1.0f, 0.0f,
		0.0f, 1.0f, 0.0f,

		// Right, cyan
		0.0f, 1.0f, 1.0f,
		0.0f, 1.0f, 1.0f,
		0.0f, 1.0f, 1.0f,
		0.0f, 1.0f, 1.0f,

		// Top, blue
		0.0f, 0.0f, 1.0f,
		0.0f, 0.0f, 1.0f,
		0.0f, 0.0f, 1.0f,
		0.0f, 0.0f, 1.0f,

		// Bottom, magenta
		1.0f, 0.0f, 1.0f,
		1.0f, 0.0f, 1.0f,
		1.0f, 0.0f, 1.0f,
		1.0f, 0.0f, 1.0f,
	};

	GLuint index_buffer_data[36] = {		// 12 triangle faces of a box
		0, 1, 2,
		0, 2, 3,

		4, 5, 6,
		4, 6, 7,

		8, 9, 10,
		8, 10, 11,

		12, 13, 14,
		12, 14, 15,

		16, 17, 18,
		16, 18, 19,

		20, 21, 22,
		20, 22, 23,
	};


	GLfloat uv_buffer_data[48] = {
		// Front pos z
		0.5f, 1.0f / 3.0f, // top right
		0.25f, 1.0f / 3.0f, // top left
		0.25f, 2.0f / 3.0f, // bottom left
		0.5f, 2.0f / 3.0f, // bottom right
		// Back neg z
		1.0f, 1.0f / 3.0f + 0.001, // top right
		0.75f, 1.0f / 3.0f + 0.001, // top left
		0.75f, 2.0f / 3.0f - 0.001, // bottom left
		1.0f, 2.0f / 3.0f - 0.001, // bottom right
		// Left neg x
		0.75f, 1.0f / 3.0f + 0.001, // top right
		0.5f, 1.0f / 3.0f + 0.001, // top left
		0.5f, 2.0f / 3.0f - 0.001, // bottom left
		0.75f, 2.0f / 3.0f - 0.001, // bottom right
		// Right pos x
		0.25f, 1.0f / 3.0f + 0.001, // top right
		0.0f, 1.0f / 3.0f + 0.001, // top left
		0.0f, 2.0f / 3.0f - 0.001, // bottom left
		0.25f, 2.0f / 3.0f - 0.001, // bottom right
		// Top pos y
		0.499f, 0.001f, // top right
		0.251f, 0.001f, // top left
		0.251f, 1.0f / 3.0f, // bottom left
		0.499f, 1.0f / 3.0f, // bottom right
		// Bottom neg y
		0.499f, 2.0f / 3.0f, // top right
		0.251f, 2.0f / 3.0f, // top left
		0.251f, 0.999f, // bottom left
		0.499f, 0.999f, // bottom right
	};

	// OpenGL buffers
	GLuint vertexArrayID;
	GLuint vertexBufferID;
	GLuint indexBufferID;
	GLuint colorBufferID;
	GLuint uvBufferID;
	GLuint textureID;

	// Shader variable IDs
	GLuint mvpMatrixID;
	GLuint textureSamplerID;
	GLuint programID;

	void initialize(glm::vec3 position, glm::vec3 scale);

	void render(glm::mat4 cameraMatrix);

	void cleanup();

};

#endif
