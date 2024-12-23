#include "TerrainManager.h"
#include "utils.h"
#include <cmath>
#include <iostream>
#include <set>
#include <unordered_set>

int TerrainManager::findChunkIndex(const ChunkPosition& cp) const {
    for (size_t i = 0; i < chunks.size(); ++i) {
        if (chunks[i].position == cp) {
            return static_cast<int>(i);
        }
    }
    return -1;
}

void TerrainManager::initialize(const glm::vec3& cameraPos) {
    currentCenter = getChunkPosition(cameraPos);

    createTerrainProgramIDs(programID,depthProgramID);

    // Load initial chunks within the view distance
    for(int x = -VIEW_DISTANCE; x <= VIEW_DISTANCE; ++x) {
        for(int z = -VIEW_DISTANCE; z <= VIEW_DISTANCE; ++z) {
            ChunkPosition cp = {currentCenter.x + x, currentCenter.z + z};

            Terrain terrain;
            terrain.setProgramIDs(programID, depthProgramID);

            // Initialize each chunk with its unique position
            // posX and posY are the world coordinates based on chunk grid
            float posX = cp.x * CHUNK_SIZE;
            float posZ = cp.z * CHUNK_SIZE;
            terrain.initialize(CHUNK_SIZE, CHUNK_SIZE, MAX_HEIGHT, posX, posZ);

            Chunk newChunk;
            newChunk.position = cp;
            newChunk.terrain = std::move(terrain);
            chunks.emplace_back(std::move(newChunk));
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

// Update the TerrainManager based on the camera's new position
void TerrainManager::update(const glm::vec3& cameraPos) {
    ChunkPosition newCenter = getChunkPosition(cameraPos);

    const int deltaX = newCenter.x - currentCenter.x;
    const int deltaZ = newCenter.z - currentCenter.z;

    if(deltaX == 0 && deltaZ == 0) return;

    std::vector<ChunkPosition> chunksToAdd;
    std::vector<ChunkPosition> chunksToReplace;

    // Handle movement along X axis
    if (deltaX > 0) { // Moving East
        for(int x = 1; x <= deltaX; ++x) {
            for(int z = -VIEW_DISTANCE; z <= VIEW_DISTANCE; ++z) {
                ChunkPosition cp = {newCenter.x + VIEW_DISTANCE, newCenter.z + z};
                chunksToAdd.push_back(cp);
            }
        }
        // Identify chunks to replace on the West side
        for(int x = 0; x < deltaX; ++x) {
            for(int z = -VIEW_DISTANCE; z <= VIEW_DISTANCE; ++z) {
                ChunkPosition cp = {currentCenter.x - VIEW_DISTANCE + x, currentCenter.z + z};
                chunksToReplace.push_back(cp);
            }
        }
    }
    else if (deltaX < 0) { // Moving West
        for(int z = -VIEW_DISTANCE; z <= VIEW_DISTANCE; ++z) {
            ChunkPosition cp = {newCenter.x - VIEW_DISTANCE, newCenter.z + z};
            chunksToAdd.push_back(cp);
        }
        // Identify chunks to replace on the East side
        for(int x = 0; x < -deltaX; ++x) {
            for(int z = -VIEW_DISTANCE; z <= VIEW_DISTANCE; ++z) {
                ChunkPosition cp = {currentCenter.x + VIEW_DISTANCE + x, currentCenter.z + z};
                chunksToReplace.push_back(cp);
            }
        }
    }

    // Handle movement along Z axis
    if (deltaZ > 0) { // Moving North
        for(int z = 1; z <= deltaZ; ++z) {
            for(int x = -VIEW_DISTANCE; x <= VIEW_DISTANCE; ++x) {
                ChunkPosition cp = {newCenter.x + x, newCenter.z + VIEW_DISTANCE};
                bool add = true;
                if (deltaX != 0) {
                    for (auto & chunk : chunksToAdd) {
                        if (chunk.x == cp.x && chunk.z == cp.z) add = false;
                    }
                }
                if (add) chunksToAdd.push_back(cp);
            }
        }
        // Identify chunks to replace on the South side
        for(int z = 0; z < deltaZ; ++z) {
            for(int x = -VIEW_DISTANCE; x <= VIEW_DISTANCE; ++x) {
                ChunkPosition cp = {currentCenter.x + x, currentCenter.z - VIEW_DISTANCE + z};
                bool replace = true;
                if (deltaX != 0) {
                    for (auto & chunk : chunksToReplace) {
                        if (chunk.x == cp.x && chunk.z == cp.z) replace = false;
                    }
                }
                if (replace) chunksToReplace.push_back(cp);
            }
        }
    }

    else if (deltaZ < 0) { // Moving South
        for(int x = -VIEW_DISTANCE; x <= VIEW_DISTANCE; ++x) {
            ChunkPosition cp = {newCenter.x + x, newCenter.z - VIEW_DISTANCE};
            bool add = true;
            if (deltaX != 0) {
                for (auto & chunk : chunksToAdd) {
                    if (chunk.x == cp.x && chunk.z == cp.z) add = false;
                }
            }
            if (add) chunksToAdd.push_back(cp);
        }
        // Identify chunks to replace on the North side
        for(int z = 0; z < -deltaZ; ++z) {
            for(int x = -VIEW_DISTANCE; x <= VIEW_DISTANCE; ++x) {
                ChunkPosition cp = {currentCenter.x + x, currentCenter.z + VIEW_DISTANCE + z};
                bool replace = true;
                if (deltaX != 0) {
                    for (auto & chunk : chunksToReplace) {
                        if (chunk.x == cp.x && chunk.z == cp.z) replace = false;
                    }
                }
                if (replace) chunksToReplace.push_back(cp);
            }
        }
    }

    for (int i = 0; i < chunksToReplace.size(); i++) {
        ChunkPosition chunkToAddPosition = chunksToAdd[i];
        int posX = chunkToAddPosition.x * CHUNK_SIZE;
        int posZ = chunkToAddPosition.z * CHUNK_SIZE;

        Chunk& updatedChunk = chunks.at(findChunkIndex(chunksToReplace[i]));
        updatedChunk.position = chunkToAddPosition;
        updatedChunk.terrain.setTerrain(CHUNK_SIZE, CHUNK_SIZE, MAX_HEIGHT, posX, posZ);
    }

    currentCenter = newCenter;
}

void TerrainManager::render(const glm::mat4& vp, const glm::mat4& lightSpaceMatrix, const glm::vec3& lightDirection, const glm::vec3& lightIntensity, const glm::vec3& cameraPos) {
    update(cameraPos);

    for(auto& chunk : chunks) {
        chunk.terrain.render(vp, lightSpaceMatrix, lightDirection, lightIntensity);
    }
}

void TerrainManager::cleanup() {
    for(auto& chunk : chunks) {
        chunk.terrain.cleanup();
    }
    chunks.clear();

    glDeleteProgram(programID);
    glDeleteProgram(depthProgramID);
}