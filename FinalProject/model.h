#ifndef MODEL_H
#define MODEL_H

#include <glad/gl.h>
#include <glm/glm.hpp>
#include <tiny_gltf.h>
#include <vector>
#include <map>


class Model
{
public:

    struct PrimitiveObject {
        GLuint vao;
        std::map<int, GLuint> vbos;
        GLuint textureID;
    };

    void initialize(const std::string &filename, float xpos, float ypos, float zpos, float size, float rotation, glm::vec3 rotationAxis);

    void render(const glm::mat4 &cameraMatrix, const glm::vec3 &lightPosition, const glm::vec3 &lightIntensity);

    void cleanup();

private:
    tinygltf::Model m_model;
    std::vector<PrimitiveObject> m_primitiveObjects;
    glm::mat4 m_modelMatrix;

    // GLSL program and uniform locations
    GLuint m_programID = 0;
    GLuint m_mvpMatrixID = 0;
    GLuint m_lightPositionID = 0;
    GLuint m_lightIntensityID = 0;

    // helper functions
    bool loadModel(tinygltf::Model &model, const char *filename);

    std::vector<PrimitiveObject> bindModel(tinygltf::Model &model);
    void bindModelNodes(std::vector<PrimitiveObject> &primitiveObjects,
                        tinygltf::Model &model,
                        tinygltf::Node &node);
    void bindMesh(std::vector<PrimitiveObject> &primitiveObjects,
                  tinygltf::Model &model,
                  tinygltf::Mesh &mesh);

    void drawModel(const std::vector<PrimitiveObject> &primitiveObjects,
                   tinygltf::Model &model);
    void drawModelNodes(const std::vector<PrimitiveObject> &primitiveObjects,
                        tinygltf::Model &model,
                        tinygltf::Node &node);
    void drawMesh(const std::vector<PrimitiveObject> &primitiveObjects,
                  tinygltf::Model &model,
                  tinygltf::Mesh &mesh);
};

#endif
