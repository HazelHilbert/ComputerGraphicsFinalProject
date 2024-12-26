#ifndef CITY_H
#define CITY_H

#include <glm/glm.hpp>
#include <vector>
#include "model.h"
#include "glad/gl.h"

class City {
public:
    void setModelMatrix(glm::vec3 position, float size = 1, float rotation = 0, glm::vec3 rotationAxis = glm::vec3(0, 0, 1));

    void render(glm::mat4& vp, glm::vec3& lightPosition, glm::vec3& lightIntensity);

    void drawModel(glm::mat4& mvp);

    void cleanup();

    glm::vec3 position = glm::vec3(0,0,0);
    glm::mat4 modelMatrix = glm::mat4(1.0f);

    static tinygltf::Model model;

    static GLuint programID;
    static tinygltf::Model cityLOD0, cityLOD1, cityLOD2, hullLOD0, hullLOD1, hullLOD2;
    static std::vector<GLuint> bufferIDs;
    static std::vector<GLuint> textureIDs;
};

#endif
