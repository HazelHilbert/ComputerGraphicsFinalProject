#include "city.h"
#include <glm/gtc/matrix_transform.hpp>
#include <render/shader.h>
#include <iostream>

GLuint City::programID = 0;

void City::setModelMatrix(glm::vec3 position, float size, float rotation, glm::vec3 rotationAxis) {
    rotationScaleMatrix = glm::rotate(glm::mat4(1.0f), glm::radians(270.f), glm::vec3(1.f,0.f,0.f)); // default rotation
    rotationScaleMatrix = glm::rotate(rotationScaleMatrix, glm::radians(rotation), rotationAxis);
    rotationScaleMatrix = glm::scale(rotationScaleMatrix, glm::vec3(size));

    updatePosition(position);
}

void City::updatePosition(glm::vec3 position) {
    this->position = position;
    modelMatrix = glm::translate(glm::mat4(1.0f), position);
    modelMatrix = modelMatrix * rotationScaleMatrix;
}

void City::render(glm::mat4& vp, glm::vec3& lightDirection, glm::vec3& lightIntensity, glm::vec3& cameraPos) {
    if (!renderData) return;

    glUseProgram(programID);
    //move();
    glm::mat4 mvp = vp * modelMatrix;
    glm::mat3 normalMatrix = glm::mat3(glm::transpose(glm::inverse(modelMatrix)));


    glUniformMatrix4fv(glGetUniformLocation(programID, "MVPMatrix"), 1, GL_FALSE, &mvp[0][0]);
    glUniformMatrix4fv(glGetUniformLocation(programID, "modelMatrix"), 1, GL_FALSE, &modelMatrix[0][0]);
    glUniformMatrix3fv(glGetUniformLocation(programID, "normalMatrix"), 1, GL_FALSE, &normalMatrix[0][0]);

    glUniform3fv(glGetUniformLocation(programID, "lightDir"), 1, &lightDirection[0]);
    glUniform3fv(glGetUniformLocation(programID, "lightIntensity"), 1, &lightIntensity[0]);
    glUniform3fv(glGetUniformLocation(programID, "cameraPos"), 1, &cameraPos[0]);

    drawModel();
}

void City::drawModel() {
    const tinygltf::Model& modelRef = renderData->model;

    for (const auto& node : modelRef.nodes) {
        if (node.mesh < 0) continue;
        const auto& mesh = modelRef.meshes[node.mesh];

        for (const auto& primitive : mesh.primitives) {
            const auto& material    = modelRef.materials[primitive.material];
            const auto& textureInfo = material.pbrMetallicRoughness.baseColorTexture;

            glActiveTexture(GL_TEXTURE0);
            if (textureInfo.index >= 0 && textureInfo.index < (int)renderData->textureIDs.size()) {
                glBindTexture(GL_TEXTURE_2D, renderData->textureIDs[textureInfo.index]);
            }
            glUniform1i(glGetUniformLocation(programID, "modelTexture"), 0);

            // POSITION
            if (primitive.attributes.find("POSITION") != primitive.attributes.end()) {
                int accessorIdx = primitive.attributes.at("POSITION");
                const auto& accessor   = modelRef.accessors[accessorIdx];
                const auto& bufferView = modelRef.bufferViews[accessor.bufferView];

                glBindBuffer(GL_ARRAY_BUFFER, renderData->bufferIDs[accessor.bufferView]);
                glVertexAttribPointer(
                    0, 3, GL_FLOAT, GL_FALSE,
                    bufferView.byteStride,
                    reinterpret_cast<void*>(accessor.byteOffset)
                );
                glEnableVertexAttribArray(0);
            }

            // TEXCOORD_0
            if (primitive.attributes.find("TEXCOORD_0") != primitive.attributes.end()) {
                int accessorIdx = primitive.attributes.at("TEXCOORD_0");
                const auto& accessor   = modelRef.accessors[accessorIdx];
                const auto& bufferView = modelRef.bufferViews[accessor.bufferView];

                glBindBuffer(GL_ARRAY_BUFFER, renderData->bufferIDs[accessor.bufferView]);
                glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, bufferView.byteStride,
                    reinterpret_cast<void*>(accessor.byteOffset));
                glEnableVertexAttribArray(2);
            }

            // NORMAL
            if (primitive.attributes.find("NORMAL") != primitive.attributes.end()) {
                int accessorIdx = primitive.attributes.at("NORMAL");
                const auto& accessor   = modelRef.accessors[accessorIdx];
                const auto& bufferView = modelRef.bufferViews[accessor.bufferView];

                glBindBuffer(GL_ARRAY_BUFFER, renderData->bufferIDs[accessor.bufferView]);
                glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, bufferView.byteStride,
                    reinterpret_cast<void*>(accessor.byteOffset));
                glEnableVertexAttribArray(1);
            }

            // INDICES
            if (primitive.indices >= 0) {
                const auto& accessor   = modelRef.accessors[primitive.indices];
                const auto& bufferView = modelRef.bufferViews[accessor.bufferView];

                glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, renderData->bufferIDs[accessor.bufferView]);
                glDrawElements(GL_TRIANGLES, accessor.count, accessor.componentType,
                    reinterpret_cast<void*>(accessor.byteOffset));
            }
        }

        glDisableVertexAttribArray(0);
        glDisableVertexAttribArray(1);
        glDisableVertexAttribArray(2);
    }
}

void City::cleanup() {
}

void City::move() {
    if (up) {
        if (yOffset <= offsetLength) yOffset += offsetSpeed;
        else up = false;
    } else {
        if (yOffset >= -offsetLength) yOffset -= offsetSpeed;
        else up = true;
    }
    updatePosition(position+glm::vec3(0,yOffset,0));
}
