#include "TerrainManager.h"
#include "utils.h"
#include <cmath>
#include <iostream>

void TerrainManager::initialize(const glm::vec3& cameraPos) {
    currentCenter = getChunkPosition(cameraPos);

    createTerrainProgramIDs(programID,depthProgramID);

    // Load initial chunks within the view distance
    for(int x = -VIEW_DISTANCE; x <= VIEW_DISTANCE; ++x) {
        for(int z = -VIEW_DISTANCE; z <= VIEW_DISTANCE; ++z) {
            ChunkPosition cp = {currentCenter.x + x, currentCenter.z + z};
            loadChunk(cp);
        }
    }
}

// Determine the chunk position based on camera coordinates
ChunkPosition TerrainManager::getChunkPosition(const glm::vec3& pos) {
    return {
        static_cast<int>(std::floor(pos.x / static_cast<float>(CHUNK_SIZE))),
        static_cast<int>(std::floor(pos.z / static_cast<float>(CHUNK_SIZE)))
    };
}

// Load a chunk if it's not already loaded
void TerrainManager::loadChunk(const ChunkPosition& cp) {
    if(chunks.find(cp) != chunks.end()) return; // Already loaded

    Terrain terrain;
    terrain.setProgramIDs(programID, depthProgramID);
    // Initialize each chunk with its unique position
    // posX and posY are the world coordinates based on chunk grid
    float posX = cp.x * CHUNK_SIZE;
    float posZ = cp.z * CHUNK_SIZE;
    terrain.initialize(CHUNK_SIZE, CHUNK_SIZE, MAX_HEIGHT, posX, posZ);
    chunks.emplace(cp, std::move(terrain));

    //std::cout << "Loaded chunk (" << cp.x << ", " << cp.z << ")" << std::endl;
}

// Unload a chunk that's no longer within the view distance
void TerrainManager::unloadChunk(const ChunkPosition& cp) {
    auto it = chunks.find(cp);
    if(it != chunks.end()) {
        it->second.cleanup();
        chunks.erase(it);
        //std::cout << "Unloaded chunk (" << cp.x << ", " << cp.z << ")" << std::endl;
    }
}

// Update the TerrainManager based on the camera's new position
void TerrainManager::update(const glm::vec3& cameraPos) {
    ChunkPosition newCenter = getChunkPosition(cameraPos);

    if(newCenter.x != currentCenter.x || newCenter.z != currentCenter.z) {
        // Load new chunks within the new view distance
        for(int x = -VIEW_DISTANCE; x <= VIEW_DISTANCE; ++x) {
            for(int z = -VIEW_DISTANCE; z <= VIEW_DISTANCE; ++z) {
                ChunkPosition cp = {newCenter.x + x, newCenter.z + z};
                loadChunk(cp);
            }
        }

        // Unload chunks that are outside the view distance
        for(auto it = chunks.begin(); it != chunks.end(); ) {
            if(std::abs(it->first.x - newCenter.x) > VIEW_DISTANCE || std::abs(it->first.z - newCenter.z) > VIEW_DISTANCE) {
                it->second.cleanup();
                //std::cout << "Unloading chunk (" << it->first.x << ", " << it->first.z << ")" << std::endl;
                it = chunks.erase(it);
            } else {
                ++it;
            }
        }

        currentCenter = newCenter;
    }
}

void TerrainManager::render(const glm::mat4& vp, const glm::mat4& lightSpaceMatrix, const glm::vec3& lightDirection, const glm::vec3& lightIntensity, const glm::vec3& cameraPos) {
    update(cameraPos);

    for(auto& pair : chunks) {
        pair.second.render(vp, lightSpaceMatrix, lightDirection, lightIntensity);
    }
}

void TerrainManager::cleanup() {
    for(auto& pair : chunks) {
        pair.second.cleanup();
    }
    chunks.clear();

    glDeleteProgram(programID);
    glDeleteProgram(depthProgramID);
}