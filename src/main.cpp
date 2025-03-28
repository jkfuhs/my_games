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
        // light source shader program
        std::shared_ptr<Program> lightProg;
        // skysphere shader program
        std::shared_ptr<Program> skyProg;

        // mix ratio uniform
        float mixRatio = 0.2;

        Camera camera;

        float deltaTime = 0.0f;
        float lastFrame = 0.0f;
        Model *skySphere;
        unsigned int skySphereTexture;
        Model *ourModel;

        float lastX = SCR_WIDTH / 2.0f;
        float lastY = SCR_HEIGHT / 2.0f;
        bool firstMouse = true;
        glm::vec3 directionalLight = glm::vec3(-0.2f, -1.0f, -0.3f);
        glm::vec3 lightColor = glm::vec3(1.0f, 1.0f, 1.0f);

        unsigned int cubeVAO, cubeVBO;
        unsigned int planeVAO, planeVBO;

        unsigned int cubeTexture;
        unsigned int planeTexture;

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
            glDepthFunc(GL_LESS); 

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

            // Initialize shader for light sources
            lightProg = std::make_shared<Program>();
            lightProg->setVerbose(true);
            lightProg->setShaderNames(shaderDirectory + "/lightVertex.vs", shaderDirectory + "/lightFragment.fs");
            lightProg->init();
            lightProg->addAttribute("aPos");

            // Initialize shader for sky
            skyProg = std::make_shared<Program>();
            skyProg->setVerbose(true);
            skyProg->setShaderNames(shaderDirectory + "/skyShader.vs", shaderDirectory + "/skyShader.fs");
            skyProg->init();
            skyProg->addAttribute("aPos");
            skyProg->addAttribute("aTexCoords");
        }

        void initGeom(const std::string&resourceDirectory)
        {
            // skySphere = new Model((resourceDirectory + "/sphere.obj").c_str());
            // stbi_set_flip_vertically_on_load(true);
            // skySphereTexture = TextureFromFile("/jungle_panorama.jpg", resourceDirectory);
            // stbi_set_flip_vertically_on_load(false);
            // ourModel = new Model((resourceDirectory + "/backpack/backpack.obj").c_str());
            
            // set up vertex data (and buffer(s)) and configure vertex attributes
            // ------------------------------------------------------------------
            float cubeVertices[] = {
                // positions          // texture Coords
                -0.5f, -0.5f, -0.5f,  0.0f, 0.0f,
                0.5f, -0.5f, -0.5f,  1.0f, 0.0f,
                0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
                0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
                -0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
                -0.5f, -0.5f, -0.5f,  0.0f, 0.0f,

                -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
                0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
                0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
                0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
                -0.5f,  0.5f,  0.5f,  0.0f, 1.0f,
                -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,

                -0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
                -0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
                -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
                -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
                -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
                -0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

                0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
                0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
                0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
                0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
                0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
                0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

                -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
                0.5f, -0.5f, -0.5f,  1.0f, 1.0f,
                0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
                0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
                -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
                -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,

                -0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
                0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
                0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
                0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
                -0.5f,  0.5f,  0.5f,  0.0f, 0.0f,
                -0.5f,  0.5f, -0.5f,  0.0f, 1.0f
            };
            

            glGenVertexArrays(1, &cubeVAO);
            glGenBuffers(1, &cubeVBO);
            glBindVertexArray(cubeVAO);
            glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
            glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertices), &cubeVertices, GL_STATIC_DRAW);
            glEnableVertexAttribArray(0);
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
            glEnableVertexAttribArray(1);
            glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
            glBindVertexArray(0);

            stbi_set_flip_vertically_on_load(true);
            cubeTexture  = TextureFromFile("/marble.jpg", resourceDirectory);
        }

        void initGround(std::string& resourceDir)
        {
            float planeVertices[] = {
                // positions          // texture Coords (note we set these higher than 1 (together with GL_REPEAT as texture wrapping mode). this will cause the floor texture to repeat)
                5.0f, -0.501f,  5.0f,  2.0f, 0.0f,
                -5.0f, -0.501f,  5.0f,  0.0f, 0.0f,
                -5.0f, -0.501f, -5.0f,  0.0f, 2.0f,

                5.0f, -0.501f,  5.0f,  2.0f, 0.0f,
                -5.0f, -0.501f, -5.0f,  0.0f, 2.0f,
                5.0f, -0.501f, -5.0f,  2.0f, 2.0f								
            };

            glGenVertexArrays(1, &planeVAO);
            glGenBuffers(1, &planeVBO);
            glBindVertexArray(planeVAO);
            glBindBuffer(GL_ARRAY_BUFFER, planeVBO);
            glBufferData(GL_ARRAY_BUFFER, sizeof(planeVertices), &planeVertices, GL_STATIC_DRAW);
            glEnableVertexAttribArray(0);
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
            glEnableVertexAttribArray(1);
            glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
            glBindVertexArray(0);

            planeTexture = TextureFromFile("/metal.png", resourceDir);

        }

        void drawGround(std::shared_ptr<Program> curS)
        {
            // TODO:drawGround
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
        void shutdown()
        {
            glDeleteVertexArrays(1, &cubeVAO);
            glDeleteVertexArrays(1, &planeVAO);
            glDeleteBuffers(1, &cubeVBO);
            glDeleteBuffers(1, &planeVBO);
        }
        void updateVars()
        {
            float currentFrame = glfwGetTime();
            deltaTime = currentFrame - lastFrame;
            lastFrame = currentFrame;

            camera.move(deltaTime);
        }
        void render_sky(glm::mat4 projection)
        {
            skyProg->bind();
            CHECKED_GL_CALL(glActiveTexture(GL_TEXTURE0));
            skyProg->setInt("texture1", 0);
            CHECKED_GL_CALL(glBindTexture(GL_TEXTURE_2D, skySphereTexture));
            
            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model, camera.Position);
            model = glm::scale(model, glm::vec3(10.0f));
            glm::mat4 view = camera.GetViewMatrix();

            CHECKED_GL_CALL(glUniformMatrix4fv(skyProg->getUniform("model"), 1, GL_FALSE, glm::value_ptr(model)));
            // CHECKED_GL_CALL(glUniformMatrix4fv(skyProg->getUniform("view"), 1, GL_FALSE, glm::value_ptr(view)));
            CHECKED_GL_CALL(glUniformMatrix4fv(skyProg->getUniform("projection"), 1, GL_FALSE, glm::value_ptr(projection)));

            skySphere->Draw(skyProg);
            skyProg->unbind();
        }
        void render()
        {
            // CHECKED_GL_CALL(glClearColor(0.05f, 0.05f, 0.05f, 1.0f));
            // CHECKED_GL_CALL(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));

            // view/projection transformations
            glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
            glm::mat4 view = camera.GetViewMatrix();

            // render_sky(projection);
            // render pbjects
            // prog->bind();
            // prog->setFloat(("material.shine"), 32.0f);

            // prog->setVector3f("spotLight.position", camera.Position);
            // prog->setVector3f("spotLight.direction", camera.Front);
            // prog->setVector3f("spotLight.ambient", glm::vec3(0.05f));
            // prog->setVector3f("spotLight.diffuse", glm::vec3(0.5f));
            // prog->setVector3f("spotLight.specular", glm::vec3(1.0f));
            // prog->setVector3f("spotLight.attenuation", glm::vec3(1.0f, 0.09f, 0.032f));
            // prog->setFloat("spotLight.centerCone", glm::cos(glm::radians(12.5f)));
            // prog->setFloat("spotLight.outerCone", glm::cos(glm::radians(17.5f)));

            
            // set view/projection transforms
            // CHECKED_GL_CALL(glUniformMatrix4fv(prog->getUniform("projection"), 1, GL_FALSE, glm::value_ptr(projection)));
            // CHECKED_GL_CALL(glUniformMatrix4fv(prog->getUniform("view"), 1, GL_FALSE, glm::value_ptr(view)));
          
            // render loaded model
            // glm::mat4 model = glm::mat4(1.0f);
            // model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f));
            // model = glm::scale(model, glm::vec3(1.0f, 1.0f, 1.0f));
            // CHECKED_GL_CALL(glUniformMatrix4fv(prog->getUniform("model"), 1, GL_FALSE, glm::value_ptr(model)));
            // (*ourModel).Draw(prog);
            // prog->unbind();

            skyProg->bind();
            CHECKED_GL_CALL(glActiveTexture(GL_TEXTURE0));
            skyProg->setInt("texture1", 0);

            glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            glm::mat4 model = glm::mat4(1.0f);
            skyProg->setMat4("view", view);
            skyProg->setMat4("projection", projection);
            // cubes
            glBindVertexArray(cubeVAO);
            glBindTexture(GL_TEXTURE_2D, cubeTexture);
            model = glm::translate(model, glm::vec3(-1.0f, 0.0f, -1.0f));
            skyProg->setMat4("model", model);
            glDrawArrays(GL_TRIANGLES, 0, 36);
            model = glm::mat4(1.0f);
            model = glm::translate(model, glm::vec3(2.0f, 0.0f, 0.0f));
            skyProg->setMat4("model", model);
            glDrawArrays(GL_TRIANGLES, 0, 36);
            // floor
            glBindVertexArray(planeVAO);
            glBindTexture(GL_TEXTURE_2D, planeTexture);
            skyProg->setMat4("model", glm::mat4(1.0f));
            glDrawArrays(GL_TRIANGLES, 0, 6);
            glBindVertexArray(0);

            skyProg->unbind();
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
    application->initGround(resourceDir);
    
    while (!glfwWindowShouldClose(windowManager->getHandle()))
    {
        application->updateVars();
        application->render();
        
        glfwSwapBuffers(windowManager->getHandle());
        glfwPollEvents();
    }

    // de-allocate all resources
    application->shutdown();
    glfwTerminate();
    return 0;
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    CHECKED_GL_CALL(glViewport(0, 0, width, height));
}
