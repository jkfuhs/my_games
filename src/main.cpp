#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <iostream>
#include <memory>

#include "WindowManager.h"
#include "Program.h"
#include "GLSL.h"
// #include "filesystem.h"
// #include "Shape.h"
#include "MatrixStack.h" 
#include "Camera.h"
#include "Model.h"

#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

// value_ptr for glm
#include "D:/my_games/lib/glm/gtc/type_ptr.hpp"
#include "D:/my_games/lib/glm/gtc/matrix_transform.hpp"

#define PROJECT_NAME "my_game"
#define RESOURCE_DIR "D:/my_games/resources"
#define SHADER_DIR "D:/my_games/shaders"
#define NR_POINT_LIGHTS 4
void framebuffer_size_callback(GLFWwindow* window, int width, int height);

#define REMEMBER_FOR_CHLOE 4780

// settings
// const unsigned int SCR_WIDTH = 800;
// const unsigned int SCR_HEIGHT = 600;
const unsigned int SCR_WIDTH = 1920;
const unsigned int SCR_HEIGHT = 1080;

class Application : public EventCallbacks
{
    public:
        WindowManager * windowManager = nullptr;
        
        // default shader program
        std::shared_ptr<Program> prog;
        // texture shader program
        std::shared_ptr<Program> lightProg;
        // mix ratio uniform
        float mixRatio = 0.2;

        Camera camera;

        float deltaTime = 0.0f;
        float lastFrame = 0.0f;
        Model *ourModel;

        float lastX = SCR_WIDTH / 2.0f;
        float lastY = SCR_HEIGHT / 2.0f;
        bool firstMouse = true;
        glm::vec3 directionalLight = glm::vec3(-0.2f, -1.0f, -0.3f);
        glm::vec3 lightColor = glm::vec3(1.0f, 1.0f, 1.0f);

        void cursorCallback(GLFWwindow *window, double xposIn, double yposIn)
        {
            float xpos = static_cast<float>(xposIn);
            float ypos = static_cast<float>(yposIn);

            float xoffset = 0;
            float yoffset = 0;
            if (firstMouse)
            {
                firstMouse = false;
            }
            else 
            {
                xoffset = xpos - lastX;
                yoffset = lastY - ypos;
            }
            lastX = xpos;
            lastY = ypos;

            const float sensitivity = 0.1f;
            xoffset *= sensitivity;
            yoffset *= sensitivity;
            
            camera.rotateCamera(xoffset, yoffset);
        }
        void keyCallback(GLFWwindow *window, int key, int scancode, int action, int mods)
        {
            if (action == GLFW_PRESS)
            {
                switch(key)
                {
                    case GLFW_KEY_ESCAPE:
                        glfwSetWindowShouldClose(window, GL_TRUE);
                        break;
                    case GLFW_KEY_UP:
                        mixRatio += 0.1;
                        mixRatio = (mixRatio > 1.0 ? 1.0 : mixRatio);
                        break;
                    case GLFW_KEY_DOWN:
                        mixRatio -= 0.1;
                        mixRatio = (mixRatio < 0.0 ? 0.0 : mixRatio);
                        break;
                    case GLFW_KEY_W:
                        camera.addMotion(glm::vec3(0.0f, 0.0f, -1.0f));
                    break;
                    case GLFW_KEY_S:
                        camera.addMotion(glm::vec3(0.0f, 0.0f, 1.0f));
                    break;
                    case GLFW_KEY_A:
                        camera.addMotion(glm::vec3(-1.0f, 0.0f, 0.0f));
                    break;
                    case GLFW_KEY_D:
                        camera.addMotion(glm::vec3(1.0f, 0.0f, 0.0f));
                    break;
                    case GLFW_KEY_SPACE:
                        camera.addMotion(glm::vec3(0.0f, 1.0f, 0.0f));
                    break;
                    case GLFW_KEY_V:
                        camera.addMotion(glm::vec3(0.0f, -1.0f, 0.0f));
                    break;
                }
            }
            if (action == GLFW_RELEASE)
            {
                switch(key)
                {
                    case GLFW_KEY_W:
                        camera.addMotion(glm::vec3(0.0f, 0.0f, 1.0f));
                        break;
                    case GLFW_KEY_S:
                        camera.addMotion(glm::vec3(0.0f, 0.0f, -1.0f));
                    break;
                    case GLFW_KEY_A:
                        camera.addMotion(glm::vec3(1.0f, 0.0f, 0.0f));
                    break;
                    case GLFW_KEY_D:
                        camera.addMotion(glm::vec3(-1.0f, 0.0f, 0.0f));
                    break;
                    case GLFW_KEY_SPACE:
                        camera.addMotion(glm::vec3(0.0f, -1.0f, 0.0f));
                    break;
                    case GLFW_KEY_V:
                        camera.addMotion(glm::vec3(0.0f, 1.0f, 0.0f));
                    break;
                }
            }
        }

        void mouseCallback(GLFWwindow *window, int button, int action, int mods)
        {
            // do nothing for now
        }

        void scrollCallback(GLFWwindow *window, double in_deltaX, double in_deltaY)
        {
            camera.adjustZoom(in_deltaY);
        }

        void resizeCallback(GLFWwindow *window, int width, int height)
        {
            CHECKED_GL_CALL(glViewport(0, 0, width, height));
        }

        void init(const std::string &shaderDirectory, const std::string &resourceDirectory)
        {
            GLSL::checkVersion();

            // enable z-buffer
            CHECKED_GL_CALL(glEnable(GL_DEPTH_TEST));

            // intialize and set starting camera location
            camera = Camera(glm::vec3(0.0f, 0.0f, 3.0f));

            // Initialize the GLSL program that we will use for local shading
            prog = std::make_shared<Program>();
            prog->setVerbose(true);
            prog->setShaderNames(shaderDirectory + "/simpleVertex.vs", shaderDirectory + "/simpleFragment.fs");
            prog->init();
            prog->addAttribute("aPos");
            prog->addAttribute("aNormal");
            prog->addAttribute("aTexCoords");
            // tranform matrices
            prog->addUniform("model");
            prog->addUniform("view");
            prog->addUniform("projection");

            // Initialize shader for light sources
            lightProg = std::make_shared<Program>();
            lightProg->setVerbose(true);
            lightProg->setShaderNames(shaderDirectory + "/lightVertex.vs", shaderDirectory + "/lightFragment.fs");
            lightProg->init();
            lightProg->addAttribute("aPos");
            lightProg->addUniform("model");
            lightProg->addUniform("view");
            lightProg->addUniform("projection");
            
        }

        void initGeom(const std::string&resourceDirectory)
        {
            ourModel = new Model((resourceDirectory + "/backpack/backpack.obj").c_str());

        }

        void initGround()
        {
            // TODO:initGround
        }

        void drawGround(std::shared_ptr<Program> curS)
        {
            // TODO:drawGround
        }

        // helper function to pass material data to GPU
        void SetMaterial(std::shared_ptr<Program> curS, int mat)
        {
            // TODO:SetMaterrial
        }

        // helper function to set model transforms
        void setModelTransforms(glm::vec3 trans, float rotY, float rotX, float scale, std::shared_ptr<Program> curS)
        {
            glm::mat4 Trans = glm::translate(glm::mat4(1.0f), trans);
            glm::mat4 RotX = glm::rotate(glm::mat4(1.0f), rotX, glm::vec3(1, 0, 0));
            glm::mat4 RotY = glm::rotate(glm::mat4(1.0f), rotY, glm::vec3(0, 1, 0));
            glm::mat4 ScaleS = glm::scale(glm::mat4(1.0f), glm::vec3(scale));
            glm::mat4 ctm = Trans * RotX * RotY * ScaleS;
            CHECKED_GL_CALL(glUniformMatrix4fv(curS->getUniform("M"), 1, GL_FALSE, value_ptr(ctm)));
        }

        void setMat4(std::shared_ptr<Program> prog, std::shared_ptr<MatrixStack>M) 
        {
            CHECKED_GL_CALL(glUniformMatrix4fv(prog->getUniform("M"), 1, GL_FALSE, value_ptr(M->topMatrix())));
        }
        void updateVars()
        {
            float currentFrame = glfwGetTime();
            deltaTime = currentFrame - lastFrame;
            lastFrame = currentFrame;

            camera.move(deltaTime);
        }
        void render()
        {
            CHECKED_GL_CALL(glClearColor(0.05f, 0.05f, 0.05f, 1.0f));
            CHECKED_GL_CALL(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));

            // view/projection transformations
            glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
            glm::mat4 view = camera.GetViewMatrix();

            // render basic cube objects
            prog->bind();
            prog->setFloat(("material.shine"), 32.0f);

            prog->setVector3f("spotLight.position", camera.Position);
            prog->setVector3f("spotLight.direction", camera.Front);
            prog->setVector3f("spotLight.ambient", glm::vec3(0.05f));
            prog->setVector3f("spotLight.diffuse", glm::vec3(0.5f));
            prog->setVector3f("spotLight.specular", glm::vec3(1.0f));
            prog->setVector3f("spotLight.attenuation", glm::vec3(1.0f, 0.09f, 0.032f));
            prog->setFloat("spotLight.centerCone", glm::cos(glm::radians(12.5f)));
            prog->setFloat("spotLight.outerCone", glm::cos(glm::radians(17.5f)));

            
            // set view/projection transforms
            CHECKED_GL_CALL(glUniformMatrix4fv(prog->getUniform("projection"), 1, GL_FALSE, glm::value_ptr(projection)));
            CHECKED_GL_CALL(glUniformMatrix4fv(prog->getUniform("view"), 1, GL_FALSE, glm::value_ptr(view)));
          
            // render loaded model
            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f));
            model = glm::scale(model, glm::vec3(1.0f, 1.0f, 1.0f));
            CHECKED_GL_CALL(glUniformMatrix4fv(prog->getUniform("model"), 1, GL_FALSE, glm::value_ptr(model)));
            (*ourModel).Draw(prog);
            prog->unbind();

        }
};


int main()
{

    std::string resourceDir = RESOURCE_DIR;
    std::string shaderDir = SHADER_DIR;
    Application *application = new Application();
    WindowManager *windowManager = new WindowManager();
    windowManager->init(SCR_WIDTH, SCR_HEIGHT, PROJECT_NAME);
    windowManager->setEventCallbacks(application);
    application->windowManager = windowManager;
    
    application->init(shaderDir, resourceDir);
    application->initGeom(resourceDir);
    
    while (!glfwWindowShouldClose(windowManager->getHandle()))
    {
        application->updateVars();
        application->render();
        glfwSwapBuffers(windowManager->getHandle());
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    CHECKED_GL_CALL(glViewport(0, 0, width, height));
}
