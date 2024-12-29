#include "FoxManager.h"

void FoxManager::initialize() {
    fox.initialize();
    foxPosition = glm::vec3(0.0f, 0, 0.0f);
    modelMatrix = glm::scale(modelMatrix,glm::vec3(0.05));
    modelMatrix = glm::translate(modelMatrix,foxPosition);
}

void FoxManager::update(float deltaTime) {
    fox.update(deltaTime);
    foxPosition.z += 1;
    modelMatrix = glm::scale(glm::mat4(1.0f), glm::vec3(0.05f));
    modelMatrix = glm::translate(modelMatrix, foxPosition);
}

void FoxManager::render(glm::mat4 cameraMatrix, glm::vec3 lightPosition, glm::vec3 lightIntensity) {
    fox.render(modelMatrix, cameraMatrix, lightPosition, lightIntensity);
}

void FoxManager::cleanup() {
    fox.cleanup();
}