#include "city.h"
#include <glm/gtc/matrix_transform.hpp>
#include <render/shader.h>
#include <iostream>

GLuint City::programID = 0;
tinygltf::Model City::model;
tinygltf::Model City::cityLOD0, City::cityLOD1, City::cityLOD2, City::hullLOD0, City::hullLOD1, City::hullLOD2;
std::vector<GLuint> City::bufferIDs;
std::vector<GLuint> City::textureIDs;

void City::setModelMatrix(glm::vec3 position, float size, float rotation, glm::vec3 rotationAxis) {
    this->position = position;

    modelMatrix = glm::translate(modelMatrix, position);
    modelMatrix = glm::rotate(modelMatrix, glm::radians(270.f) ,glm::vec3(1.f,0.f,0.f)); // default rotation
    modelMatrix = glm::rotate(modelMatrix, glm::radians(rotation), rotationAxis);
    modelMatrix = glm::scale(modelMatrix, glm::vec3(size));
}

void City::render(glm::mat4& vp, glm::vec3& lightPosition, glm::vec3& lightIntensity) {
    glUseProgram(programID);

    glm::mat4 mvp = vp * modelMatrix;

    glUniformMatrix4fv(glGetUniformLocation(programID, "MVPMatrix"), 1, GL_FALSE, &mvp[0][0]);
    glUniform3fv(glGetUniformLocation(programID, "lightPos"), 1, &lightPosition[0]);
    glUniform3fv(glGetUniformLocation(programID, "lightIntensity"), 1, &lightIntensity[0]);

    drawModel(mvp);
}

void City::drawModel(glm::mat4& mvp) {
    for (const auto& node : model.nodes) {
        if (node.mesh < 0) continue;

        const auto& mesh = model.meshes[node.mesh];

        for (const auto& primitive : mesh.primitives) {

            const auto& material = model.materials[primitive.material];
            const auto& textureInfo = material.pbrMetallicRoughness.baseColorTexture;
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, textureIDs[textureInfo.index]);
            glUniform1i(glGetUniformLocation(programID, "modelTexture"), 0);


            if (primitive.attributes.find("POSITION") != primitive.attributes.end()) {
                const int accessorIdx = primitive.attributes.at("POSITION");
                const auto& accessor = model.accessors[accessorIdx];
                const auto& bufferView = model.bufferViews[accessor.bufferView];

                glBindBuffer(GL_ARRAY_BUFFER, bufferIDs[accessor.bufferView]);
                glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, bufferView.byteStride,
                                      reinterpret_cast<void*>(accessor.byteOffset));
                glEnableVertexAttribArray(0);
            }

            if (primitive.attributes.find("TEXCOORD_0") != primitive.attributes.end()) {
                const int accessorIdx = primitive.attributes.at("TEXCOORD_0");
                const auto& accessor = model.accessors[accessorIdx];
                const auto& bufferView = model.bufferViews[accessor.bufferView];

                glBindBuffer(GL_ARRAY_BUFFER, bufferIDs[accessor.bufferView]);
                glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, bufferView.byteStride, reinterpret_cast<void*>(accessor.byteOffset));
                glEnableVertexAttribArray(2);
            }

            if (primitive.attributes.find("NORMAL") != primitive.attributes.end()) {
                const int accessorIdx = primitive.attributes.at("NORMAL");
                const auto& accessor = model.accessors[accessorIdx];
                const auto& bufferView = model.bufferViews[accessor.bufferView];

                glBindBuffer(GL_ARRAY_BUFFER, bufferIDs[accessor.bufferView]);
                glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, bufferView.byteStride, reinterpret_cast<void*>(accessor.byteOffset));
                glEnableVertexAttribArray(1);
            }

            if (primitive.indices >= 0) {
                const auto& accessor = model.accessors[primitive.indices];
                const auto& bufferView = model.bufferViews[accessor.bufferView];

                glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, bufferIDs[accessor.bufferView]);
                glDrawElements(GL_TRIANGLES, accessor.count, accessor.componentType,reinterpret_cast<void*>(accessor.byteOffset));
            }
        }

        glDisableVertexAttribArray(0);
        glDisableVertexAttribArray(1);
        glDisableVertexAttribArray(2);
    }
}

void City::cleanup() {
}