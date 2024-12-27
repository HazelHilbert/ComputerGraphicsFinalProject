#ifndef CITY_H
#define CITY_H

#include <glm/glm.hpp>
#include <vector>
#include "glad/gl.h"
#include <tiny_gltf.h>

struct GltfRenderData {
    tinygltf::Model model;
    std::vector<GLuint> bufferIDs;
    std::vector<GLuint> textureIDs;
};

class City {
public:
    void setModelMatrix(glm::vec3 position, float size = 1, float rotation = 0, glm::vec3 rotationAxis = glm::vec3(0, 0, 1));
    void updatePosition(glm::vec3 position);
    void render(glm::mat4& vp, glm::vec3& lightPosition, glm::vec3& lightIntensity);
    void drawModel();
    void cleanup();

    glm::vec3 position = glm::vec3(0,0,0);
    glm::mat4 modelMatrix = glm::mat4(1.0f);
    glm::mat4 rotationScaleMatrix = glm::mat4(1.0f);

    GltfRenderData* renderData = nullptr;

    static GLuint programID;
};

#endif