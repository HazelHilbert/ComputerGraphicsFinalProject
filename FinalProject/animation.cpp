#include "animation.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <string>
#include <iostream>
#include <vector>
#include <cassert>
#include <random>
#include <math.h>
#include <render/shader.h>

// GLTF model loader
#define TINYGLTF_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <tiny_gltf.h>

#define BUFFER_OFFSET(i) ((char *)NULL + (i))

glm::mat4 MyBot::getNodeTransform(const tinygltf::Node& node) {
	glm::mat4 transform(1.0f);

	if (node.matrix.size() == 16) {
		transform = glm::make_mat4(node.matrix.data());
	} else {
		if (node.translation.size() == 3) {
			transform = glm::translate(transform, glm::vec3(node.translation[0], node.translation[1], node.translation[2]));
		}
		if (node.rotation.size() == 4) {
			glm::quat q(node.rotation[3], node.rotation[0], node.rotation[1], node.rotation[2]);
			transform *= glm::mat4_cast(q);
		}
		if (node.scale.size() == 3) {
			transform = glm::scale(transform, glm::vec3(node.scale[0], node.scale[1], node.scale[2]));
		}
	}
	return transform;
}

void MyBot::computeLocalNodeTransform(const tinygltf::Model& model,
	int nodeIndex,
	std::vector<glm::mat4> &localTransforms)
{
	// ---------------------------------------
	// TODO: your code here
	// ---------------------------------------
	tinygltf::Node node = model.nodes[nodeIndex];
	localTransforms[nodeIndex] = getNodeTransform(node);
	for (int childIndex : node.children) {
		computeLocalNodeTransform(model, childIndex, localTransforms);
	}
}

void MyBot::computeGlobalNodeTransform(const tinygltf::Model& model,
	const std::vector<glm::mat4> &localTransforms,
	int nodeIndex, const glm::mat4& parentTransform,
	std::vector<glm::mat4> &globalTransforms)
{
	// ----------------------------------------
	// TODO: your code here
	// ----------------------------------------
	globalTransforms[nodeIndex] = parentTransform * localTransforms[nodeIndex];
	for (int childIndex : model.nodes[nodeIndex].children) {
		computeGlobalNodeTransform(model, localTransforms, childIndex, globalTransforms[nodeIndex], globalTransforms);
	}
}

std::vector<MyBot::SkinObject> MyBot::prepareSkinning(const tinygltf::Model &model) {
	std::vector<SkinObject> skinObjects;

	// In our Blender exporter, the default number of joints that may influence a vertex is set to 4, just for convenient implementation in shaders.

	for (size_t i = 0; i < model.skins.size(); i++) {
		SkinObject skinObject;

		const tinygltf::Skin &skin = model.skins[i];

		// Read inverseBindMatrices
		const tinygltf::Accessor &accessor = model.accessors[skin.inverseBindMatrices];
		assert(accessor.type == TINYGLTF_TYPE_MAT4);
		const tinygltf::BufferView &bufferView = model.bufferViews[accessor.bufferView];
		const tinygltf::Buffer &buffer = model.buffers[bufferView.buffer];
		const float *ptr = reinterpret_cast<const float *>(
            buffer.data.data() + accessor.byteOffset + bufferView.byteOffset);

		skinObject.inverseBindMatrices.resize(accessor.count);
		for (size_t j = 0; j < accessor.count; j++) {
			float m[16];
			memcpy(m, ptr + j * 16, 16 * sizeof(float));
			skinObject.inverseBindMatrices[j] = glm::make_mat4(m);
		}

		assert(skin.joints.size() == accessor.count);

		skinObject.globalJointTransforms.resize(skin.joints.size());
		skinObject.jointMatrices.resize(skin.joints.size());

		// ----------------------------------------------
		// TODO: your code here to compute joint matrices
		// ----------------------------------------------
		rootNodeIndex = skin.joints.size()-1;
		std::cout << "rootNodeIndex = " << rootNodeIndex << std::endl;

		std::vector<glm::mat4> localTransforms(model.nodes.size(), glm::mat4(1.0f));
		computeLocalNodeTransform(model, rootNodeIndex, localTransforms);

		std::vector<glm::mat4> globalTransforms(model.nodes.size(), glm::mat4(1.0f));
		computeGlobalNodeTransform(model, localTransforms, rootNodeIndex, glm::mat4(1.0f), globalTransforms);

		for (int j = 0; j < skinObject.jointMatrices.size(); j++) {
			skinObject.jointMatrices[j] = globalTransforms[skin.joints[j]] * skinObject.inverseBindMatrices[j];
		}
		// ----------------------------------------------

		skinObjects.push_back(skinObject);
	}
	return skinObjects;
}

int MyBot::findKeyframeIndex(const std::vector<float>& times, float animationTime)
{
	int left = 0;
	int right = times.size() - 1;

	while (left <= right) {
		int mid = (left + right) / 2;

		if (mid + 1 < times.size() && times[mid] <= animationTime && animationTime < times[mid + 1]) {
			return mid;
		}
		else if (times[mid] > animationTime) {
			right = mid - 1;
		}
		else { // animationTime >= times[mid + 1]
			left = mid + 1;
		}
	}

	// Target not found
	return times.size() - 2;
}

std::vector<MyBot::AnimationObject> MyBot::prepareAnimation(const tinygltf::Model &model)
{
	std::vector<AnimationObject> animationObjects;
	for (const auto &anim : model.animations) {
		AnimationObject animationObject;

		for (const auto &sampler : anim.samplers) {
			SamplerObject samplerObject;

			const tinygltf::Accessor &inputAccessor = model.accessors[sampler.input];
			const tinygltf::BufferView &inputBufferView = model.bufferViews[inputAccessor.bufferView];
			const tinygltf::Buffer &inputBuffer = model.buffers[inputBufferView.buffer];

			assert(inputAccessor.componentType == TINYGLTF_COMPONENT_TYPE_FLOAT);
			assert(inputAccessor.type == TINYGLTF_TYPE_SCALAR);

			// Input (time) values
			samplerObject.input.resize(inputAccessor.count);

			const unsigned char *inputPtr = &inputBuffer.data[inputBufferView.byteOffset + inputAccessor.byteOffset];
			const float *inputBuf = reinterpret_cast<const float*>(inputPtr);

			// Read input (time) values
			int stride = inputAccessor.ByteStride(inputBufferView);
			for (size_t i = 0; i < inputAccessor.count; ++i) {
				samplerObject.input[i] = *reinterpret_cast<const float*>(inputPtr + i * stride);
			}

			const tinygltf::Accessor &outputAccessor = model.accessors[sampler.output];
			const tinygltf::BufferView &outputBufferView = model.bufferViews[outputAccessor.bufferView];
			const tinygltf::Buffer &outputBuffer = model.buffers[outputBufferView.buffer];

			assert(outputAccessor.componentType == TINYGLTF_COMPONENT_TYPE_FLOAT);

			const unsigned char *outputPtr = &outputBuffer.data[outputBufferView.byteOffset + outputAccessor.byteOffset];
			const float *outputBuf = reinterpret_cast<const float*>(outputPtr);

			int outputStride = outputAccessor.ByteStride(outputBufferView);

			// Output values
			samplerObject.output.resize(outputAccessor.count);

			for (size_t i = 0; i < outputAccessor.count; ++i) {

				if (outputAccessor.type == TINYGLTF_TYPE_VEC3) {
					memcpy(&samplerObject.output[i], outputPtr + i * 3 * sizeof(float), 3 * sizeof(float));
				} else if (outputAccessor.type == TINYGLTF_TYPE_VEC4) {
					memcpy(&samplerObject.output[i], outputPtr + i * 4 * sizeof(float), 4 * sizeof(float));
				} else {
					std::cout << "Unsupport accessor type ..." << std::endl;
				}

			}

			animationObject.samplers.push_back(samplerObject);
		}

		animationObjects.push_back(animationObject);
	}
	return animationObjects;
}

void MyBot::updateAnimation(
	const tinygltf::Model &model,
	const tinygltf::Animation &anim,
	const AnimationObject &animationObject,
	float time,
	std::vector<glm::mat4> &nodeTransforms)
{
	// There are many channels so we have to accumulate the transforms
	for (const auto &channel : anim.channels) {

		int targetNodeIndex = channel.target_node;
		const auto &sampler = anim.samplers[channel.sampler];

		// Access output (value) data for the channel
		const tinygltf::Accessor &outputAccessor = model.accessors[sampler.output];
		const tinygltf::BufferView &outputBufferView = model.bufferViews[outputAccessor.bufferView];
		const tinygltf::Buffer &outputBuffer = model.buffers[outputBufferView.buffer];

		// Calculate current animation time (wrap if necessary)
		const std::vector<float> &times = animationObject.samplers[channel.sampler].input;
		float animationTime = fmod(time, times.back());

		// ----------------------------------------------------------
		// TODO: Find a keyframe for getting animation data
		// ----------------------------------------------------------
		int keyframeIndex = findKeyframeIndex(times, animationTime);

		const unsigned char *outputPtr = &outputBuffer.data[outputBufferView.byteOffset + outputAccessor.byteOffset];
		const float *outputBuf = reinterpret_cast<const float*>(outputPtr);

		// -----------------------------------------------------------
		// TODO: Add interpolation for smooth animation
		// -----------------------------------------------------------
		float t = 0.0f;
		if (keyframeIndex + 1 < times.size()) {
			float time1 = times[keyframeIndex];
			float time2 = times[keyframeIndex + 1];
			t = (animationTime - time1) / (time2 - time1);
		}

		if (channel.target_path == "translation") {
			glm::vec3 translation0, translation1;
			memcpy(&translation0, outputPtr + keyframeIndex * 3 * sizeof(float), 3 * sizeof(float));
			memcpy(&translation1, outputPtr + (keyframeIndex + 1) * 3 * sizeof(float), 3 * sizeof(float));

			glm::vec3 translation = translation0;//glm::mix(translation0, translation1, t);
			nodeTransforms[targetNodeIndex] = glm::translate(nodeTransforms[targetNodeIndex], translation);
		} else if (channel.target_path == "rotation") {
			glm::quat rotation0, rotation1;
			memcpy(&rotation0, outputPtr + keyframeIndex * 4 * sizeof(float), 4 * sizeof(float));
			memcpy(&rotation1, outputPtr + (keyframeIndex + 1) * 4 * sizeof(float), 4 * sizeof(float));

			glm::quat rotation = rotation0;//glm::slerp(rotation0, rotation1, t);
			nodeTransforms[targetNodeIndex] *= glm::mat4_cast(rotation);
		} else if (channel.target_path == "scale") {
			glm::vec3 scale0, scale1;
			memcpy(&scale0, outputPtr + keyframeIndex * 3 * sizeof(float), 3 * sizeof(float));
			memcpy(&scale1, outputPtr + (keyframeIndex + 1) * 3 * sizeof(float), 3 * sizeof(float));

			glm::vec3 scale = scale0;//glm::mix(scale0, scale1, t);
			nodeTransforms[targetNodeIndex] = glm::scale(nodeTransforms[targetNodeIndex], scale);
		}
	}
}

void MyBot::updateSkinning(const std::vector<glm::mat4> &nodeTransforms) {

	// -------------------------------------------------
	// TODO: Recompute joint matrices
	// -------------------------------------------------

	for (SkinObject &skinObject : skinObjects) {
		for (int j = 0; j < skinObject.jointMatrices.size(); j++) {
			int jointNodeIndex = model.skins[0].joints[j];
			skinObject.jointMatrices[j] = nodeTransforms[jointNodeIndex] * skinObject.inverseBindMatrices[j];
		}
	}
}

void MyBot::update(float time) {

	// -------------------------------------------------
	// TODO: your code here
	// -------------------------------------------------

	if (model.animations.empty()) return;

	const tinygltf::Animation &animation = model.animations[0];
	const AnimationObject &animationObject = animationObjects[0];
	std::vector<glm::mat4> nodeTransforms(model.nodes.size(), glm::mat4(1.0f));

	updateAnimation(model, animation, animationObject, time, nodeTransforms);
	computeGlobalNodeTransform(model, nodeTransforms, rootNodeIndex, glm::mat4(1.0f), nodeTransforms);
	updateSkinning(nodeTransforms);
}

bool MyBot::loadModel(tinygltf::Model &model, const char *filename) {
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

void MyBot::initialize() {
	// Modify your path if needed
	if (!loadModel(model, "../FinalProject/assets/model/fox/fox.gltf")){ //"../FinalProject/assets/model/bot/bot.gltf")) {
		return;
	}

	// Prepare buffers for rendering
	primitiveObjects = bindModel(model);

	// Prepare joint matrices
	skinObjects = prepareSkinning(model);

	// Prepare animation data
	animationObjects = prepareAnimation(model);

	// Create and compile our GLSL program from the shaders
	animationProgramID = LoadShadersFromFile("../FinalProject/shader/bot.vert", "../FinalProject/shader/bot.frag");
	if (animationProgramID == 0)
	{
		std::cerr << "Failed to load shaders." << std::endl;
	}

	// Get a handle for GLSL variables
	mvpMatrixID = glGetUniformLocation(animationProgramID, "MVP");
	jointMatricesID = glGetUniformLocation(animationProgramID, "jointMatrices");
	lightPositionID = glGetUniformLocation(animationProgramID, "lightPosition");
	lightIntensityID = glGetUniformLocation(animationProgramID, "lightIntensity");

	// Set the sampler to texture unit 0
	glUseProgram(animationProgramID);
	GLint diffuseTextureLoc = glGetUniformLocation(animationProgramID, "diffuseTexture");
	glUniform1i(diffuseTextureLoc, 0); // Texture unit 0
}


void MyBot::bindMesh(std::vector<PrimitiveObject> &primitiveObjects,
                     tinygltf::Model &model, tinygltf::Mesh &mesh) {

    std::map<int, GLuint> vbos;

    // Load bufferViews into VBOs
    for (size_t i = 0; i < model.bufferViews.size(); ++i) {
        const tinygltf::BufferView &bufferView = model.bufferViews[i];
        int target = bufferView.target;

        if (target == 0) {
            // Skip bufferViews with target 0 (unused or special)
            continue;
        }

        const tinygltf::Buffer &buffer = model.buffers[bufferView.buffer];
        GLuint vbo;
        glGenBuffers(1, &vbo);
        glBindBuffer(target, vbo);
        glBufferData(target, bufferView.byteLength,
                    &buffer.data.at(bufferView.byteOffset), GL_STATIC_DRAW);

        vbos[i] = vbo;
    }

    // Process each primitive in the mesh
    for (size_t i = 0; i < mesh.primitives.size(); ++i) {
        const tinygltf::Primitive &primitive = mesh.primitives[i];
        const tinygltf::Material &material = model.materials[primitive.material];

        GLuint vao;
        glGenVertexArrays(1, &vao);
        glBindVertexArray(vao);

        // Bind vertex attributes
        for (const auto &attrib : primitive.attributes) {
            const tinygltf::Accessor &accessor = model.accessors[attrib.second];
            const tinygltf::BufferView &bufferView = model.bufferViews[accessor.bufferView];
            int byteStride = accessor.ByteStride(bufferView);

            glBindBuffer(GL_ARRAY_BUFFER, vbos[accessor.bufferView]);

            int size = accessor.type == TINYGLTF_TYPE_SCALAR ? 1 : accessor.type;
            int attribLocation = -1;
            if (attrib.first == "POSITION") attribLocation = 0;
            else if (attrib.first == "NORMAL") attribLocation = 1;
            else if (attrib.first == "TEXCOORD_0") attribLocation = 2;
            else if (attrib.first == "JOINTS_0") attribLocation = 3;
            else if (attrib.first == "WEIGHTS_0") attribLocation = 4;

            if (attribLocation >= 0) {
                glEnableVertexAttribArray(attribLocation);
                glVertexAttribPointer(
                    attribLocation,
                    size,
                    accessor.componentType,
                    accessor.normalized ? GL_TRUE : GL_FALSE,
                    byteStride,
                    BUFFER_OFFSET(accessor.byteOffset)
                );
            } else {
                std::cout << "Attribute location missing for: " << attrib.first << std::endl;
            }
        }

        // Handle indices
        GLuint indexVBO = 0;
        if (primitive.indices >= 0) {
            const tinygltf::Accessor &indexAccessor = model.accessors[primitive.indices];
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbos[indexAccessor.bufferView]);
        }

        // Create PrimitiveObject
        PrimitiveObject primitiveObject;
        primitiveObject.vao = vao;
        primitiveObject.vbos = vbos;
        primitiveObject.textureID = 0; // Default to 0 (no texture)

        // Load the diffuse texture if available
        if (material.values.find("baseColorTexture") != material.values.end()) {
            int textureIndex = material.values.at("baseColorTexture").TextureIndex();
            if (textureIndex >= 0 && textureIndex < model.textures.size()) {
                const tinygltf::Texture &tex = model.textures[textureIndex];
                const tinygltf::Image &image = model.images[tex.source];

                glGenTextures(1, &primitiveObject.textureID);
                glBindTexture(GL_TEXTURE_2D, primitiveObject.textureID);

                GLenum format = GL_RGB;
                if (image.component == 1) format = GL_RED;
                else if (image.component == 2) format = GL_RG;
                else if (image.component == 3) format = GL_RGB;
                else if (image.component == 4) format = GL_RGBA;

                GLenum internalFormat = GL_RGBA;
                if (image.pixel_type == TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE) {
                    if (image.component == 3) internalFormat = GL_RGB;
                    else if (image.component == 4) internalFormat = GL_RGBA;
                }

                glTexImage2D(
                    GL_TEXTURE_2D,
                    0,
                    internalFormat,
                    image.width,
                    image.height,
                    0,
                    format,
                    GL_UNSIGNED_BYTE,
                    &image.image.at(0)
                );

                glGenerateMipmap(GL_TEXTURE_2D);

                // Set texture parameters
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            }
        }

        primitiveObjects.push_back(primitiveObject);
        glBindVertexArray(0);
    }
}


void MyBot::bindModelNodes(std::vector<PrimitiveObject> &primitiveObjects,
					tinygltf::Model &model,
					tinygltf::Node &node) {
	// Bind buffers for the current mesh at the node
	if ((node.mesh >= 0) && (node.mesh < model.meshes.size())) {
		bindMesh(primitiveObjects, model, model.meshes[node.mesh]);
	}

	// Recursive into children nodes
	for (size_t i = 0; i < node.children.size(); i++) {
		assert((node.children[i] >= 0) && (node.children[i] < model.nodes.size()));
		bindModelNodes(primitiveObjects, model, model.nodes[node.children[i]]);
	}
}

std::vector<MyBot::PrimitiveObject> MyBot::bindModel(tinygltf::Model &model) {
	std::vector<PrimitiveObject> primitiveObjects;

	const tinygltf::Scene &scene = model.scenes[model.defaultScene];
	for (size_t i = 0; i < scene.nodes.size(); ++i) {
		assert((scene.nodes[i] >= 0) && (scene.nodes[i] < model.nodes.size()));
		bindModelNodes(primitiveObjects, model, model.nodes[scene.nodes[i]]);
	}

	return primitiveObjects;
}

void MyBot::drawMesh(const std::vector<PrimitiveObject> &primitiveObjects,
					tinygltf::Model &model, tinygltf::Mesh &mesh) {

	for (size_t i = 0; i < mesh.primitives.size(); ++i) {
		const PrimitiveObject &primitiveObject = primitiveObjects[i];
		glBindVertexArray(primitiveObject.vao);

		// Bind the diffuse texture
		if (primitiveObject.textureID != 0) {
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, primitiveObject.textureID);
		} else {
			// Bind a default texture or unbind if no texture is present
			glBindTexture(GL_TEXTURE_2D, 0);
		}

		// Set the sampler uniform to texture unit 0
		// It's efficient to do this once in initialize, but ensuring it's correct here
		glUniform1i(glGetUniformLocation(animationProgramID, "diffuseTexture"), 0);

		// Bind index buffer and draw
		const tinygltf::Primitive &primitive = mesh.primitives[i];
		if (primitive.indices >= 0) {
			const tinygltf::Accessor &indexAccessor = model.accessors[primitive.indices];
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

		glBindVertexArray(0);
	}
}


void MyBot::drawModelNodes(const std::vector<PrimitiveObject>& primitiveObjects,
					tinygltf::Model &model, tinygltf::Node &node) {
	// Draw the mesh at the node, and recursively do so for children nodes
	if ((node.mesh >= 0) && (node.mesh < model.meshes.size())) {
		drawMesh(primitiveObjects, model, model.meshes[node.mesh]);
	}
	for (size_t i = 0; i < node.children.size(); i++) {
		drawModelNodes(primitiveObjects, model, model.nodes[node.children[i]]);
	}
}
void MyBot::drawModel(const std::vector<PrimitiveObject>& primitiveObjects,
			tinygltf::Model &model) {
	// Draw all nodes
	const tinygltf::Scene &scene = model.scenes[model.defaultScene];
	for (size_t i = 0; i < scene.nodes.size(); ++i) {
		drawModelNodes(primitiveObjects, model, model.nodes[scene.nodes[i]]);
	}
}

void MyBot::render(glm::mat4 cameraMatrix, glm::vec3 lightPosition, glm::vec3 lightIntensity) {
	glUseProgram(animationProgramID);

	// Set camera
	glm::mat4 mvp = cameraMatrix;
	glUniformMatrix4fv(mvpMatrixID, 1, GL_FALSE, &mvp[0][0]);

	// -----------------------------------------------------------------
	// TODO: Set animation data for linear blend skinning in shader
	// -----------------------------------------------------------------

	const std::vector<glm::mat4> &jointMatrices = skinObjects[0].jointMatrices;
	glUniformMatrix4fv(jointMatricesID, jointMatrices.size(), GL_FALSE, glm::value_ptr(jointMatrices[0]));

	// -----------------------------------------------------------------

	// Set light data
	glUniform3fv(lightPositionID, 1, &lightPosition[0]);
	glUniform3fv(lightIntensityID, 1, &lightIntensity[0]);

	// Draw the GLTF model
	drawModel(primitiveObjects, model);
}

void MyBot::cleanup() {
	glDeleteProgram(animationProgramID);
}
