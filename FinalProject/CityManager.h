#ifndef CITYMANAGER_H
#define CITYMANAGER_H

#include "model.h"
#include "city.h"
#include <vector>
#include <string>
#include <glm/glm.hpp>

struct SkyCity {
    City city;
    City hull;
};

class CityManager {
public:
    bool loadModel(tinygltf::Model &model, const char *filename);

    void initialize(int numberOfCities);

    void generateCities(int count);

    void render(glm::mat4& vp, glm::vec3 lightPosition, glm::vec3 lightIntensity, glm::vec3 cameraPos);

    void cleanup();

private:
    std::vector<SkyCity> cities;

    const float LOD0Radius = 100;
    const float LOD1Radius = 200;
    const float LOD2Radius = 500;
    const float viewRadius = LOD2Radius;

    const std::string CITY_LOD0 = "../FinalProject/assets/model/city/city_LOD0.gltf";
    const std::string CITY_LOD1 = "../FinalProject/assets/model/city/city_LOD1.gltf";
    const std::string CITY_LOD2 = "../FinalProject/assets/model/city/city_LOD2.gltf";

    const std::string HULL_LOD0 = "../FinalProject/assets/model/hull/hull_LOD0.gltf";
    const std::string HULL_LOD1 = "../FinalProject/assets/model/hull/hull_LOD1.gltf";
    const std::string HULL_LOD2 = "../FinalProject/assets/model/hull/hull_LOD2.gltf";

    tinygltf::Model cityLOD0, cityLOD1, cityLOD2, hullLOD0, hullLOD1, hullLOD2;

    int distanceToCamera(City city, glm::vec3 cameraPos);
    float randomFloat(float min, float max);
};

#endif
