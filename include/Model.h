#ifndef MODEL_INCLUDE_H
#define MODEL_INCLUDE_H

#include <iostream>
#include <vector>
#include <memory>


#include "Mesh.h"
#include "Program.h"
#include "stb_image.h"
#include "tiny_obj_loader.h"


unsigned int TextureFromFile(const std::string &path, const std::string &directory, bool gamma = false);
unsigned int loadCubemap(const std::string &path, const std::vector<std::string> &faces);


class Model
{
public:
    // vector containing positions and orientations of models
    // for example, a model of a tree can be placed in multiple locations
    std::vector<glm::mat4> model_matrices;

    // constructor, expects a filepath to a 3D model;
    Model(const std::string &path);
    
    // draw the model and all of its meshes
    void Draw(std::shared_ptr<Program> shader);

    void addTexture(const std::string &texture_name);

    void clearBuffers();

private:
    // model data

    static std::map<std::string, unsigned int> textures_loaded;
    std::vector<tinyobj::material_t> materials;
    std::vector<Mesh> meshes;
    std::string resource_directory;

    glm::vec3 model_min = glm::vec3(std::numeric_limits<float>::max());
    glm::vec3 model_max = glm::vec3(std::numeric_limits<float>::min());

    void loadModel(const std::string &path);
    Mesh processMesh(tinyobj::shape_t shape, tinyobj::attrib_t attribs, std::vector<tinyobj::material_t> materials);
    void loadMaterialTextures(tinyobj::material_t material);

};

#endif // MODEL_INCLUDE_H