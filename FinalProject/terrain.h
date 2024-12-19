#ifndef TERRAIN_H
#define TERRAIN_H

#include <vector>
#include <glm/glm.hpp>
#include "glad/gl.h"

class Terrain {
public:
    void initialize(int width, int depth, float maxHeight);

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
    //GLuint colorBufferID;
    GLuint uvBufferID;
    GLuint textureID;

    // Shader variable IDs
    GLuint mvpMatrixID;
    GLuint textureSamplerID;
    GLuint programID;

    // Variables for depth mapping
    GLuint fbo;
    GLuint depthTexture;
    GLuint depthProgramID;
    GLuint lightSpaceMatrixID;
};

#endif
