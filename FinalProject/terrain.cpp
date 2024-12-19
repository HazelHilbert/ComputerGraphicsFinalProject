#include "terrain.h"
#include <render/shader.h>

#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
#include <utils.h>

#define STB_PERLIN_IMPLEMENTATION
#include "stb_perlin.h"

static int shadowMapWidth = 1024;
static int shadowMapHeight = 1024;

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

            //uvs.emplace_back(glm::vec2(x / static_cast<float>(width), z / static_cast<float>(depth)));
            float tilingFactor = 20.0f;
            uvs.emplace_back(glm::vec2(
                (x / static_cast<float>(width)) * tilingFactor,
                (z / static_cast<float>(depth)) * tilingFactor
            ));

        }
    }

    // Index buffer generation
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

    // Initialize normals
    normals.resize(vertices.size(), glm::vec3(0.0f, 0.0f, 0.0f));

    // Compute normals
    for (size_t i = 0; i < indices.size(); i += 3) {
        GLuint idx0 = indices[i];
        GLuint idx1 = indices[i + 1];
        GLuint idx2 = indices[i + 2];

        glm::vec3 v0 = vertices[idx0];
        glm::vec3 v1 = vertices[idx1];
        glm::vec3 v2 = vertices[idx2];

        glm::vec3 edge1 = v1 - v0;
        glm::vec3 edge2 = v2 - v0;

        glm::vec3 faceNormal = glm::normalize(glm::cross(edge1, edge2));

        normals[idx0] += faceNormal;
        normals[idx1] += faceNormal;
        normals[idx2] += faceNormal;
    }

    // Normalize the normals
    for (size_t i = 0; i < normals.size(); ++i) {
        normals[i] = glm::normalize(normals[i]);
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

    glGenBuffers(1, &normalBufferID);
    glBindBuffer(GL_ARRAY_BUFFER, normalBufferID);
    glBufferData(GL_ARRAY_BUFFER, normals.size() * sizeof(glm::vec3), normals.data(), GL_STATIC_DRAW);
/*
    // Create a fbo
    glGenFramebuffers(1, &fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);

    // Create a depth texture
    glGenTextures(1, &depthTexture);
    glBindTexture(GL_TEXTURE_2D, depthTexture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, shadowMapWidth, shadowMapHeight, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthTexture, 0);

    if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    {
        std::cout << "Failed to create framebuffer" << std::endl;
    }
*/
    programID = LoadShadersFromFile("../FinalProject/shader/terrain.vert", "../FinalProject/shader/terrain.frag");
    if (programID == 0) {
        std::cerr << "Failed to load shaders." << std::endl;
    }
/*
    depthProgramID = LoadShadersFromFile("../FinalProject/shader/depth.vert", "../FinalProject/shader/depth.frag");
    if (depthProgramID == 0)
    {
        std::cerr << "Failed to load depth shaders." << std::endl;
    }
*/
    std::string filePath = "../FinalProject/assets/textures/grass.jpg";
    textureID = LoadTextureTileBox(filePath.c_str());
    textureSamplerID = glGetUniformLocation(programID,"textureSampler");

    // Get a handle for our "MVP" uniform
    mvpMatrixID = glGetUniformLocation(programID, "MVP");

    lightIntensityID = glGetUniformLocation(programID, "lightIntensity");
    lightDirectionID = glGetUniformLocation(programID, "lightDirection");

  //  lightSpaceMatrixID = glGetUniformLocation(programID, "lightSpaceMatrixRender");
    //lightSpaceMatrixID = glGetUniformLocation(depthProgramID, "lightSpaceMatrix");
}

void Terrain::render(glm::mat4 vp, glm::mat4 lightSpaceMatrix, glm::vec3 lightDirection, glm::vec3 lightIntensity) {
    glUseProgram(programID);

    glBindVertexArray(vertexArrayID);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, textureID);
    glUniform1i(textureSamplerID, 0);

    glm::mat4 model = glm::mat4(1.0f);
    glm::mat4 mvp = vp * model;
    glUniformMatrix4fv(mvpMatrixID, 1, GL_FALSE, &mvp[0][0]);

    glUniform3fv(lightIntensityID, 1, &lightIntensity[0]);
    glUniform3fv(lightDirectionID, 1, &lightDirection[0]);

    glEnableVertexAttribArray(0); // Positions
    glBindBuffer(GL_ARRAY_BUFFER, vertexBufferID);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

    glEnableVertexAttribArray(2); // UVs
    glBindBuffer(GL_ARRAY_BUFFER, uvBufferID);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, 0);

    glEnableVertexAttribArray(3); // normals
    glBindBuffer(GL_ARRAY_BUFFER, normalBufferID);
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 0, 0);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBufferID);
/*
    // Depth Mapping
    glUseProgram(depthProgramID);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glUniformMatrix4fv(lightSpaceMatrixID, 1, GL_FALSE, &lightSpaceMatrix[0][0]);

    glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(indices.size()), GL_UNSIGNED_INT, 0);

    // render scene
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glUseProgram(programID);

    glUniformMatrix4fv(mvpMatrixID, 1, GL_FALSE, &mvp[0][0]);
    glUniformMatrix4fv(lightSpaceMatrixID, 1, GL_FALSE, &lightSpaceMatrix[0][0]);
    glBindTexture(GL_TEXTURE_2D, depthTexture);

    // Set light data
    glUniform3fv(lightDirectionID, 1, &lightDirection[0]);
    glUniform3fv(lightIntensityID, 1, &lightIntensity[0]);
*/
    glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(indices.size()), GL_UNSIGNED_INT, 0);

    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(2);
    glDisableVertexAttribArray(3);
}

void Terrain::cleanup() {
    glDeleteBuffers(1, &vertexBufferID);
    glDeleteBuffers(1, &indexBufferID);
    glDeleteVertexArrays(1, &vertexArrayID);
    glDeleteBuffers(1, &uvBufferID);
    glDeleteTextures(1, &textureID);
    glDeleteProgram(programID);
    glDeleteFramebuffers(1, &fbo);
    glDeleteTextures(1, &depthTexture);
    glDeleteProgram(depthProgramID);
}
