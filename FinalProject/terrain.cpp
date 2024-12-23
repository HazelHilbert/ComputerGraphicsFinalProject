#include "terrain.h"

#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
#include <utils.h>
#include <stb_image_write.h>
#include <future>
#include <condition_variable>

#define STB_PERLIN_IMPLEMENTATION
#include "stb_perlin.h"

static int shadowMapWidth = 2048;
static int shadowMapHeight = 1536;

bool saveDepth = false;

static void saveDepthTexture(GLuint fbo, std::string filename) {
    int width = shadowMapWidth;
    int height = shadowMapHeight;

    int channels = 3;

    std::vector<float> depth(width * height);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    //glReadBuffer(GL_DEPTH_COMPONENT);
    glReadPixels(0, 0, width, height, GL_DEPTH_COMPONENT, GL_FLOAT, depth.data());
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    std::vector<unsigned char> img(width * height * 3);
    for (int i = 0; i < width * height; ++i) img[3*i] = img[3*i+1] = img[3*i+2] = depth[i] * 255;

    stbi_write_png(filename.c_str(), width, height, channels, img.data(), width * channels);
}

std::future<TerrainData> Terrain::generateTerrainAsync(int width, int depth, float maxHeight, float posX, float posZ) {
    return std::async(std::launch::async, [=]() -> TerrainData {
        TerrainData data;

        // Begin generating vertices and normals
        float halfWidth = width / 2.0f;
        float halfDepth = depth / 2.0f;

        // Perlin noise parameters
        float scale = 0.02f; // Controls the frequency of the noise
        int octaves = 6;     // Number of layers of noise
        float persistence = 0.5f; // Amplitude multiplier for each octave
        float lacunarity = 2.0f;  // Frequency multiplier for each octave

        for (int z = 0; z <= depth; ++z) {
            for (int x = 0; x <= width; ++x) {
                float worldX = x - halfWidth + posX;
                float worldZ = z - halfDepth + posZ;

                float noiseValue = 0.0f;
                float frequency = scale;
                float amplitude = 1.0f;
                float maxAmplitude = 0.0f; // For normalization

                // Generate fractal noise by combining multiple octaves
                for (int i = 0; i < octaves; ++i) {
                    float sampleX = worldX * frequency;
                    float sampleZ = worldZ * frequency;

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

                data.vertices.emplace_back(glm::vec3(worldX, height, worldZ));
            }
        }

        // Initialize normals
        data.normals.resize(data.vertices.size(), glm::vec3(0.0f, 0.0f, 0.0f));

        // Compute normals
        for (size_t i = 0; i < indices.size(); i += 3) {
            GLuint idx0 = indices[i];
            GLuint idx1 = indices[i + 1];
            GLuint idx2 = indices[i + 2];

            glm::vec3 v0 = data.vertices[idx0];
            glm::vec3 v1 = data.vertices[idx1];
            glm::vec3 v2 = data.vertices[idx2];

            glm::vec3 edge1 = v1 - v0;
            glm::vec3 edge2 = v2 - v0;

            glm::vec3 faceNormal = glm::normalize(glm::cross(edge1, edge2));

            data.normals[idx0] += faceNormal;
            data.normals[idx1] += faceNormal;
            data.normals[idx2] += faceNormal;
        }

        // Normalize the normals
        for (auto & normal : data.normals) {
            normal = glm::normalize(normal);
        }

        return data;
    });
}

void Terrain::updateBuffers(const TerrainData& data) {
    std::lock_guard<std::mutex> lock(bufferMutex); // Protect buffer updates if accessed from multiple threads

    // Update vertex buffer
    glBindBuffer(GL_ARRAY_BUFFER, vertexBufferID);
    glBufferData(GL_ARRAY_BUFFER, data.vertices.size() * sizeof(glm::vec3), data.vertices.data(), GL_STATIC_DRAW);

    // Update normal buffer
    glBindBuffer(GL_ARRAY_BUFFER, normalBufferID);
    glBufferData(GL_ARRAY_BUFFER, data.normals.size() * sizeof(glm::vec3), data.normals.data(), GL_STATIC_DRAW);
}


void Terrain::setProgramIDs(GLuint inputProgramID, GLuint inputDepthProgramID) {
    if (programID == 0) programID = inputProgramID;
    if (depthProgramID == 0) depthProgramID = inputDepthProgramID;
}

void Terrain::initialize(int width, int depth, float maxHeight, float posX, float posZ) {
    // Index buffer generation
    for (int z = 0; z < depth; ++z) {
        int currentRow = z * (width + 1);
        int nextRow = (z + 1) * (width + 1);

        for (int x = 0; x < width; ++x) {
            GLuint topLeft = currentRow + x;
            GLuint topRight = topLeft + 1;
            GLuint bottomLeft = nextRow + x;
            GLuint bottomRight = bottomLeft + 1;

            indices.push_back(topLeft);
            indices.push_back(bottomLeft);
            indices.push_back(topRight);
            indices.push_back(topRight);
            indices.push_back(bottomLeft);
            indices.push_back(bottomRight);
        }
    }

    // UV initialization
    float tilingFactor = 20.0f;
    for (int z = 0; z <= depth; ++z) {
        for (int x = 0; x <= width; ++x) {
            uvs.emplace_back(glm::vec2(
                (x / static_cast<float>(width)) * tilingFactor,
                (z / static_cast<float>(depth)) * tilingFactor
            ));
        }
    }

    glGenVertexArrays(1, &vertexArrayID);
    glBindVertexArray(vertexArrayID);

    glGenBuffers(1, &vertexBufferID);

    glGenBuffers(1, &normalBufferID);

    std::future<TerrainData> fut = generateTerrainAsync(width, depth, maxHeight, posX, posZ);
    TerrainData data = fut.get();
    updateBuffers(data);

    glGenBuffers(1, &uvBufferID);
    glBindBuffer(GL_ARRAY_BUFFER, uvBufferID);
    glBufferData(GL_ARRAY_BUFFER, uvs.size() * sizeof(glm::vec2), uvs.data(), GL_STATIC_DRAW);

    glGenBuffers(1, &indexBufferID);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBufferID);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint), indices.data(), GL_STATIC_DRAW);

    // Create a fbo
    glGenFramebuffers(1, &fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);

    // Create a depth texture
    glGenTextures(1, &depthTexture);
    glBindTexture(GL_TEXTURE_2D, depthTexture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_NONE);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, shadowMapWidth, shadowMapHeight, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthTexture, 0);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);

    if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    {
        std::cout << "Failed to create framebuffer" << std::endl;
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    if (programID == 0 || depthProgramID == 0) {
        createTerrainProgramIDs(programID, depthProgramID);
    }

    std::string filePath = "../FinalProject/assets/textures/grass.jpg";
    textureID = LoadTextureTileBox(filePath.c_str());
    textureSamplerID = glGetUniformLocation(programID,"textureSampler");

    // Get a handle for our "MVP" uniform
    mvpMatrixID = glGetUniformLocation(programID, "MVP");
    modelMatrixIDRender = glGetUniformLocation(programID, "Model");
    modelMatrixIDDepth = glGetUniformLocation(depthProgramID, "Model");

    lightIntensityID = glGetUniformLocation(programID, "lightIntensity");
    lightDirectionID = glGetUniformLocation(programID, "lightDirection");

    lightSpaceMatrixIDRender = glGetUniformLocation(programID, "lightSpaceMatrixRender");
    lightSpaceMatrixIDDepth = glGetUniformLocation(depthProgramID, "lightSpaceMatrix");

    depthTextureSamplerID = glGetUniformLocation(programID, "depthTextureSampler");

    //std::cout << "Vertices: " << vertices.size() << std::endl;
}

void Terrain::render(glm::mat4 vp, glm::mat4 lightSpaceMatrix, glm::vec3 lightDirection, glm::vec3 lightIntensity) {
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

    glBindVertexArray(vertexArrayID);
    glEnableVertexAttribArray(0);

    glm::mat4 model = glm::mat4(1.0f);

    // -------------------
    // Depth Mapping
    // -------------------
    glUseProgram(depthProgramID);
    glViewport(0, 0, shadowMapWidth, shadowMapHeight);

    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glUniformMatrix4fv(lightSpaceMatrixIDDepth, 1, GL_FALSE, &lightSpaceMatrix[0][0]);
    glUniformMatrix4fv(modelMatrixIDDepth, 1, GL_FALSE, &model[0][0]);

    glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(indices.size()), GL_UNSIGNED_INT, 0);
    // -------------------
    // -------------------

    glUseProgram(programID);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0, 0, 2*1024, 2*768);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, textureID);
    glUniform1i(textureSamplerID, 0);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, depthTexture);
    glUniform1i(depthTextureSamplerID, 1);

    glm::mat4 mvp = vp * model;
    glUniformMatrix4fv(mvpMatrixID, 1, GL_FALSE, &mvp[0][0]);
    glUniformMatrix4fv(modelMatrixIDRender, 1, GL_FALSE, &model[0][0]);

    glUniform3fv(lightIntensityID, 1, &lightIntensity[0]);
    glUniform3fv(lightDirectionID, 1, &lightDirection[0]);
    glUniformMatrix4fv(lightSpaceMatrixIDRender, 1, GL_FALSE, &lightSpaceMatrix[0][0]);

    glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(indices.size()), GL_UNSIGNED_INT, 0);

    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(2);
    glDisableVertexAttribArray(3);

    if (saveDepth) {
        std::string filename = "depth_camera.png";
        saveDepthTexture(0, filename);
        std::cout << "Depth texture saved to " << filename << std::endl;
        filename = "depth_light.png";
        saveDepthTexture(fbo, filename);
        std::cout << "Depth texture saved to " << filename << std::endl;
        saveDepth = false;
    }
}

void Terrain::cleanup() {
    glDeleteBuffers(1, &vertexBufferID);
    glDeleteBuffers(1, &indexBufferID);
    glDeleteVertexArrays(1, &vertexArrayID);
    glDeleteBuffers(1, &uvBufferID);
    glDeleteTextures(1, &textureID);
    glDeleteFramebuffers(1, &fbo);
}
