#pragma once

#ifndef SHADER_PROGRAM_H_INCLUDED
#define SHADER_PROGRAM_H_INCLUDED

#include <map>
#include <string>

#include <glad/glad.h>

#include "D:/my_games/lib/glm/gtc/type_ptr.hpp"

std::string readFileAsString(const std::string &fileName);

class Program
{
    protected:
        std::string vShaderName;
        std::string fShaderName;

    private:
        GLuint pid = 0;
        std::map<std::string, GLint> attributes;
        std::map<std::string, GLint> uniforms;
        bool verbose = true;

    public:
        void setVerbose(const bool v) {verbose = v;}
        bool isVerbose() const { return verbose;}
        
        void setShaderNames(const std::string &v, const std:: string &f);
        virtual bool init();
        virtual void bind();
        virtual void unbind();

        void addAttribute(const std::string &name);
        void addUniform(const std::string &name);
        void setBool(const std::string &name, bool b);
        void setInt(const std::string &name, int i);
        void setFloat(const std::string &name, float f);
        void setVector3f(const std::string &name, glm::vec3 v);
        void setMat4(const std::string &name, glm::mat4 m);
        GLint getAttribute(const std::string &name) const;
        GLint getUniform(const std::string &name) const;
};

#endif //SHADER_PROGRAM_H_INCLUDED