#ifndef UTILS_H
#define UTILS_H
#include "glad/gl.h"

GLuint LoadTextureTileBox(const char *texture_file_path);

void createTerrainProgramIDs(GLuint& inputProgramID, GLuint& inputDepthProgramID);

#endif
