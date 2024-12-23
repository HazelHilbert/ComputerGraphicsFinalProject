#ifndef TERRAIN_MANAGER_H
#define TERRAIN_MANAGER_H

#include "terrain.h"
#include <unordered_map>
#include <glm/glm.hpp>

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

struct Chunk {
    ChunkPosition position;
    Terrain terrain;
};

class TerrainManager {
public:
    int findChunkIndex(const ChunkPosition &cp) const;

    void initialize(const glm::vec3& cameraPos);

    void update(const glm::vec3& cameraPos);

    void render(const glm::mat4& vp, const glm::mat4& lightSpaceMatrix, const glm::vec3& lightDirection, const glm::vec3& lightIntensity, const glm::vec3& cameraPos);

    void cleanup();

private:
    std::vector<Chunk> chunks;
    ChunkPosition currentCenter;

    ChunkPosition getChunkPosition(const glm::vec3& pos);

    GLuint programID = 0;
    GLuint depthProgramID = 0;
};

#endif
