#ifndef APPLICATION_H
#define APPLICATION_H

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <iostream>
#include <memory>

#include "Model.h"
#include "WindowManager.h"
#include "Program.h"
#include "GLSL.h"

#include "MatrixStack.h" 
#include "Camera.h"

// value_ptr for glm
const std::string PROJECT_NAME = "my_game";
#include "D:/my_games/lib/glm/gtc/type_ptr.hpp"
#include "D:/my_games/lib/glm/gtc/matrix_transform.hpp"

// settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;
// const unsigned int SCR_WIDTH = 1920;
// const unsigned int SCR_HEIGHT = 1080;



struct DirLight
{
    glm::vec3 direction;
    glm::vec3 ambient;
    glm::vec3 diffuse;
    glm::vec3 specular;
};

struct PointLight
{
    glm::vec3 position;
    glm::vec3 ambient;
    glm::vec3 diffuse;
    glm::vec3 specular;
    glm::vec3 attenuation;
};

struct SpotLight
{
    glm::vec3 position;
    glm::vec3 direction;
    glm::vec3 ambient;
    glm::vec3 diffuse;
    glm::vec3 specular;
    glm::vec3 attenuation;
    float innerCone;
    float outerCone;
    bool isOn;
};

class Application : public EventCallbacks
{
    private:
        WindowManager * windowManager = nullptr;

        std::map<std::string, Program> shaders;

        const std::string &resourceDir;
        const std::string &shaderDir;

        Camera camera;

        // frame time values
        float deltaTime = 0.0f;
        float lastFrame = 0.0f;

        // cursor position values (initialize to center of screen)
        float lastX = SCR_WIDTH / 2.0f;
        float lastY = SCR_HEIGHT / 2.0f;
        bool firstMouse = true;

        unsigned int planeVAO, planeVBO;
        unsigned int quadVAO, quadVBO;
        unsigned int skyBoxVAO, skyBoxVBO;
        unsigned int fbo, rbo;  // frame buffer and render buffer objects

        // textures
        std::vector<glm::vec3> textures;
        unsigned int planeTexture;
        unsigned int skyBoxTex;
        unsigned int frame_texture;

        std::map<int, std::function<void(int)>> keybinds;

        const unsigned int skyboxTexture = 11;

        // light source positions
        DirLight directionalLight = 
        {
            glm::vec3(-0.2f, -1.0f, -0.3f),
            glm::vec3(0.2f, 0.2f, 0.2f),
            glm::vec3(0.5f, 0.5f, 0.5f),
            glm::vec3(1.0f, 1.0f, 1.0f)
        };

        std::vector<PointLight> pointLights;
        std::vector<SpotLight> spotLights;


        void cursorCallback(GLFWwindow *window, double xposIn, double yposIn);
        void keyCallback(GLFWwindow *window, int key, int scancode, int action, int mods);
        void mouseCallback(GLFWwindow *window, int button, int action, int mods);
        void scrollCallback(GLFWwindow *window, double in_deltaX, double in_deltaY);
        void resizeCallback(GLFWwindow *window, int width, int height);
        void framebuffer_size_callback(GLFWwindow* window, int width, int height);

        void initializeShader(const std::string &shaderName, bool verbose, const std::string &vertexShader, const std::string &fragmentShader, const std::vector<std::string> &attributes);
        void initGeom();
        void init();
        
        void initSky();
        void initGround();
        void updateVars();
        void render();
        void drawSky(glm::mat4 view, glm::mat4 projection);
        void drawGround(std::shared_ptr<Program> &curS);
        void drawScene(glm::mat4 view, glm::mat4 projection);
        void setLightUniforms(Program &prog);

    public:
        Application(const std::string &shaderDirectory, const std::string &resourceDirectory);
        void run(std::function<void()> init, std::function<void()> loop);
        void shutdown();
        void setKeyBind(int key, std::function<void(int)> func);
        void setKeyBindSet(Camera_Type type);
        void setCameraType(Camera_Type type);
        Model *addModel(const std::string &modelPath, const std::string &shaderName = "default");
};

#endif //APPLICATION_H