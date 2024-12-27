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

void CityManager::uploadModel(const tinygltf::Model &model, GltfRenderData &renderData) {
    renderData.model = model;

    // Upload buffers to GPU
    for (const auto& bufferView : renderData.model.bufferViews) {
        GLuint bufferID;
        glGenBuffers(1, &bufferID);
        glBindBuffer(bufferView.target, bufferID);

        glBufferData(bufferView.target,
                     bufferView.byteLength,
                     &renderData.model.buffers[bufferView.buffer].data[bufferView.byteOffset],
                     GL_STATIC_DRAW);

        renderData.bufferIDs.push_back(bufferID);
    }

    // Upload textures to GPU
    for (const auto& texture : renderData.model.textures) {
        const auto& image = renderData.model.images[texture.source];

        GLuint textureID;
        glGenTextures(1, &textureID);
        glBindTexture(GL_TEXTURE_2D, textureID);

        glTexImage2D(
            GL_TEXTURE_2D, 0, GL_RGBA,
            image.width, image.height, 0,
            GL_RGBA, GL_UNSIGNED_BYTE,
            image.image.data()
        );

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

        glGenerateMipmap(GL_TEXTURE_2D);

        renderData.textureIDs.push_back(textureID);
    }
}

void CityManager::initialize(int numberOfCities) {
    City::programID = LoadShadersFromFile("../FinalProject/shader/model.vert", "../FinalProject/shader/model.frag");
    if (City::programID == 0) {
        std::cerr << "Failed to load shaders." << std::endl;
    }

    loadModel(cityLOD0, CITY_LOD0.c_str());
    loadModel(cityLOD1, CITY_LOD1.c_str());
    loadModel(cityLOD2, CITY_LOD2.c_str());

    loadModel(hullLOD0, HULL_LOD0.c_str());
    loadModel(hullLOD1, HULL_LOD1.c_str());
    loadModel(hullLOD2, HULL_LOD2.c_str());

    uploadModel(cityLOD0, cityLOD0Data);
    uploadModel(cityLOD1, cityLOD1Data);
    uploadModel(cityLOD2, cityLOD2Data);

    uploadModel(hullLOD0, hullLOD0Data);
    uploadModel(hullLOD1, hullLOD1Data);
    uploadModel(hullLOD2, hullLOD2Data);

    generateCities(numberOfCities);
}

void CityManager::generateCities(int numberOfCities) {
    for (int i = 0; i < numberOfCities; i++) {

        float theta = 2.0f * M_PI * randomFloat(0,1);
        float r = viewRadius * sqrt(randomFloat(0,1));
        glm::vec3 position(r * cos(theta), randomFloat(150, 200), r * sin(theta));
        float size = randomFloat(5, 10);
        float rotation = randomFloat(0, 360);

        City city;
        city.setModelMatrix(position, size, rotation, glm::vec3(0,0,1));

        City hull;
        hull.setModelMatrix(position - glm::vec3(0,size/10,0), size, rotation, glm::vec3(0,0,1));

        float offset = randomFloat(-city.offsetLength,city.offsetLength);
        bool up = randomFloat(0,1) < 0.5;
        city.yOffset = offset;
        city.up = up;
        hull.yOffset = offset;
        hull.up = up;

        city.renderData = &cityLOD0Data;
        hull.renderData = &hullLOD0Data;

        SkyCity skyCity;
        skyCity.city = city;
        skyCity.hull = hull;

        cities.push_back(skyCity);
    }
}

void CityManager::render(glm::mat4& vp, glm::vec3 lightDirection, glm::vec3 lightIntensity, glm::vec3 cameraPos) {
    for (auto& skyCity : cities) {
        glUseProgram(City::programID);
        glUniform3fv(glGetUniformLocation(City::programID, "lightDir"), 1, &lightDirection[0]);
        glUniform3fv(glGetUniformLocation(City::programID, "lightIntensity"), 1, &lightIntensity[0]);

        glm::vec3 oldPosition = skyCity.city.position;
        float hull_offset = oldPosition.y - skyCity.hull.position.y;
        glm::vec3 cameraToOldPos = oldPosition - cameraPos;
        float distanceXY = glm::length(glm::vec3(cameraToOldPos.x, 0,cameraToOldPos.z));

        // out of frame
        if (distanceXY > viewRadius) {
            glm::vec3 normalizedXY = glm::normalize(glm::vec3(cameraToOldPos.x, 0, cameraToOldPos.z));
            glm::vec3 newPosition = cameraPos - normalizedXY * viewRadius;
            newPosition.y = oldPosition.y;

            skyCity.city.updatePosition(newPosition);
            skyCity.hull.updatePosition(newPosition - glm::vec3(0,hull_offset,0));
            continue;
        }

        float distance = glm::distance(oldPosition, cameraPos);

        glm::vec3 LODCameraPos = glm::vec3(0);

        // LOD2
        if (distance > LOD1Radius) {
            skyCity.city.renderData = &cityLOD2Data;
            skyCity.hull.renderData = &hullLOD2Data;
        }

        // LOD1
        else if (distance > LOD0Radius) {
            skyCity.city.renderData = &cityLOD1Data;
            skyCity.hull.renderData = &hullLOD1Data;
        }

        // LOD0
        else {
            skyCity.city.renderData = &cityLOD0Data;
            skyCity.hull.renderData = &hullLOD0Data;
            LODCameraPos = cameraPos;
        }


        skyCity.city.render(vp, lightDirection, lightIntensity, LODCameraPos);
        skyCity.hull.render(vp, lightDirection, lightIntensity, LODCameraPos);
    }
}

void CityManager::cleanup() {
    glDeleteProgram(City::programID);
}

float CityManager::randomFloat(float min, float max) {
    float random = static_cast<float>(std::rand()) / static_cast<float>(RAND_MAX);
    return min + random * (max - min);
}
