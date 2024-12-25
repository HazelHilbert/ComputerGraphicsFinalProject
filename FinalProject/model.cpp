#include "model.h"

#include <iostream>
#include <cassert>
#include <fstream>
#include <render/shader.h>
#include <tiny_gltf.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#define BUFFER_OFFSET(i) ((char *)NULL + (i))

bool Model::loadModel(tinygltf::Model &model, const char *filename) {
    tinygltf::TinyGLTF loader;
    std::string err;
    std::string warn;

    bool res = loader.LoadASCIIFromFile(&model, &err, &warn, filename);
    if (!warn.empty()) {
        std::cerr << "WARN: " << warn << std::endl;
    }

    if (!err.empty()) {
        std::cerr << "ERR: " << err << std::endl;
    }

    if (!res) {
        std::cerr << "Failed to load glTF: " << filename << std::endl;
    } else {
        std::cout << "Loaded glTF: " << filename << std::endl;
    }

    return res;
}

void Model::initialize(const std::string &filename, float xpos, float ypos, float size, float rotation, glm::vec3 rotationAxis) {
    if (!loadModel(m_model, filename.c_str())) {
        return;
    }

    m_primitiveObjects = bindModel(m_model);

    m_programID = LoadShadersFromFile("../FinalProject/shader/model.vert", "../FinalProject/shader/model.frag");
    if (m_programID == 0) {
        std::cerr << "Failed to load shaders." << std::endl;
    }

    glUseProgram(m_programID);

    m_mvpMatrixID = glGetUniformLocation(m_programID, "MVPMatrix");
    m_lightPositionID = glGetUniformLocation(m_programID, "lightPos");
    m_lightIntensityID = glGetUniformLocation(m_programID, "lightIntensity");
    glUniform1i(glGetUniformLocation(m_programID, "modelTexture"), 0); // Texture unit 0

    // Create transformation matrix
    m_modelMatrix = glm::mat4(1.0f);
    m_modelMatrix = glm::translate(m_modelMatrix, glm::vec3(xpos, ypos, 0.0f));
    m_modelMatrix = glm::rotate(m_modelMatrix, glm::radians(rotation), rotationAxis);
    m_modelMatrix = glm::scale(m_modelMatrix, glm::vec3(size, size, size));

    size_t triangleCount = 0;
    for (const auto &mesh : m_model.meshes) {
        for (const auto &primitive : mesh.primitives) {
            if (primitive.indices >= 0) {
                const auto &indexAccessor = m_model.accessors[primitive.indices];
                triangleCount += indexAccessor.count / 3; // Assuming triangles
            }
        }
    }
    std::cout << "Total triangles in the model: " << triangleCount << std::endl;
}


std::vector<Model::PrimitiveObject> Model::bindModel(tinygltf::Model &model) {
    std::vector<PrimitiveObject> primitiveObjects;

    for (tinygltf::Mesh &mesh : model.meshes) {
        bindMesh(primitiveObjects, model, mesh);
    }

    return primitiveObjects;
}

void Model::bindMesh(std::vector<PrimitiveObject> &primitiveObjects, tinygltf::Model &model, tinygltf::Mesh &mesh) {
    std::map<int, GLuint> vbos;
    GLuint textureID = 0;

    // Load and bind buffer views
    for (size_t i = 0; i < model.bufferViews.size(); ++i) {
        const auto &bufferView = model.bufferViews[i];
        if (bufferView.target == 0) continue;

        const auto &buffer = model.buffers[bufferView.buffer];
        GLuint vbo;
        glGenBuffers(1, &vbo);
        glBindBuffer(bufferView.target, vbo);
        glBufferData(bufferView.target, bufferView.byteLength, &buffer.data.at(bufferView.byteOffset), GL_STATIC_DRAW);

        vbos[i] = vbo;
    }

    // Load and bind textures (if available)
    if (!model.textures.empty()) {
        const auto &texture = model.textures[0];
        const auto &image = model.images[texture.source];

        glGenTextures(1, &textureID);
        glBindTexture(GL_TEXTURE_2D, textureID);

        glTexImage2D(
            GL_TEXTURE_2D,
            0,
            GL_RGBA,
            image.width,
            image.height,
            0,
            GL_RGBA,
            GL_UNSIGNED_BYTE,
            image.image.data()
        );

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    }

    // Process mesh primitives
    for (const auto &primitive : mesh.primitives) {
        GLuint vao;
        glGenVertexArrays(1, &vao);
        glBindVertexArray(vao);

        for (const auto &attrib : primitive.attributes) {
            const auto &accessor = model.accessors[attrib.second];
            int byteStride = accessor.ByteStride(model.bufferViews[accessor.bufferView]);

            glBindBuffer(GL_ARRAY_BUFFER, vbos[accessor.bufferView]);

            int size = accessor.type == TINYGLTF_TYPE_SCALAR ? 1 : accessor.type;
            int vaa = -1;
            if (attrib.first == "POSITION") vaa = 0;
            else if (attrib.first == "NORMAL") vaa = 1;
            else if (attrib.first == "TEXCOORD_0") vaa = 2;

            if (vaa >= 0) {
                glEnableVertexAttribArray(vaa);
                glVertexAttribPointer(vaa, size, accessor.componentType,
                                      accessor.normalized ? GL_TRUE : GL_FALSE,
                                      byteStride, BUFFER_OFFSET(accessor.byteOffset));
            }
        }

        PrimitiveObject primitiveObject;
        primitiveObject.vao = vao;
        primitiveObject.vbos = vbos;
        primitiveObject.textureID = textureID;
        primitiveObjects.push_back(primitiveObject);

        glBindVertexArray(0);
    }
}

void Model::drawModel(const std::vector<PrimitiveObject> &primitiveObjects, tinygltf::Model &model) {
    for (size_t i = 0; i < primitiveObjects.size(); ++i) {
        const auto &primitiveObject = primitiveObjects[i];
        glBindVertexArray(primitiveObject.vao);

        // Bind texture
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, primitiveObject.textureID);

        for (const auto &primitive : model.meshes[i].primitives) {
            if (primitive.indices >= 0) {
                const auto &indexAccessor = model.accessors[primitive.indices];
                glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, primitiveObject.vbos.at(indexAccessor.bufferView));

                GLenum mode = GL_TRIANGLES; // Default to triangles
                if (primitive.mode != TINYGLTF_MODE_TRIANGLES) {
                    mode = primitive.mode;
                }

                glDrawElements(
                    mode,
                    indexAccessor.count,
                    indexAccessor.componentType,
                    BUFFER_OFFSET(indexAccessor.byteOffset)
                );
            }
        }

        glBindVertexArray(0);
    }
}

void Model::render(const glm::mat4 &cameraMatrix, const glm::vec3 &lightPosition, const glm::vec3 &lightIntensity) {
    glUseProgram(m_programID);

    glm::mat4 mvp = cameraMatrix * m_modelMatrix;
    glUniformMatrix4fv(m_mvpMatrixID, 1, GL_FALSE, glm::value_ptr(mvp));
    glUniform3fv(m_lightPositionID, 1, &lightPosition[0]);
    glUniform3fv(m_lightIntensityID, 1, &lightIntensity[0]);

    drawModel(m_primitiveObjects, m_model);
}

void Model::cleanup() {
    for (const auto &primitiveObject : m_primitiveObjects) {
        glDeleteVertexArrays(1, &primitiveObject.vao);
        for (const auto &vbo : primitiveObject.vbos) {
            glDeleteBuffers(1, &vbo.second);
        }
        glDeleteTextures(1, &primitiveObject.textureID);
    }
    m_primitiveObjects.clear();

    if (m_programID != 0) {
        glDeleteProgram(m_programID);
        m_programID = 0;
    }
}
