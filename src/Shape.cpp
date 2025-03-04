#include "Shape.h"

#include <iostream>
#include <cassert>

#include "GLSL.h"
#include "Program.h"

#define MIN(x, y) ((x) < (y) ? (x) : (y))
#define MAX(x, y) ((x) > (y) ? (x) : (y))

// copy data from shape to this object
void Shape::createShape(tinyobj::shape_t & shape)
{
    posBuf = shape.mesh.positions;
    norBuf = shape.mesh.normals;
    texBuf = shape.mesh.texcoords;
    eleBuf = shape.mesh.indices;
}

void Shape::measure()
{
    float minX, minY, minZ;
    float maxX, maxY, maxZ;

    minX = minY = minZ = std::numeric_limits<float>::max();
    maxX = maxY = maxZ = std::numeric_limits<float>::min();

    // Go through all vectices to determine min and max of each dimension
    for (size_t v = 0; v < posBuf.size(); v++)
    {
        minX = MIN(posBuf[X(v)], minX);
        maxX = MAX(posBuf[X(v)], maxX);
        minY = MIN(posBuf[Y(v)], minY);
        maxY = MAX(posBuf[Y(v)], maxY);
        minZ = MIN(posBuf[Z(v)], minZ);
        maxZ = MAX(posBuf[Z(v)], maxZ);
    }
    min.x = minX;
    min.y = minY;
    min.z = minZ;
    max.x = maxX;
    max.y = maxY;
    max.z = maxZ;
}

void Shape::init()
{
    // Initialize Vertex Array Object
    CHECKED_GL_CALL(glGenVertexArrays(1, &vaoID));
    CHECKED_GL_CALL(glBindVertexArray(vaoID));

    // Send the position array to GPU
    CHECKED_GL_CALL(glGenBuffers(1, &posBufID));
    CHECKED_GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, posBufID));
    CHECKED_GL_CALL(glBufferData(GL_ARRAY_BUFFER, posBuf.size()*sizeof(float), &posBuf[0], GL_STATIC_DRAW));

    // Send the normal array to the GPU
    if (norBuf.empty())
    {
        size_t n = posBuf.size();
        norBuf.reserve(n);

        for (int i=0; i<n; i++)
        {
            norBuf.push_back(0.0);
        }

        for (int i=0; i+2<eleBuf.size(); i+=3)
        {
            glm::vec3 v1 = glm::vec3(posBuf[3*eleBuf[i]], posBuf[3*eleBuf[i]+1], posBuf[3*eleBuf[i]+2]);
            glm::vec3 v2 = glm::vec3(posBuf[3*eleBuf[i+1]], posBuf[3*eleBuf[i+1]+1], posBuf[3*eleBuf[i+1]+2]);
            glm::vec3 v3 = glm::vec3(posBuf[3*eleBuf[i+2]], posBuf[3*eleBuf[i+2]+1], posBuf[3*eleBuf[i+2]+2]);

            glm::vec3 e1 = v2 - v1;
            glm::vec3 e2 = v3 - v1;
            glm::vec3 norm = glm::cross(e1, e2);

            for (int j=0; j<3; j++)
            {
                for (int k=0; k<3; k++)
                {
                    norBuf[3*eleBuf[i+j]+k] += norm[k];
                }
            }
        }

        for (int i=0; i+2<norBuf.size(); i+=3)
        {
            glm::vec3 norm = glm::vec3(norBuf[i], norBuf[i+1], norBuf[i+2]);
            norm = normalize(norm);
            for (int j=0; j<3; j++)
            {
                norBuf[i+j] = norm[j];
            }
        }
    }

    CHECKED_GL_CALL(glGenBuffers(1, &norBufID));
    CHECKED_GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, norBufID));
    CHECKED_GL_CALL(glBufferData(GL_ARRAY_BUFFER, norBuf.size() * sizeof(float), &norBuf[0], GL_STATIC_DRAW));

    // send the texture array to the GPU
    if (texBuf.empty())
    {
        texBufID = 0;
    }
    else
    {
        CHECKED_GL_CALL(glGenBuffers(1, &texBufID));
        CHECKED_GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, texBufID));
        CHECKED_GL_CALL(glBufferData(GL_ARRAY_BUFFER, texBuf.size()*sizeof(float), &texBuf[0], GL_STATIC_DRAW));
    }

    // Send the element array to the GPU
    CHECKED_GL_CALL(glGenBuffers(1, &eleBufID));
    CHECKED_GL_CALL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, eleBufID));
    CHECKED_GL_CALL(glBufferData(GL_ELEMENT_ARRAY_BUFFER, eleBuf.size()*sizeof(unsigned int), &eleBuf[0], GL_STATIC_DRAW));

    // Unbind the arrays
    CHECKED_GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, 0));
    CHECKED_GL_CALL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0));
}

void Shape::draw(const std::shared_ptr<Program> prog) const
{
    int h_pos, h_nor, h_tex;
    h_pos = h_nor = h_tex = -1;

    CHECKED_GL_CALL(glBindVertexArray(vaoID));

    // Bind position buffer
    h_pos = prog->getAttribute("vertPos");
    GLSL::enableVertexAttribArray(h_pos);
    CHECKED_GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, posBufID));
    CHECKED_GL_CALL(glVertexAttribPointer(h_pos, 3, GL_FLOAT, GL_FALSE, 0, (const void *)0));

    // Bind normal buffer
    h_nor = prog->getAttribute("vertNor");
    if (h_nor != -1 && norBufID != 0)
    {
        GLSL::enableVertexAttribArray(h_nor);
        CHECKED_GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, norBufID));
        CHECKED_GL_CALL(glVertexAttribPointer(h_nor, 3, GL_FLOAT, GL_FALSE, 0, (const void *)0));
    }

    // Bind texture coordinate buffer
    if (texBufID != 0)
    {
        h_tex = prog->getAttribute("vertTex");
        if (h_tex != -1 && texBufID != 0)
        {
            GLSL::enableVertexAttribArray(h_tex);
            CHECKED_GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, texBufID));
            CHECKED_GL_CALL(glVertexAttribPointer(h_tex, 2, GL_FLOAT, GL_FALSE, 0, (const void *)0));
        }
    }

    // Bind element buffer
    CHECKED_GL_CALL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, eleBufID));

    // Draw
    CHECKED_GL_CALL(glDrawElements(GL_TRIANGLES, (int)eleBuf.size(), GL_UNSIGNED_INT, (const void *)0));

    // Disable and unbind
    if (h_tex != -1)
    {
        GLSL::disableVertexAttribArray(h_tex);
    }
    if (h_nor != -1)
    {
        GLSL::disableVertexAttribArray(h_nor);
    }
    GLSL::disableVertexAttribArray(h_pos);
    CHECKED_GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, 0));
    CHECKED_GL_CALL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0));
}