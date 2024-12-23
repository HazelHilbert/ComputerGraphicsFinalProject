#ifndef TERRAIN_H
#define TERRAIN_H

#include <future>
#include <vector>
#include <glm/glm.hpp>
#include "glad/gl.h"

struct TerrainData {
    std::vector<glm::vec3> vertices;
    std::vector<glm::vec3> normals;
};

class Terrain {
public:
    std::future<TerrainData> generateTerrainAsync(int width, int depth, float maxHeight, float posX, float posZ);

    void updateBuffers(const TerrainData& data);

    void setProgramIDs(GLuint inputProgramID, GLuint inputDepthProgramID);

    void initialize(int width, int depth, float maxHeight, float posX = 0.0f, float posZ = 0.0f);

    void render(glm::mat4 vp, glm::mat4 lightSpaceMatrix, glm::vec3 lightDirection, glm::vec3 lightIntensity);

    void cleanup();

private:
    std::vector<glm::vec3> vertices;
    std::vector<glm::vec2> uvs;
    std::vector<GLuint> indices;
    std::vector<glm::vec3> normals;

    // OpenGL buffers
    GLuint vertexArrayID;
    GLuint vertexBufferID;
    GLuint indexBufferID;
    GLuint normalBufferID;
    GLuint lightIntensityID;
    GLuint lightDirectionID;
    GLuint uvBufferID;
    GLuint textureID;

    // Shader variable IDs
    GLuint mvpMatrixID;
    GLuint modelMatrixIDRender;
    GLuint modelMatrixIDDepth;
    GLuint textureSamplerID;
    GLuint programID = 0;

    // Variables for depth mapping
    GLuint fbo;
    GLuint depthTexture;
    GLuint depthProgramID = 0;
    GLuint lightSpaceMatrixIDRender;
    GLuint lightSpaceMatrixIDDepth;
    GLuint depthTextureSamplerID;

    std::mutex bufferMutex;
};

#endif
