#include "Model.h"



Model::Model(const std::string path, const std::map<std::string, unsigned int> default_map): textures_loaded(default_map)
{
    loadModel(path);
}

void Model::Draw(std::shared_ptr<Program> shader)
{
    for (unsigned int i = 0; i < meshes.size(); i++)
    {
        meshes[i].Draw(shader, materials, textures_loaded);
    }
}

void Model::loadModel(std::string path)
{
    tinyobj::ObjReader reader;
    tinyobj::ObjReaderConfig reader_config;

    if (!reader.ParseFromFile(path, reader_config))
    {
        if (!reader.Error().empty())
        {
            std::cerr << "TinyObjReader: " << reader.Error();
        }
        exit(1);
    }

    if (!reader.Warning().empty())
    {
        std::cout << "TinyObjReader: " << reader.Warning();
    }
    resource_directory = path.substr(0, path.find_last_of('/'));

    auto& attribs = reader.GetAttrib();
    auto& shapes = reader.GetShapes();
    auto& mats = reader.GetMaterials();
    
    materials.assign(mats.begin(), mats.end());

    // append default material
    // materials.push_back(tinyobj::material_t());

    // loop over shapes
    for (size_t s = 0; s < shapes.size(); s++)
    {
        Mesh mesh = processMesh(shapes[s], attribs, materials);
        meshes.push_back(mesh);
    }

    // loop over materials
    for (size_t m = 0; m < materials.size(); m++)
    {
        loadMaterialTextures(materials[m]);
    }
}

void Model::loadMaterialTextures(tinyobj::material_t material)
{
    unsigned int texture_id;
    std::string path;
        // process diffuse texture
        if (material.diffuse_texname.length() > 0)
        {
            // only load textures if they aren't already loaded
            if (textures_loaded.find(material.diffuse_texname) == textures_loaded.end())
            {
                path = material.diffuse_texname;
                texture_id = TextureFromFile(path, resource_directory);
                textures_loaded[path] = texture_id;
            }
        }

        // process specular texture
        if (material.specular_texname.length() > 0)
        {
            if (textures_loaded.find(material.specular_texname) == textures_loaded.end())
            {
                path = material.specular_texname;
                texture_id = TextureFromFile(path, resource_directory);
                textures_loaded[path] = texture_id;
            }
        }
}

Mesh Model::processMesh(tinyobj::shape_t shape, tinyobj::attrib_t attribs, std::vector<tinyobj::material_t> materials)
{
    std::vector<Vertex> vertices;
    // loop over faces
    size_t index_offset = 0;
    for (size_t f = 0; f < shape.mesh.num_face_vertices.size(); f++)
    {
        size_t fv = size_t(shape.mesh.num_face_vertices[f]);

        // loop over vertices in each face
        for (size_t v = 0; v < fv; v++)
        {
            Vertex vertex;
            tinyobj::index_t idx = shape.mesh.indices[index_offset + v];
            vertex.Position.x = attribs.vertices[3 * size_t(idx.vertex_index)+0];
            vertex.Position.y = attribs.vertices[3 * size_t(idx.vertex_index)+1];
            vertex.Position.z = attribs.vertices[3 * size_t(idx.vertex_index)+2];

            // check if normals exist
            if (idx.normal_index >= 0)
            {
                vertex.Normal.x = attribs.normals[3 * size_t(idx.normal_index) + 0];
                vertex.Normal.y = attribs.normals[3 * size_t(idx.normal_index) + 1];
                vertex.Normal.z = attribs.normals[3 * size_t(idx.normal_index) + 2];
            }

            // check if texcoords exist
            if (idx.texcoord_index >= 0)
            {
                vertex.TexCoord.x = attribs.texcoords[2 * size_t(idx.texcoord_index) + 0];
                vertex.TexCoord.y = attribs.texcoords[2 * size_t(idx.texcoord_index) + 1];
            }

            vertices.push_back(vertex);            
        }            
        index_offset += fv;
    }
    
    return Mesh(vertices, shape.mesh.material_ids);
    
}

unsigned int TextureFromFile(const std::string path, const std::string &directory, bool gamma)
{
    std::string filename = std::string(path);
    filename = directory + '/' + filename;

    unsigned int textureID;
    CHECKED_GL_CALL(glGenTextures(1, &textureID));

    int width, height, nrComponents;
    unsigned char *data = stbi_load(filename.c_str(), &width, &height, &nrComponents, 0);
    if (data)
    {
        GLenum format;
        switch(nrComponents)
        {
            case 1:
                format = GL_RED;
                break;
            case 3:
                format = GL_RGB;
                break;
            case 4:
                format = GL_RGBA;
                break;
        }

        CHECKED_GL_CALL(glBindTexture(GL_TEXTURE_2D, textureID));
        CHECKED_GL_CALL(glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data));
        CHECKED_GL_CALL(glGenerateMipmap(GL_TEXTURE_2D));

        CHECKED_GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT));
        CHECKED_GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT));
        CHECKED_GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR));
        CHECKED_GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));

        stbi_image_free(data);
    }
    else
    {
        std::cout << "Texture failed to load at path: " << path << std::endl;
        stbi_image_free(data);
    }

    return textureID;
}
