#ifndef FOXMANAGER_H
#define FOXMANAGER_H

#include "animation.h"
#include <glm/gtc/matrix_transform.hpp>

class FoxManager {
public:
    MyBot fox;
    glm::mat4 modelMatrix = glm::mat4(1.0f);

    void initialize();
    void update(float deltaTime);
    void render(glm::mat4 cameraMatrix, glm::vec3 lightPosition, glm::vec3 lightIntensity);
    void cleanup();
};

#endif
