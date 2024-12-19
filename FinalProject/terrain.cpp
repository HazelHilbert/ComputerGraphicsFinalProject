#include "terrain.h"
#include <render/shader.h>

#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
#include <utils.h>

#define STB_PERLIN_IMPLEMENTATION
#include "stb_perlin.h"

void Terrain::initialize(int width, int depth, float maxHeight) {
    float halfWidth = width / 2.0f;
    float halfDepth = depth / 2.0f;

    // Perlin noise parameters
    float scale = 0.02f; // Controls the frequency of the noise
    int octaves = 6;     // Number of layers of noise
    float persistence = 0.5f; // Amplitude multiplier for each octave
    float lacunarity = 2.0f;  // Frequency multiplier for each octave

    for (int z = 0; z <= depth; ++z) {
        for (int x = 0; x <= width; ++x) {
            float worldX = x - halfWidth;
            float worldZ = z - halfDepth;

            float noiseValue = 0.0f;
            float frequency = scale;
            float amplitude = 1.0f;
            float maxAmplitude = 0.0f; // For normalization

            // Generate fractal noise by combining multiple octaves
            for (int i = 0; i < octaves; ++i) {
                float sampleX = worldX * frequency;
                float sampleZ = worldZ * frequency;

                // stb_perlin_noise3 requires float inputs; the third parameter is usually set to 0 for 2D noise
                float perlin = stb_perlin_noise3(sampleX, sampleZ, 0.0f, 0, 0, 0);
                noiseValue += perlin * amplitude;

                maxAmplitude += amplitude;
                amplitude *= persistence;
                frequency *= lacunarity;
            }

            // Normalize the noise value to range [-1, 1]
            noiseValue /= maxAmplitude;

            // Scale the noise value to the desired height range
            float height = noiseValue * maxHeight;

            vertices.emplace_back(glm::vec3(worldX, height, worldZ));
            uvs.emplace_back(glm::vec2(x / static_cast<float>(width), z / static_cast<float>(depth)));
        }
    }

    // Index buffer generation remains the same
    for (int z = 0; z < depth; ++z) {
        for (int x = 0; x < width; ++x) {
            GLuint topLeft = z * (width + 1) + x;
            GLuint topRight = topLeft + 1;
            GLuint bottomLeft = (z + 1) * (width + 1) + x;
            GLuint bottomRight = bottomLeft + 1;

            indices.push_back(topLeft);
            indices.push_back(bottomLeft);
            indices.push_back(topRight);
            indices.push_back(topRight);
            indices.push_back(bottomLeft);
            indices.push_back(bottomRight);
        }
    }

    glGenVertexArrays(1, &vertexArrayID);
    glBindVertexArray(vertexArrayID);

    glGenBuffers(1, &vertexBufferID);
    glBindBuffer(GL_ARRAY_BUFFER, vertexBufferID);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec3), vertices.data(), GL_STATIC_DRAW);

    glGenBuffers(1, &uvBufferID);
    glBindBuffer(GL_ARRAY_BUFFER, uvBufferID);
    glBufferData(GL_ARRAY_BUFFER, uvs.size() * sizeof(glm::vec2), uvs.data(), GL_STATIC_DRAW);

    glGenBuffers(1, &indexBufferID);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBufferID);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint), indices.data(), GL_STATIC_DRAW);

    programID = LoadShadersFromFile("../FinalProject/shader/terrain.vert", "../FinalProject/shader/terrain.frag");
    if (programID == 0) {
        std::cerr << "Failed to load shaders." << std::endl;
    }

    std::string filePath = "../FinalProject/assets/textures/grass.jpg";
    textureID = LoadTextureTileBox(filePath.c_str());
    textureSamplerID = glGetUniformLocation(programID,"textureSampler");

    // Get a handle for our "MVP" uniform
    mvpMatrixID = glGetUniformLocation(programID, "MVP");
}

void Terrain::render(glm::mat4 vp) {
    glUseProgram(programID);

    glBindVertexArray(vertexArrayID);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, textureID);
    glUniform1i(textureSamplerID, 0);

    glm::mat4 model = glm::mat4(1.0f);
    glm::mat4 mvp = vp * model;
    glUniformMatrix4fv(mvpMatrixID, 1, GL_FALSE, &mvp[0][0]);

    glEnableVertexAttribArray(0); // Positions
    glBindBuffer(GL_ARRAY_BUFFER, vertexBufferID);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

    glEnableVertexAttribArray(2); // UVs
    glBindBuffer(GL_ARRAY_BUFFER, uvBufferID);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, 0);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBufferID);
    glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(indices.size()), GL_UNSIGNED_INT, 0);

    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);
}

void Terrain::cleanup() {
    glDeleteBuffers(1, &vertexBufferID);
    glDeleteBuffers(1, &indexBufferID);
    glDeleteVertexArrays(1, &vertexArrayID);
    glDeleteBuffers(1, &uvBufferID);
    glDeleteTextures(1, &textureID);
    glDeleteProgram(programID);
}
