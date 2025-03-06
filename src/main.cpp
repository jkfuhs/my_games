#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <memory>

#include "WindowManager.h"
#include "Program.h"
#include "GLSL.h"
#include "Shape.h"
#include "MatrixStack.h" 
#include "Camera.h"

#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader/tiny_obj_loader.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

// value_ptr for glm
#include "../lib/glm/gtc/type_ptr.hpp"
#include "../lib/glm/gtc/matrix_transform.hpp"

#define PROJECT_NAME "my_game"
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
        std::shared_ptr<Program> lightProg;
        // mix ratio uniform
        float mixRatio = 0.2;

        Camera camera;

        float deltaTime = 0.0f;
        float lastFrame = 0.0f;

        float lastX = SCR_WIDTH / 2.0f;
        float lastY = SCR_HEIGHT / 2.0f;
        bool firstMouse = true;
        glm::vec3 lightPos = glm::vec3(1.2f, 1.0f, 2.0f);
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
            glViewport(0, 0, width, height);
        }

        void init(const std::string &shaderDirectory, const std::string &resourceDirectory)
        {
            GLSL::checkVersion();

            // Set background color
            glClearColor(0.2f, 0.3f, 0.3f, 1.0f);

            // enable z-buffer
            glEnable(GL_DEPTH_TEST);

            camera = Camera(glm::vec3(0.0f, 0.0f, 3.0f));

            // Initialize the GLSL program that we will use for local shading
            prog = std::make_shared<Program>();
            prog->setVerbose(true);
            prog->setShaderNames(shaderDirectory + "/simpleVertex.glsl", shaderDirectory + "/simpleFragment.glsl");
            prog->init();
            prog->addAttribute("aPos");
            prog->addAttribute("aNormal");
            // prog->addUniform("texture1");
            // prog->addUniform("texture2");
            // prog->addUniform("mixRatio");
            prog->addUniform("model");
            prog->addUniform("view");
            prog->addUniform("projection");
            prog->addUniform("lightColor");
            prog->addUniform("objectColor");
            prog->addUniform("lightPos");
            prog->addUniform("viewPos");

            // Initialize shader for light sources
            lightProg = std::make_shared<Program>();
            lightProg->setVerbose(true);
            lightProg->setShaderNames(shaderDirectory + "/lightVertex.glsl", shaderDirectory + "/lightFragment.glsl");
            lightProg->init();
            lightProg->addAttribute("aPos");
            lightProg->addUniform("model");
            lightProg->addUniform("view");
            lightProg->addUniform("projection");
            lightProg->addUniform("Color");
            
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

        void initGeom(const std::string&resourceDirectory, unsigned int *VAO, unsigned int *lightVAO)
        {
            // read shapes from object files
            // 1. initialize mesh
            // 2. load geometry
            // 3. load material data
            float vertices[] = {
                -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
                 0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f, 
                 0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f, 
                 0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f, 
                -0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f, 
                -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f, 
            
                -0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 1.0f,
                 0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 1.0f,
                 0.5f,  0.5f,  0.5f,  0.0f,  0.0f, 1.0f,
                 0.5f,  0.5f,  0.5f,  0.0f,  0.0f, 1.0f,
                -0.5f,  0.5f,  0.5f,  0.0f,  0.0f, 1.0f,
                -0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 1.0f,
            
                -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,
                -0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
                -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
                -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
                -0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f,
                -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,
            
                 0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,
                 0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
                 0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
                 0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
                 0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,
                 0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,
            
                -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,
                 0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,
                 0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
                 0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
                -0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
                -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,
            
                -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,
                 0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,
                 0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
                 0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
                -0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
                -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f
            };
            unsigned int indices[] = {
                0, 1, 3,
                1, 2, 3               
            };
            unsigned int VBO;
            // unsigned int EBO;
            glGenVertexArrays(1, VAO);
            glGenBuffers(1, &VBO);
            // glGenBuffers(1, &EBO);

            glBindBuffer(GL_ARRAY_BUFFER, VBO);
            glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

            glBindVertexArray(*VAO);

            // glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
            // glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
            // position attribute
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
            glEnableVertexAttribArray(0);
            // normal vector attribute
            glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
            glEnableVertexAttribArray(1);
            // // texture attribute
            // glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
            // glEnableVertexAttribArray(2);

            glGenVertexArrays(1, lightVAO);
            glBindVertexArray(*lightVAO);
            glBindBuffer(GL_ARRAY_BUFFER, VBO);
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
            glEnableVertexAttribArray(0);

            // load textures
            // texture 1
            // unsigned int texture1, texture2;
            // glGenTextures(1, &texture1);
            // glBindTexture(GL_TEXTURE_2D, texture1);
            // // set texture wrapping/filtering options
            // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
            // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            // // load and generate texture
            // int width, height, nrChannels;
            // unsigned char *data = stbi_load((resourceDirectory + "/container.jpg").c_str(), &width, &height, &nrChannels, 0);
            // if (data)
            // {
            //     glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
            //     glGenerateMipmap(GL_TEXTURE_2D);
            // }
            // else
            // {
            //     std::cout << "Failed to load texture" << std::endl;
            // }
            // stbi_image_free(data);
            
            // // texture 2
            // glGenTextures(1, &texture2);
            // glBindTexture(GL_TEXTURE_2D, texture2);
            // // set the texture wrapping parameters
            // glGenTextures(1, &texture2);
            // glBindTexture(GL_TEXTURE_2D, texture2);
            // // set the texture wrapping parameters
            // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
            // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
            // // set texture filtering parameters
            // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
            // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            // // load image, create texture, and generate mipmaps
            // stbi_set_flip_vertically_on_load(true);
            // data = stbi_load((resourceDirectory + "/awesomeface.png").c_str(), &width, &height, &nrChannels, 0);
            // if (data)
            // {
            //     glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
            //     glGenerateMipmap(GL_TEXTURE_2D);
            // }
            // else
            // {
            //     std::cout << "Failed to load texture" << std::endl;
            // }
            // stbi_image_free(data);
            
            // prog->bind();
            // glUniform1i(prog->getUniform("texture1"), 0);
            // glUniform1i(prog->getUniform("texture2"), 1);
            
            // glActiveTexture(GL_TEXTURE0);
            // glBindTexture(GL_TEXTURE_2D, texture1);
            // glActiveTexture(GL_TEXTURE1);
            // glBindTexture(GL_TEXTURE_2D, texture2);
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

        void setMat4(std::shared_ptr<Program> prog, std::shared_ptr<MatrixStack>M) 
        {
            glUniformMatrix4fv(prog->getUniform("M"), 1, GL_FALSE, value_ptr(M->topMatrix()));
        }
        void updateVars()
        {
            float currentFrame = glfwGetTime();
            deltaTime = currentFrame - lastFrame;
            lastFrame = currentFrame;

            lightPos = glm::vec3(cos(currentFrame) * 1.2f, 1.0f + 2.0 * sin(currentFrame), 2.0f) ;

            camera.move(deltaTime);
        }
        void render(unsigned int VAO, unsigned int lightVAO)
        {
            glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            // view/projection transformations
            glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
            glm::mat4 view = camera.GetViewMatrix();

            // set uniforms
            prog->bind();
            glUniform3f(prog->getUniform("objectColor"), 1.0f, 0.5f, 0.31f);
            glUniform3fv(prog->getUniform("lightColor"), 1, glm::value_ptr(lightColor));
            glUniform3fv(prog->getUniform("lightPos"), 1, glm::value_ptr(lightPos));
            glUniform3fv(prog->getUniform("viewPos"), 1, glm::value_ptr(camera.Position));
            glUniformMatrix4fv(prog->getUniform("projection"), 1, GL_FALSE, glm::value_ptr(projection));
            glUniformMatrix4fv(prog->getUniform("view"), 1, GL_FALSE, glm::value_ptr(view));

            // light model transformations
            glm::mat4 model = glm::mat4(1.0f);
            glUniformMatrix4fv(prog->getUniform("model"), 1, GL_FALSE, glm::value_ptr(model));

            // render cube
            glBindVertexArray(VAO);
            glDrawArrays(GL_TRIANGLES, 0, 36);

            prog->unbind();
            // render lamp object
            lightProg->bind();
            glUniform3fv(lightProg->getUniform("Color"), 1, glm::value_ptr(lightColor));
            glUniformMatrix4fv(lightProg->getUniform("view"), 1, GL_FALSE, glm::value_ptr(view));
            glUniformMatrix4fv(lightProg->getUniform("projection"), 1, GL_FALSE, glm::value_ptr(projection));
            model  = glm::mat4(1.0f);
            model = glm::translate(model, lightPos);
            model = glm::scale(model, glm::vec3(0.2f));
            glUniformMatrix4fv(lightProg->getUniform("model"), 1, GL_FALSE, glm::value_ptr(model));
            
            glBindVertexArray(lightVAO);
            // glDrawArrays(GL_TRIANGLES, 0, 36);
            
            // glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
            lightProg->unbind();
        }
};


int main()
{

    std::string resourceDir = RESOURCE_DIR;
    std::string shaderDir = SHADER_DIR;
    Application *application = new Application();
    WindowManager *windowManager = new WindowManager();
    windowManager->init(800, 600, PROJECT_NAME);
    windowManager->setEventCallbacks(application);
    application->windowManager = windowManager;
    
    unsigned int VAO, lightVAO;
    application->init(shaderDir, resourceDir);
    application->initGeom(resourceDir, &VAO, &lightVAO);
    
    while (!glfwWindowShouldClose(windowManager->getHandle()))
    {
        application->updateVars();
        application->render(VAO, lightVAO);
        glfwSwapBuffers(windowManager->getHandle());
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}
