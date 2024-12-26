#include "CityManager.h"

#include <glm/gtc/matrix_transform.hpp>
#include <random>
#include <iostream>
#include <render/shader.h>

bool CityManager::loadModel(tinygltf::Model &model, const char *filename) {
    tinygltf::TinyGLTF loader;
    std::string err;
    std::string warn;

    bool res = loader.LoadASCIIFromFile(&model, &err, &warn, filename);
    if (!warn.empty()) {
        std::cout << "WARN: " << warn << std::endl;
    }

    if (!err.empty()) {
        std::cout << "ERR: " << err << std::endl;
    }

    if (!res)
        std::cout << "Failed to load glTF: " << filename << std::endl;
    else
        std::cout << "Loaded glTF: " << filename << std::endl;

    return res;
}

void CityManager::initialize(int numberOfCities) {
    City::programID = LoadShadersFromFile("../FinalProject/shader/model.vert", "../FinalProject/shader/model.frag");
    if (City::programID == 0) {
        std::cerr << "Failed to load shaders." << std::endl;
    }

    loadModel(City::cityLOD0, CITY_LOD0.c_str());
    loadModel(City::cityLOD1, CITY_LOD1.c_str());
    loadModel(City::cityLOD2, CITY_LOD2.c_str());
    loadModel(City::hullLOD0, HULL_LOD0.c_str());
    loadModel(City::hullLOD1, HULL_LOD1.c_str());
    loadModel(City::hullLOD2, HULL_LOD2.c_str());


    City::model = City::hullLOD0;

    for (const auto& bufferView : City::model.bufferViews) {
        GLuint bufferID;
        glGenBuffers(1, &bufferID);
        glBindBuffer(bufferView.target, bufferID);
        glBufferData(bufferView.target,
                     bufferView.byteLength,
                     &City::model.buffers[bufferView.buffer].data[bufferView.byteOffset],
                     GL_STATIC_DRAW);
        City::bufferIDs.push_back(bufferID);
    }

    for (const auto& texture : City::model.textures) {
        const auto& image = City::model.images[texture.source];

        GLuint textureID;
        glGenTextures(1, &textureID);
        glBindTexture(GL_TEXTURE_2D, textureID);

        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image.width, image.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image.image.data());

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

        glGenerateMipmap(GL_TEXTURE_2D);

        City::textureIDs.push_back(textureID);
    }

    generateCities(numberOfCities);
}

void CityManager::generateCities(int numberOfCities) {
    for (int i = 0; i < numberOfCities; i++) {

        glm::vec3 position = glm::vec3(randomFloat(-viewRadius,viewRadius),randomFloat(100,200),randomFloat(-viewRadius,viewRadius));

        float size = randomFloat(5,10);
        float rotation = randomFloat(0,365);

        City city;
        city.setModelMatrix(position,size,rotation,glm::vec3(0,0,1));

        City hull;
        hull.setModelMatrix(position+glm::vec3(0,10,0),size,rotation,glm::vec3(0,0,1));

        SkyCity skyCity;
        skyCity.city = city;
        skyCity.hull = hull;

        cities.push_back(skyCity);
    }
}

void CityManager::render(glm::mat4& vp, glm::vec3 lightPosition, glm::vec3 lightIntensity, glm::vec3 cameraPos) {
    for (auto& skyCity : cities) {
        glUseProgram(City::programID);
        glUniform3fv(glGetUniformLocation(City::programID, "lightPos"), 1, &lightPosition[0]);
        glUniform3fv(glGetUniformLocation(City::programID, "lightIntensity"), 1, &lightIntensity[0]);

        tinygltf::Model cityModel, hullModel;
        int distance = distanceToCamera(skyCity.city, cameraPos);
        if (distance == -1) {
            continue; //TODO: change pos

            distance = distanceToCamera(skyCity.city, cameraPos);
        }


        if (distance == 0) {
            cityModel = cityLOD0;
            hullModel = hullLOD0;
        } else if (distance == 1) {
            cityModel = cityLOD1;
            hullModel = hullLOD1;
        } else if (distance == 2) {
            cityModel = cityLOD2;
            hullModel = hullLOD2;
        }

        //pass city and hull model?
        skyCity.city.render(vp, lightPosition, lightIntensity);
        skyCity.hull.render(vp, lightPosition, lightIntensity);
    }
}

void CityManager::cleanup() {
    glDeleteProgram(City::programID);
}

int CityManager::distanceToCamera(City city, glm::vec3 cameraPos) {
    glm::vec3 removeY = glm::vec3(1,0,1);
    float distance = glm::distance(city.position*removeY, cameraPos*removeY);

    if (distance > LOD2Radius) return -1;
    if (distance > LOD1Radius) return 2;
    if (distance > LOD0Radius) return 1;
    return 0;
}


float CityManager::randomFloat(float min, float max) {
    float random = static_cast<float>(std::rand()) / static_cast<float>(RAND_MAX);
    return min + random * (max - min);
}