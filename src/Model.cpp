#include "Model.h"

std::map<std::string, unsigned int> Model::textures_loaded;

Model::Model(const std::string &path)
{
    // default model position
    model_matrices.push_back(glm::mat4(1.0f));

    loadModel(path);
}

void Model::clearBuffers()
{
    for (unsigned int i = 0; i < meshes.size(); i++)
    {
        meshes[i].clearBuffers();
    }
}

void Model::Draw(Program *shader)
{
    for (glm::mat4 &m : model_matrices)
    {
        shader->setMat4("model", m);

        for (unsigned int i = 0; i < meshes.size(); i++)
        {
            meshes[i].Draw(shader, materials, textures_loaded);
        }
    }
}

void Model::loadModel(const std::string &path)
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

    // load textures
    for (tinyobj::material_t &m : materials)
    {
        loadMaterialTextures(m);
    }
    // center meshes and bind buffers
    for (Mesh &mesh : meshes)
    {
        mesh.center(model_min, model_max);
        mesh.setupMesh();
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

void Model::addTexture(const std::string &texture_name)
{
    if (textures_loaded.find(texture_name) == textures_loaded.end())
    {
        unsigned int texture_id = TextureFromFile(texture_name, resource_directory);
        textures_loaded[texture_name] = texture_id;
    }
    for (Mesh &mesh : meshes)
    {
        mesh.addTexture(textures_loaded[texture_name]);
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

            // compute model limits
            model_min.x = min(model_min.x, vertex.Position.x);
            model_min.y = min(model_min.y, vertex.Position.y);
            model_min.z = min(model_min.z, vertex.Position.z);

            model_max.x = max(model_max.x, vertex.Position.x);
            model_max.y = max(model_max.y, vertex.Position.y);
            model_max.z = max(model_max.z, vertex.Position.z);

            vertices.push_back(vertex);            
        }            
        index_offset += fv;
    }
    
    return Mesh(vertices, shape.mesh.material_ids);
}

unsigned int loadCubemap(const std::string &path, const std::vector<std::string> &faces)
{
    unsigned int textureID;
    CHECKED_GL_CALL(glGenTextures(1, &textureID));
    CHECKED_GL_CALL(glBindTexture(GL_TEXTURE_CUBE_MAP, textureID));

    int width, height, nrChannels;
    for (unsigned int i = 0; i < faces.size(); i++)
    {
        std::string filename = path + '/' + faces[i];
        unsigned char *data = stbi_load(filename.c_str(), &width, &height, &nrChannels, 0);
        if (data)
        {
            CHECKED_GL_CALL(glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
                                        0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data
            ));
        }
        else
        {
            std::cout << "Cubemap tex failed to load at path: " << filename << std::endl;
        }
        stbi_image_free(data);
    }
    CHECKED_GL_CALL(glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
    CHECKED_GL_CALL(glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
    CHECKED_GL_CALL(glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
    CHECKED_GL_CALL(glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));
    CHECKED_GL_CALL(glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE));

    return textureID;
}

unsigned int TextureFromFile(const std::string &path, const std::string &directory, bool gamma)
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

        CHECKED_GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, format == GL_RGBA ? GL_CLAMP_TO_EDGE : GL_REPEAT));
        CHECKED_GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, format == GL_RGBA ? GL_CLAMP_TO_EDGE : GL_REPEAT));
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
