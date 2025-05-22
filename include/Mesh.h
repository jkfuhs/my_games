#pragma once
#ifndef SHAPES_INCLUDE_H
#define SHAPES_INCLUDE_H

#include "Mesh.fwd.h"
#include <string>
#include <memory>

#include "Program.h"
#include "D:/my_games/lib/glm/gtc/matrix_transform.hpp"
#include "tiny_obj_loader.h"
#include <GLSL.h>


struct Vertex
{
    glm::vec3 Position;
    glm::vec3 Normal;
    glm::vec2 TexCoord;
};

float min(float x, float y);
float max(float x, float y);

class Mesh 
{
public:
    

    Mesh(std::vector<Vertex> vertices, std::vector<int> material_ids);
    void center(glm::vec3 min, glm::vec3 max);
    void Draw(Program *shader, std::vector<tinyobj::material_t> materials, std::map<std::string, unsigned int> textures);
    void addTexture(int texture_index);
    void setupMesh();
    void clearBuffers();
private:
    // render data
    unsigned int VAO       = 0, 
                 VBO       = 0; 
    // mesh data
    std::vector<Vertex>   vertices;
    std::vector<int>  material_ids;
    std::vector<int>  texture_ids;

};

#endif // SHAPES_INCLUDE_H