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
    setupMesh();
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

void Mesh::Draw(std::shared_ptr<Program> shader, std::vector<tinyobj::material_t> materials, std::map<std::string, unsigned int> textures)
{
    unsigned int diffuseNr = 1;
    unsigned int specularNr = 1;
    unsigned int textureNr = 0;

    // loop through mesh's material_ids
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
    CHECKED_GL_CALL(glActiveTexture(GL_TEXTURE0));

    // draw mesh
    CHECKED_GL_CALL(glBindVertexArray(VAO));
    CHECKED_GL_CALL(glDrawArrays(GL_TRIANGLES, 0, vertices.size()));
    // glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
    CHECKED_GL_CALL(glBindVertexArray(0));
}