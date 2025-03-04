#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <memory>

#include "WindowManager.h"
#include "Program.h"
#include "GLSL.h"
#include "Shape.h"
#include "MatrixStack.h"

#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader/tiny_obj_loader.h>

// value_ptr for glm
#include "../lib/glm/gtc/type_ptr.hpp"
#include "../lib/glm/gtc/matrix_transform.hpp"

#define RESOURCE_DIR "../resources/"
#define SHADER_DIR "../shaders/"
void framebuffer_size_callback(GLFWwindow* window, int width, int height);

// settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

class Application : public EventCallbacks
{
    public:
        WindowManager * windowManager = nullptr;
        
        // default shader program
        std::shared_ptr<Program> prog;
        // texture shader program
        std::shared_ptr<Program> texProg;

        void keyCallback(GLFWwindow *window, int key, int scancode, int action, int mods)
        {
            if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
            {
                glfwSetWindowShouldClose(window, GL_TRUE);
            }
        }

        void mouseCallback(GLFWwindow *window, int button, int action, int mods)
        {
            // do nothing for now
        }

        void scrollCallback(GLFWwindow * window, double in_deltaX, double in_deltaY)
        {
            // do nothing for now
        }

        void resizeCallback(GLFWwindow *window, int width, int height)
        {
            glViewport(0, 0, width, height);
        }

        void init(const std::string &shaderDirectory, const std::string &resourceDirectory)
        {
            GLSL::checkVersion();

            // Set background color
            glClearColor(0.2f, 0.3f, 0.3f, 1.0f);

            // Initialize the GLSL program that we will use for local shading
            prog = std::make_shared<Program>();
            prog->setVerbose(true);
            prog->setShaderNames(shaderDirectory + "/simpleVertex.glsl", shaderDirectory + "/simple_frag.glsl");
            prog->init();
            prog->addAttribute("vertPos");
        }

        void loadMultiShape(std::vector<tinyobj::shape_t> TOshapes, std::vector<std::shared_ptr<Shape>> *mesh, glm::vec3 *meshMin, glm::vec3 *meshMax)
        {
            for (int i=0; i < TOshapes.size(); i++)
            {
                (*mesh).push_back(std::make_shared<Shape>());
                (*mesh)[i]->createShape(TOshapes[i]);
                (*mesh)[i]->measure();
                (*mesh)[i]->init();
            }

            // calculate bounding box of mult-shape
            *meshMin = (*mesh)[0]->min;
            *meshMax = (*mesh)[0]->max;
            for (int i=1; i<(*mesh).size(); i++)
            {
                for (int j=0; j<2; j++)
                {
                    if ((*mesh)[i]->min[j] < (*meshMin)[j])
                    {
                        (*meshMin)[j] = (*mesh)[i]->min[j];
                    }
                    if ((*mesh)[i]->max[j] > (*meshMax)[j])
                    {
                        (*meshMax)[j] = (*mesh)[i]->max[j];
                    }
                }
            }
        }

        void initGeom(const std::string&resourceDirectory)
        {
            // read shapes from object files
            // 1. initialize mesh
            // 2. load geometry
            // 3. load material data
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
            glUniformMatrix4fv(curS->getUniform("M"), 1, GL_FALSE, value_ptr(ctm));
        }

        void setModel(std::shared_ptr<Program> prog, std::shared_ptr<MatrixStack>M) 
        {
            glUniformMatrix4fv(prog->getUniform("M"), 1, GL_FALSE, value_ptr(M->topMatrix()));
        }

        void render()
        {}
};

void processInput(GLFWwindow *window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
}

int main()
{

    std::string resourceDir = RESOURCE_DIR;
    std::string shaderDir = SHADER_DIR;
    Application *application = new Application();
    WindowManager *windowManager = new WindowManager();
    windowManager->init(800, 600);
    windowManager->setEventCallbacks(application);
    application->windowManager = windowManager;
    // glfw: initialize and configure
    // ------------------------------------------------------
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 4);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    
    #ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    #endif

    // glfw window creation
    GLFWwindow* window = glfwCreateWindow(800, 600, "mywindow", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    // glad: load all OpenGL function pointers
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    
    // application->init(shaderDir, resourceDir);
    // application->initGeom(resourceDir);


    glViewport(0, 0, 800, 600);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    float vertices[] = {
        -0.5f, -0.5f, 0.0f,
         0.5f, -0.5f, 0.0f,
         0.0f,  0.5f, 0.0f
    };
    while (!glfwWindowShouldClose(window))
    {
        processInput(window);

        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}
