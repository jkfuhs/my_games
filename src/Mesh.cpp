#include "Mesh.h"

Mesh::Mesh(std::vector<Vertex> vertices, std::vector<int> material_ids)
{
    this->vertices = (vertices);
    this->material_ids.push_back(material_ids[0]);
    
    // sort and place material_ids so no repeats exist
    for (int i = 1; i < material_ids.size(); i++)
    {
        auto index = std::lower_bound(this->material_ids.begin(), this->material_ids.end(), material_ids[i]);
        if (index != this->material_ids.end() && *index != material_ids[i])
        {
            this->material_ids.insert(index, material_ids[i]);
        }
    }
    
}

void Mesh::clearBuffers()
{
    CHECKED_GL_CALL(glDeleteVertexArrays(1, &VAO));
    CHECKED_GL_CALL(glDeleteBuffers(1, &VBO));
}

void Mesh::setupMesh()
{
    CHECKED_GL_CALL(glGenVertexArrays(1, &VAO));
    CHECKED_GL_CALL(glGenBuffers(1, &VBO));

    CHECKED_GL_CALL(glBindVertexArray(VAO));
    // buffer vertex position data
    CHECKED_GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, VBO));
    CHECKED_GL_CALL(glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices[0], GL_STATIC_DRAW));
    
    // vertex positions
    CHECKED_GL_CALL(glEnableVertexAttribArray(0));
    CHECKED_GL_CALL(glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0));
    // vertex normals
    CHECKED_GL_CALL(glEnableVertexAttribArray(1));
    CHECKED_GL_CALL(glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Normal)));
    // texture coords
    CHECKED_GL_CALL(glEnableVertexAttribArray(2));
    CHECKED_GL_CALL(glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, TexCoord)));
    
    CHECKED_GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, 0));
    CHECKED_GL_CALL(glBindVertexArray(0));
}

void Mesh::Draw(Program *shader, std::vector<tinyobj::material_t> materials, std::map<std::string, unsigned int> textures)
{
    unsigned int diffuseNr = 1;
    unsigned int specularNr = 1;
    unsigned int textureNr = 0;

    // loop through mesh's material_ids
    if (material_ids[0] >= 0)
    {
        shader->setFloat("material.shine", materials[material_ids[0]].shininess);
        shader->setVector3f("material.emission", glm::vec3(materials[material_ids[0]].emission[0], materials[material_ids[0]].emission[1], materials[material_ids[0]].emission[2]));

        for (unsigned int i = 0; i < material_ids.size(); i++)
        {
            // activate texture unit before binding
            // retrieve texture number
            std::string number;
            std::string name;

            // bind diffuse texture
            name = materials[i].diffuse_texname;
            if (name.size() > 0)
            {
                CHECKED_GL_CALL(glActiveTexture(GL_TEXTURE0 + textureNr));
                number = std::to_string(diffuseNr);
                shader->setInt(("material.texture_diffuse" + number).c_str(), textureNr);
                CHECKED_GL_CALL(glBindTexture(GL_TEXTURE_2D, textures[name]));
                textureNr++;
                diffuseNr++;
            }

            
            // bind specular texture
            name = materials[i].specular_texname;
            if (name.size() > 0)
            {
                CHECKED_GL_CALL(glActiveTexture(GL_TEXTURE0 + textureNr));
                number = std::to_string(specularNr);
                shader->setInt(("material.texture_specular" + number).c_str(), textureNr);
                CHECKED_GL_CALL(glBindTexture(GL_TEXTURE_2D, textures[name]));
                textureNr++;
                specularNr++;
            }        
        }
    }

    for (unsigned int i = 0; i < texture_ids.size(); i++)
    {
        CHECKED_GL_CALL(glActiveTexture(GL_TEXTURE0 + textureNr));
        shader->setInt(("material.texture_diffuse" + std::to_string(i+1)).c_str(), textureNr);
        CHECKED_GL_CALL(glBindTexture(GL_TEXTURE_2D, texture_ids[i]));
        textureNr++;
    }
    CHECKED_GL_CALL(glActiveTexture(GL_TEXTURE0));

    // draw mesh
    CHECKED_GL_CALL(glBindVertexArray(VAO));
    CHECKED_GL_CALL(glDrawArrays(GL_TRIANGLES, 0, vertices.size()));
    // glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
    CHECKED_GL_CALL(glBindVertexArray(0));
}

void Mesh::center(glm::vec3 model_min, glm::vec3 model_max)
{
    glm::vec3 translate;
    float scale;

    translate = 0.5f * (model_min + model_max);
    scale = max(max(model_max.x - model_min.x, model_max.y - model_min.y), model_max.z - model_min.z);

    for (int i = 0; i < vertices.size(); i++)
    {
        vertices[i].Position -= translate;
        vertices[i].Position *= 2.0f / scale;
    }
}

void Mesh::addTexture(int texture_id)
{
    texture_ids.push_back(texture_id);
}

float min(float x, float y)
{
    return x < y ? x : y;
}
float max(float x, float y)
{
    return x > y ? x : y;
}