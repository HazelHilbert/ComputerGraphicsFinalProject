#ifndef TERRAIN_MANAGER_H
#define TERRAIN_MANAGER_H

#include "terrain.h"
#include <unordered_map>
#include <glm/glm.hpp>
#include <future>
#include <vector>

const int CHUNK_SIZE = 500;
const int MAX_HEIGHT = 30;

const int VIEW_DISTANCE = 2; // 1 for a 3x3 grid

// Structure to uniquely identify each chunk by its grid position
struct ChunkPosition {
    int x;
    int z;

    bool operator==(const ChunkPosition& other) const {
        return x == other.x && z == other.z;
    }
};

// Custom hash ti use ChunkPosition as a key in an unordered_map
struct ChunkPositionHash {
    std::size_t operator()(const ChunkPosition& cp) const {
        auto h1 = std::hash<int>()(cp.x);
        auto h2 = std::hash<int>()(cp.z);
        return (h1 ^ (h2 << 1));
    }
};

struct Chunk {
    ChunkPosition position;
    Terrain terrain;

    // Delete copy and move constructors and assignment operators
    Chunk(const Chunk&) = delete;
    Chunk& operator=(const Chunk&) = delete;
    Chunk(Chunk&&) = default;
    Chunk& operator=(Chunk&&) = default;

    Chunk() = default;
};

class TerrainManager {
public:
    void initialize(const glm::vec3& cameraPos);
    void update(const glm::vec3& cameraPos);
    void render(const glm::mat4& vp, const glm::mat4& lightSpaceMatrix,
                const glm::vec3& lightDirection, const glm::vec3& lightIntensity,
                const glm::vec3& cameraPos);
    void cleanup();

private:
    std::vector<std::unique_ptr<Chunk>> chunks;
    ChunkPosition currentCenter;

    // Stores futures for terrains that are being generated asynchronously.
    // Key: The chunk position for which generation is in progress
    // Value: The future that will yield the TerrainData
    std::unordered_map<ChunkPosition, std::future<TerrainData>, ChunkPositionHash> generationFutures;

    int findChunkIndex(const ChunkPosition &cp) const;
    ChunkPosition getChunkPosition(const glm::vec3& pos);

    bool chucksToAddReplace(ChunkPosition newCenter,
                            std::vector<ChunkPosition>& chunksToAdd,
                            std::vector<ChunkPosition>& chunksToReplace);

    // Poll all known futures to see if they are ready. If so, update buffers
    // on the associated chunk, and remove them from generationFutures.
    void pollTerrainFutures();

    GLuint programID = 0;
    GLuint depthProgramID = 0;
};

#endif
