#ifndef CITYMANAGER_H
#define CITYMANAGER_H

#include "model.h"
#include <vector>
#include <string>
#include <glm/glm.hpp>

struct City {
    Model cityModel;
    Model hullModel;
    glm::vec3 position;
    float size;
    float rotation;
    glm::vec3 rotationAxis;
};

class CityManager {
public:
    CityManager();
    ~CityManager();

    // Initialize the city manager with a specified number of cities
    void initialize(int numberOfCities, float areaSize, float minSize, float maxSize);

    // Render all cities
    void renderAll(const glm::mat4 &cameraMatrix,
                  const glm::vec3 &lightPosition,
                  const glm::vec3 &lightIntensity);

    // Cleanup all cities
    void cleanup();

private:
    std::vector<City> m_cities;

    // Random number generation utilities
    float randomFloat(float min, float max);

    const std::string CITY_LOD0 = "../FinalProject/assets/model/city/city_LOD0.gltf";
    const std::string CITY_LOD1 = "../FinalProject/assets/model/city/city_LOD1.gltf";
    const std::string CITY_LOD2 = "../FinalProject/assets/model/city/city_LOD2.gltf";

    const std::string HULL_LOD0 = "../FinalProject/assets/model/hull/hull_LOD0.gltf";
    const std::string HULL_LOD1 = "../FinalProject/assets/model/hull/hull_LOD1.gltf";
    const std::string HULL_LOD2 = "../FinalProject/assets/model/hull/hull_LOD2.gltf";
};

#endif
