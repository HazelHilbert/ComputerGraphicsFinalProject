#include "FoxManager.h"

void FoxManager::initialize() {
    fox.initialize();
}

void FoxManager::update(float deltaTime) {
    fox.update(deltaTime);
    modelMatrix = glm::translate(modelMatrix,glm::vec3(0,0,0.4));
}

void FoxManager::render(glm::mat4 cameraMatrix, glm::vec3 lightPosition, glm::vec3 lightIntensity) {
    fox.render(modelMatrix, cameraMatrix, lightPosition, lightIntensity);
}

void FoxManager::cleanup() {
    fox.cleanup();
}