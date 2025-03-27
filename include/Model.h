#ifndef MODEL_INCLUDE_H
#define MODEL_INCLUDE_H

#include <iostream>
#include <vector>
#include <memory>


#include "Mesh.h"
#include "Program.h"
#include "stb_image.h"
#include "tiny_obj_loader.h"

unsigned int TextureFromFile(const std::string path, const std::string &directory, bool gamma = false);


static std::map<std::string, unsigned int> mk_map()
{
    std::map<std::string, unsigned int> m;
    m.insert(std::pair<std::string, unsigned int>("", 0));
    return m;
}

class Model
{
public:
    // constructor, expects a filepath to a 3D model;
    Model(const std::string path, const std::map<std::string, unsigned int> default_map = mk_map());
    
    // draw the model and all of its meshes
    void Draw(std::shared_ptr<Program> shader);
private:
    // model data

    std::map<std::string, unsigned int> textures_loaded;
    std::vector<tinyobj::material_t> materials;
    std::vector<Mesh> meshes;
    std::string resource_directory;

    glm::vec3 model_min = glm::vec3(std::numeric_limits<float>::max());
    glm::vec3 model_max = glm::vec3(std::numeric_limits<float>::min());

    void loadModel(std::string path);
    Mesh processMesh(tinyobj::shape_t shape, tinyobj::attrib_t attribs, std::vector<tinyobj::material_t> materials);
    void loadMaterialTextures(tinyobj::material_t material);

};

#endif // MODEL_INCLUDE_H