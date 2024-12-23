#include "TerrainManager.h"
#include "utils.h"
#include <iostream>
#include <set>

// Helper function to create a unique_ptr for Chunk
std::unique_ptr<Chunk> createChunk(const ChunkPosition& cp, GLuint programID, GLuint depthProgramID) {
    std::unique_ptr<Chunk> chunk(new Chunk());
    chunk->position = cp;

    // Initialize Terrain within the Chunk
    chunk->terrain.setProgramIDs(programID, depthProgramID);

    // Initialize each chunk with its unique position
    float posX = cp.x * CHUNK_SIZE;
    float posZ = cp.z * CHUNK_SIZE;
    chunk->terrain.initialize(CHUNK_SIZE, CHUNK_SIZE, MAX_HEIGHT, posX, posZ);

    return chunk;
}

int TerrainManager::findChunkIndex(const ChunkPosition& cp) const {
    for (size_t i = 0; i < chunks.size(); ++i) {
        if (chunks[i]->position == cp) {
            return static_cast<int>(i);
        }
    }
    return -1;
}

// Determine the chunk position based on camera coordinates
ChunkPosition TerrainManager::getChunkPosition(const glm::vec3& pos) {
    return {
        static_cast<int>(std::floor(pos.x / static_cast<float>(CHUNK_SIZE))),
        static_cast<int>(std::floor(pos.z / static_cast<float>(CHUNK_SIZE)))
    };
}

bool TerrainManager::chucksToAddReplace(ChunkPosition newCenter,
                                        std::vector<ChunkPosition>& chunksToAdd,
                                        std::vector<ChunkPosition>& chunksToReplace)
{
    const int deltaX = newCenter.x - currentCenter.x;
    const int deltaZ = newCenter.z - currentCenter.z;

    // If no change in center, nothing to do
    if(deltaX == 0 && deltaZ == 0) return false;

    // Handle movement along X axis
    if (deltaX > 0) { // Moving East
        for(int x = 1; x <= deltaX; ++x) {
            for(int z = -VIEW_DISTANCE; z <= VIEW_DISTANCE; ++z) {
                ChunkPosition cp = { newCenter.x + VIEW_DISTANCE, newCenter.z + z };
                chunksToAdd.push_back(cp);
            }
        }
        // Identify chunks to replace on the West side
        for(int x = 0; x < deltaX; ++x) {
            for(int z = -VIEW_DISTANCE; z <= VIEW_DISTANCE; ++z) {
                ChunkPosition cp = { currentCenter.x - VIEW_DISTANCE + x, currentCenter.z + z };
                chunksToReplace.push_back(cp);
            }
        }
    }
    else if (deltaX < 0) { // Moving West
        for(int z = -VIEW_DISTANCE; z <= VIEW_DISTANCE; ++z) {
            ChunkPosition cp = { newCenter.x - VIEW_DISTANCE, newCenter.z + z };
            chunksToAdd.push_back(cp);
        }
        // Identify chunks to replace on the East side
        for(int x = 0; x < -deltaX; ++x) {
            for(int z = -VIEW_DISTANCE; z <= VIEW_DISTANCE; ++z) {
                ChunkPosition cp = { currentCenter.x + VIEW_DISTANCE + x, currentCenter.z + z };
                chunksToReplace.push_back(cp);
            }
        }
    }

    // Handle movement along Z axis
    if (deltaZ > 0) { // Moving North
        for(int z = 1; z <= deltaZ; ++z) {
            for(int x = -VIEW_DISTANCE; x <= VIEW_DISTANCE; ++x) {
                ChunkPosition cp = { newCenter.x + x, newCenter.z + VIEW_DISTANCE };
                // Avoid duplicates if we also moved in X
                bool add = true;
                if (deltaX != 0) {
                    for (auto & chunk : chunksToAdd) {
                        if (chunk.x == cp.x && chunk.z == cp.z) {
                            add = false;
                            break;
                        }
                    }
                }
                if (add) chunksToAdd.push_back(cp);
            }
        }
        // Identify chunks to replace on the South side
        for(int z = 0; z < deltaZ; ++z) {
            for(int x = -VIEW_DISTANCE; x <= VIEW_DISTANCE; ++x) {
                ChunkPosition cp = { currentCenter.x + x, currentCenter.z - VIEW_DISTANCE + z };
                bool replace = true;
                if (deltaX != 0) {
                    for (auto & chunk : chunksToReplace) {
                        if (chunk.x == cp.x && chunk.z == cp.z) {
                            replace = false;
                            break;
                        }
                    }
                }
                if (replace) chunksToReplace.push_back(cp);
            }
        }
    }
    else if (deltaZ < 0) { // Moving South
        for(int x = -VIEW_DISTANCE; x <= VIEW_DISTANCE; ++x) {
            ChunkPosition cp = { newCenter.x + x, newCenter.z - VIEW_DISTANCE };
            bool add = true;
            if (deltaX != 0) {
                for (auto & chunk : chunksToAdd) {
                    if (chunk.x == cp.x && chunk.z == cp.z) {
                        add = false;
                        break;
                    }
                }
            }
            if (add) chunksToAdd.push_back(cp);
        }
        // Identify chunks to replace on the North side
        for(int z = 0; z < -deltaZ; ++z) {
            for(int x = -VIEW_DISTANCE; x <= VIEW_DISTANCE; ++x) {
                ChunkPosition cp = { currentCenter.x + x, currentCenter.z + VIEW_DISTANCE + z };
                bool replace = true;
                if (deltaX != 0) {
                    for (auto & chunk : chunksToReplace) {
                        if (chunk.x == cp.x && chunk.z == cp.z) {
                            replace = false;
                            break;
                        }
                    }
                }
                if (replace) chunksToReplace.push_back(cp);
            }
        }
    }

    return true;
}

void TerrainManager::initialize(const glm::vec3& cameraPos) {
    currentCenter = getChunkPosition(cameraPos);

    createTerrainProgramIDs(programID, depthProgramID);

    // Load initial chunks within the view distance
    for(int x = -VIEW_DISTANCE; x <= VIEW_DISTANCE; ++x) {
        for(int z = -VIEW_DISTANCE; z <= VIEW_DISTANCE; ++z) {
            ChunkPosition cp = {currentCenter.x + x, currentCenter.z + z};

            // Create and initialize a Chunk using a unique_ptr
            auto chunk = createChunk(cp, programID, depthProgramID);

            // Emplace the unique_ptr into the chunks vector
            chunks.emplace_back(std::move(chunk));
        }
    }
}

// This method polls all futures in generationFutures. If any are ready, it
// grabs the TerrainData, finds the appropriate Chunk, calls updateBuffers,
// and removes the future from the map.
void TerrainManager::pollTerrainFutures() {
    for (auto it = generationFutures.begin(); it != generationFutures.end(); ) {
        // Non-blocking check if the future is ready
        if (it->second.wait_for(std::chrono::milliseconds(0)) == std::future_status::ready) {
            // Retrieve the data
            TerrainData data = it->second.get();

            // Find the chunk that corresponds to this position
            int idx = findChunkIndex(it->first);
            if (idx != -1) {
                // Update the existing Terrain object's buffers
                chunks[idx]->terrain.updateBuffers(data);
            }
            else {
                std::cerr << "Warning: Chunk position not found for updating buffers.\n";
            }

            // Erase the future from our map
            it = generationFutures.erase(it);
        } else {
            ++it;
        }
    }
}

// Update the TerrainManager based on the camera's new position
void TerrainManager::update(const glm::vec3& cameraPos) {
    ChunkPosition newCenter = getChunkPosition(cameraPos);

    std::vector<ChunkPosition> chunksToAdd;
    std::vector<ChunkPosition> chunksToReplace;

    // Determine which chunks to add and replace based on new center
    if (!chucksToAddReplace(newCenter, chunksToAdd, chunksToReplace)) {
        // If no chunks to add/replace, still poll for any completed futures
        pollTerrainFutures();
        return;
    }

    // Reassign chunk positions and initiate asynchronous terrain generation
    for (size_t i = 0; i < chunksToReplace.size(); i++) {
        const ChunkPosition& oldPos = chunksToReplace[i];
        const ChunkPosition& newPos = chunksToAdd[i];

        int idx = findChunkIndex(oldPos);
        if (idx == -1) {
            std::cerr << "Warning: Chunk to replace not found: (" << oldPos.x << ", " << oldPos.z << ")\n";
            continue; // Skip if the old chunk isn't found
        }

        auto& chunkToUpdate = chunks[idx];

        // Update the position
        chunkToUpdate->position = newPos;

        float posX = newPos.x * CHUNK_SIZE;
        float posZ = newPos.z * CHUNK_SIZE;

        // Kick off async generation for the new chunk position
        std::future<TerrainData> fut = chunkToUpdate->terrain.generateTerrainAsync(
            CHUNK_SIZE, CHUNK_SIZE, MAX_HEIGHT, posX, posZ
        );

        // Store the future so we can poll it later in pollTerrainFutures()
        generationFutures[newPos] = std::move(fut);
    }

    currentCenter = newCenter;

    // Poll to see if any of the new or old generation tasks have completed
    pollTerrainFutures();
}

void TerrainManager::render(const glm::mat4& vp, const glm::mat4& lightSpaceMatrix,
                            const glm::vec3& lightDirection, const glm::vec3& lightIntensity,
                            const glm::vec3& cameraPos)
{
    // First update (which also polls for async completions)
    update(cameraPos);

    // Now draw all chunks
    for(auto& chunkPtr : chunks) {
        chunkPtr->terrain.render(vp, lightSpaceMatrix, lightDirection, lightIntensity);
    }
}

void TerrainManager::cleanup() {
    // Make sure all futures are finished (optional):
    // If you want to ensure absolutely no background threads left:
    for (auto & kv : generationFutures) {
        // Just block until done.
        kv.second.wait();
    }
    generationFutures.clear();

    // Cleanup chunk GPU resources
    for(auto& chunkPtr : chunks) {
        chunkPtr->terrain.cleanup();
    }
    chunks.clear();

    glDeleteProgram(programID);
    glDeleteProgram(depthProgramID);
}
