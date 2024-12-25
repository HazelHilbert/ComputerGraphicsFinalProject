#ifndef MODEL_H
#define MODEL_H

#include <glad/gl.h>
#include <glm/glm.hpp>
#include <tiny_gltf.h>
#include <vector>
#include <map>
#include <string>
#include <unordered_map>

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
    // Shared model data structure
    struct ModelData {
        tinygltf::Model m_model;
        std::vector<PrimitiveObject> m_primitiveObjects;
        GLuint m_programID;
        GLuint m_mvpMatrixID;
        GLuint m_lightPositionID;
        GLuint m_lightIntensityID;

        // Reference count to manage shared resources
        int refCount;

        ModelData() : m_programID(0), m_mvpMatrixID(0),
                     m_lightPositionID(0), m_lightIntensityID(0), refCount(1) {}
    };

    // Static map to cache loaded models
    static std::unordered_map<std::string, std::shared_ptr<ModelData>> s_modelCache;

    // Shared model data
    std::shared_ptr<ModelData> m_sharedData;

    // Transformation matrix for this instance
    glm::mat4 m_modelMatrix;

    // Helper functions
    bool loadModel(tinygltf::Model &model, const std::string &filename);
    std::vector<PrimitiveObject> bindModel(tinygltf::Model &model);
    void bindModelNodes(std::vector<PrimitiveObject> &primitiveObjects,
                        tinygltf::Model &model,
                        tinygltf::Node &node);
    void bindMesh(std::vector<PrimitiveObject> &primitiveObjects,
                  tinygltf::Model &model,
                  tinygltf::Mesh &mesh);

    void drawModel(const std::vector<PrimitiveObject> &primitiveObjects,
                   tinygltf::Model &model);
};

#endif // MODEL_H
