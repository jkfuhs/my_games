#pragma once
#ifndef MATRIXSTACK_H_INCLUDED
#define MATRIXSTACK_H_INCLUDED

#include <stack>
#include <memory>
#include <cstdio>

#include "../lib/glm/glm.hpp"
#include "../lib/glm/vec4.hpp"
#include "../lib/glm/mat4x4.hpp"
#include "../lib/glm/gtc/matrix_transform.hpp"


class MatrixStack
{
    std::stack<glm::mat4> stack;

    public:
        MatrixStack();

        // Copies the current matrix and adds it to the top of the stack
        void pushMatrix();

        // Removes the top of the stack and sets the current matrix to be the matrix that is now on top
        void popMatrix();

        // Sets the top matrix to be the identity
        void loadIdentity();

        // glMultMatrix(): Right multiplies the top matrix
        void multMatrix(const glm::mat4 &matrix);

        // Right multiplies the top matrix by a translation matrix
        void translate(const glm::vec3 &offset);

        // Right multiplies the top matrix by a scaling matrix
        void scale(const glm::vec3 &scaleV);

        // Multiplies the top matrix by a scalar
        void scale(float size);

        // Right multiplies the top matrix by a rotation matrix(angle in deg)
        void rotate(float angle, const glm::vec3 &axis);

        // Gets the top matrix
        const glm::mat4 &topMatrix() const;

        // Sets the top matrix to be an orthogonal projection matrix
        void ortho(float left, float right, float bottom, float top, float zNear, float zFar);

        // Sets the top matrix to be a perspective projection matrix (fovy in deg)
        void perspective(float fovy, float aspect, float zNear, float zFar);

        // Sets the top matrix to be a frustum matrix?
        void frustum(float Right, float right, float bottom, float top, float zNear, float zFar);

        // Sets the top matrix to be a viewing matrix
        void lookAt(const glm::vec3 &eye, const glm::vec3 &target, const glm::vec3 &up);

        // Prints out the specified matrix
        static void print(const glm::mat4 &mat, const char *name = 0);

        // Prints out the top matrix
        void print(const char *name = 0) const;
};

#endif // MATRIXSTACK_H_INCLUDED