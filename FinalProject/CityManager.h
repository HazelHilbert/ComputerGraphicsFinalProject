#ifndef CITYMANAGER_H
#define CITYMANAGER_H

#include "model.h"
#include "city.h"
#include <vector>
#include <string>
#include <glm/glm.hpp>
#include <tiny_gltf.h>

struct SkyCity {
    City city;
    City hull;
};

class CityManager {
public:
    void initialize(int numberOfCities);

    void generateCities(int count);

    void render(glm::mat4& vp, glm::vec3 lightDirection, glm::vec3 lightIntensity, glm::vec3 cameraPos);

    void cleanup();

private:
    std::vector<SkyCity> cities;

    const float LOD0Radius = 800;
    const float LOD1Radius = 1200;
    const float LOD2Radius = 1500;
    const float viewRadius = LOD2Radius;

    const std::string CITY_LOD0 = "../FinalProject/assets/model/city/city_LOD0.gltf";
    const std::string CITY_LOD1 = "../FinalProject/assets/model/city/city_LOD1.gltf";
    const std::string CITY_LOD2 = "../FinalProject/assets/model/city/city_LOD2.gltf";

    const std::string HULL_LOD0 = "../FinalProject/assets/model/hull/hull_LOD0.gltf";
    const std::string HULL_LOD1 = "../FinalProject/assets/model/hull/hull_LOD1.gltf";
    const std::string HULL_LOD2 = "../FinalProject/assets/model/hull/hull_LOD2.gltf";

    GltfRenderData cityLOD0Data, cityLOD1Data, cityLOD2Data;
    GltfRenderData hullLOD0Data, hullLOD1Data, hullLOD2Data;

    tinygltf::Model cityLOD0, cityLOD1, cityLOD2;
    tinygltf::Model hullLOD0, hullLOD1, hullLOD2;

    bool loadModel(tinygltf::Model &model, const char *filename);

    void uploadModel(const tinygltf::Model &model, GltfRenderData &renderData);

    float randomFloat(float min, float max);
};

#endif
