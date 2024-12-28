#ifndef ANIMATION_H
#define ANIMATION_H

#include <glad/gl.h>
#include <glm/glm.hpp>

#include <tiny_gltf.h>
#include <vector>
#include <map>

#define BUFFER_OFFSET(i) ((char *)NULL + (i))

struct MyBot {
    struct PrimitiveObject {
        GLuint vao;
        std::map<int, GLuint> vbos;
        GLuint textureID;
    };

    struct SkinObject {
        std::vector<glm::mat4> inverseBindMatrices;
        std::vector<glm::mat4> globalJointTransforms;
        std::vector<glm::mat4> jointMatrices;
    };

    struct SamplerObject {
        std::vector<float> input;
        std::vector<glm::vec4> output;
        int interpolation;
    };

    struct AnimationObject {
        std::vector<SamplerObject> samplers;
    };

    GLuint mvpMatrixID;
    GLuint jointMatricesID;
    GLuint lightPositionID;
    GLuint lightIntensityID;
    GLuint animationProgramID;

    tinygltf::Model model;
    std::vector<PrimitiveObject> primitiveObjects;
    std::vector<SkinObject> skinObjects;
    std::vector<AnimationObject> animationObjects;

    int rootNodeIndex;

    glm::mat4 getNodeTransform(const tinygltf::Node &node);
    void computeLocalNodeTransform(const tinygltf::Model &model, int nodeIndex, std::vector<glm::mat4> &localTransforms);
    void computeGlobalNodeTransform(const tinygltf::Model &model, const std::vector<glm::mat4> &localTransforms, int nodeIndex, const glm::mat4 &parentTransform, std::vector<glm::mat4> &globalTransforms);

    std::vector<SkinObject> prepareSkinning(const tinygltf::Model &model);
    int findKeyframeIndex(const std::vector<float> &times, float animationTime);
    std::vector<AnimationObject> prepareAnimation(const tinygltf::Model &model);
    void updateAnimation(const tinygltf::Model &model, const tinygltf::Animation &anim, const AnimationObject &animationObject, float time, std::vector<glm::mat4> &nodeTransforms);
    void updateSkinning(const std::vector<glm::mat4> &nodeTransforms);
    void update(float time);

    bool loadModel(tinygltf::Model &model, const char *filename);
    void initialize();

    void bindMesh(std::vector<PrimitiveObject> &primitiveObjects, tinygltf::Model &model, tinygltf::Mesh &mesh);
    void bindModelNodes(std::vector<PrimitiveObject> &primitiveObjects, tinygltf::Model &model, tinygltf::Node &node);
    std::vector<PrimitiveObject> bindModel(tinygltf::Model &model);

    void drawMesh(const std::vector<PrimitiveObject> &primitiveObjects, tinygltf::Model &model, tinygltf::Mesh &mesh);
    void drawModelNodes(const std::vector<PrimitiveObject> &primitiveObjects, tinygltf::Model &model, tinygltf::Node &node);
    void drawModel(const std::vector<PrimitiveObject> &primitiveObjects, tinygltf::Model &model);

    void render(glm::mat4 cameraMatrix, glm::vec3 lightPosition, glm::vec3 lightIntensity);
    void cleanup();
};

#endif
