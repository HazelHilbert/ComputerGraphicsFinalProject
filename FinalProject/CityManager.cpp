#include "CityManager.h"

#include <cstdlib>
#include <ctime>
#include <glm/gtc/matrix_transform.hpp>

CityManager::CityManager() {
    std::srand(static_cast<unsigned int>(std::time(nullptr)));
}

CityManager::~CityManager() {
    cleanup();
}

void CityManager::initialize(int numberOfCities, float areaSize, float minSize, float maxSize) {
    m_cities.reserve(numberOfCities);

    for (int i = 0; i < numberOfCities; ++i) {
        City city;

        // Random position within the specified area
        float xpos = randomFloat(-areaSize, areaSize);
        float ypos = randomFloat(100.0f, 200.0f); // Assuming sky y-range
        float zpos = randomFloat(-areaSize, areaSize);
        city.position = glm::vec3(xpos, ypos, zpos);

        // Random size
        city.size = randomFloat(minSize, maxSize);

        city.rotation = 270;
        city.rotationAxis = glm::vec3(1.0f, 0.0f, 0.0f);

        city.cityModel.initialize(CITY_LOD0, xpos, ypos+1, zpos, city.size, city.rotation, city.rotationAxis);
        city.hullModel.initialize(HULL_LOD0, xpos, ypos, zpos, city.size, city.rotation, city.rotationAxis);

        m_cities.push_back(city);
    }
}

void CityManager::renderAll(const glm::mat4 &cameraMatrix, const glm::vec3 &lightPosition, const glm::vec3 &lightIntensity) {
    for (auto &city : m_cities) {
        city.cityModel.render(cameraMatrix, lightPosition, lightIntensity);
        city.hullModel.render(cameraMatrix, lightPosition, lightIntensity);
    }
}

void CityManager::cleanup() {
    for (auto &city : m_cities) {
        city.cityModel.cleanup();
        city.hullModel.cleanup();
    }
    m_cities.clear();
}

float CityManager::randomFloat(float min, float max) {
    float random = static_cast<float>(std::rand()) / static_cast<float>(RAND_MAX);
    return min + random * (max - min);
}
