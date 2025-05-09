#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <iostream>
#include <memory>

#include "WindowManager.h"
#include "Program.h"
#include "GLSL.h"

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

// configuration options
// #define CULL_FACES 

// settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;
// const unsigned int SCR_WIDTH = 1920;
// const unsigned int SCR_HEIGHT = 1080;

class Application : public EventCallbacks
{
    public:
        WindowManager * windowManager = nullptr;
        
        // default shader program
        std::shared_ptr<Program> prog;
        // light source shader program
        std::shared_ptr<Program> lightProg;
        // simple texture object shader program
        std::shared_ptr<Program> texProg;
        // shader for single set color
        std::shared_ptr<Program> singleColorProg;
        // shader for rendering a screen
        std::shared_ptr<Program> screenShader;
        // shader for rendering a cubemapped skybox
        std::shared_ptr<Program> skyboxShader;
        // shader for rendering reflective object
        std::shared_ptr<Program> reflectShader;
        // shader for rendering leg objects
        std::shared_ptr<Program> chameleonShader;

        // mix ratio uniform
        float mixRatio = 0.6;

        Camera camera;

        float deltaTime = 0.0f;
        float lastFrame = 0.0f;
        Model *ourModel;

        float lastX = SCR_WIDTH / 2.0f;
        float lastY = SCR_HEIGHT / 2.0f;
        bool firstMouse = true;
        glm::vec3 directionalLight = glm::vec3(-0.2f, -1.0f, -0.3f);
        glm::vec3 lightColor = glm::vec3(1.0f, 1.0f, 1.0f);
        bool flash_light_isOn = false;
        bool show_rear_view = false;

        std::vector<glm::vec3> pointLightPositions = {
            glm::vec3(0.7f, 0.2f, 2.0f),
            glm::vec3(2.3f, -3.3f, -4.0f),
            glm::vec3(-4.0f, 2.0f, -12.0f),
            glm::vec3(0.0f, 0.0f, -3.0f)
        };

        unsigned int planeVAO, planeVBO;
        unsigned int quadVAO, quadVBO;
        unsigned int skyBoxVAO, skyBoxVBO;
        unsigned int fbo, rbo;  // frame buffer and render buffer objects

        // textures
        std::vector<glm::vec3> textures;
        unsigned int cubeTexture;
        unsigned int planeTexture;
        unsigned int transparentTexture;
        unsigned int skyBoxTex;
        unsigned int frame_texture;

        std::vector<Model *> Models;

        std::vector<glm::vec3> cubePositions;
        std::vector<glm::vec3> vegetationPositions;

        const unsigned int skyboxTexture = 11;

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
                    case GLFW_KEY_F:
                        flash_light_isOn = !flash_light_isOn;
                    break;
                    case GLFW_KEY_R:
                        show_rear_view = !show_rear_view;
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

        void initializeShader(std::shared_ptr<Program> &prog, bool verbose, const std::string &shaderDirectory, const std::string &vertexShader, const std::string &fragmentShader, const std::vector<std::string> &attributes)
        {
            prog = std::make_shared<Program>();
            prog->setVerbose(verbose);
            prog->setShaderNames(shaderDirectory + vertexShader, shaderDirectory + fragmentShader);
            prog->init();
            for (const auto &attribute : attributes)
            {
                prog->addAttribute(attribute);
            }
        }
        
        void init(const std::string &shaderDirectory, const std::string &resourceDirectory)
        {
            GLSL::checkVersion();

            // enable z-buffer
            CHECKED_GL_CALL(glEnable(GL_DEPTH_TEST));
            glDepthFunc(GL_LEQUAL); 
            
            // enable stencil buffer
            glEnable(GL_STENCIL_TEST);
            glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
            defaultStencil();

            // enable blending
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

            // enable face culling
            #ifdef CULL_FACES
            glEnable(GL_CULL_FACE);
            glCullFace(GL_BACK);
            glFrontFace(GL_CCW);
            #endif

            // intialize and set starting camera location
            camera = Camera(glm::vec3(0.0f, 0.0f, 3.0f));

            // Initialize the GLSL program that we will use for local shading
            std::vector<std::string> attributes = {"aPos", "aNormal", "aTexCoords"};
            initializeShader(prog, true, shaderDirectory, "/simpleVertex.vs", "/simpleFragment.fs", attributes);
            
            // Initialize shader for light sources
            attributes = {"aPos"};
            initializeShader(lightProg, true, shaderDirectory, "/lightVertex.vs", "/lightFragment.fs", attributes);
            
            // Initialize shader for textured objects
            attributes = {"aPos", "aNormal", "aTexCoords"};
            initializeShader(texProg, true, shaderDirectory, "/texShader.vs", "/texShader.fs", attributes);

            // Initialize shader for single color objects
            attributes = {"aPos"};
            initializeShader(singleColorProg, true, shaderDirectory, "/lightVertex.vs", "/singleColorShader.fs", attributes);          
            
            // Initialize shader for screen rendering
            attributes = {"aPos", "aTexCoords"};
            initializeShader(screenShader, true, shaderDirectory, "/frameBuffers.vs", "/frameBuffers.fs", attributes);
            
            // Initialize shader for skybox rendering
            attributes = {"aPos"};
            initializeShader(skyboxShader, true, shaderDirectory, "/skyboxShader.vs", "/skyboxShader.fs", attributes);

            // Initialize shader for reflective objects
            attributes = {"aPos", "aNormal"};
            initializeShader(reflectShader, true, shaderDirectory, "/reflectionShader.vs", "/reflectionShader.fs", attributes);

            // Initialize shader for chameleon objects
            attributes = {"aPos", "aNormal", "aTexCoords"};
            initializeShader(chameleonShader, true, shaderDirectory, "/simpleVertex.vs", "/chameleonShader.fs", attributes);
        }

        void initGeom(const std::string&resourceDirectory)
        {   
            Model *model;
            stbi_set_flip_vertically_on_load(false);
            // model = new Model((resourceDirectory + "/backpack/backpack.obj").c_str());
            // model->model_positions.push_back({glm::vec3(4.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f)});
            // model->model_positions.push_back({glm::vec3(-4.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f)});
            // Models.push_back(model);

            model = new Model((resourceDirectory + "/chameleon_leg/Chameleon_leg.obj").c_str());
            model->model_matrices.push_back(glm::mat4(1.0f));
            Models.push_back(model);

            // model = new Model((resourceDirectory + "/cube.obj").c_str());
            // Models.push_back(model);

            // model = new Model((resourceDirectory + "/quad.obj").c_str());
            // model->addTexture("/container.jpg");
            // Models.push_back(model);
            
            // set up vertex data (and buffer(s)) and configure vertex attributes
            // ------------------------------------------------------------------

            float quadVertices[] = { // vertex attributes for a quad that fills the entire screen in Normalized Device Coordinates.
                // positions   // texCoords
                -1.0f,  1.0f,  0.0f, 1.0f,
                -1.0f, -1.0f,  0.0f, 0.0f,
                 1.0f, -1.0f,  1.0f, 0.0f,
        
                -1.0f,  1.0f,  0.0f, 1.0f,
                 1.0f, -1.0f,  1.0f, 0.0f,
                 1.0f,  1.0f,  1.0f, 1.0f
            };
            
            // setup transparent VAO
            glGenVertexArrays(1, &quadVAO);
            glGenBuffers(1, &quadVBO);
            glBindVertexArray(quadVAO);
            glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
            glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);
            glEnableVertexAttribArray(0);
            glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
            glEnableVertexAttribArray(1);
            glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
            glBindVertexArray(0);
            
            // setup FBO
            glGenFramebuffers(1, &fbo);
            glBindFramebuffer(GL_FRAMEBUFFER, fbo);
            // setup FBO texture
            glGenTextures(1, &frame_texture);
            glBindTexture(GL_TEXTURE_2D, frame_texture);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, frame_texture, 0);
            // setup RBO
            glGenRenderbuffers(1, &rbo);
            glBindRenderbuffer(GL_RENDERBUFFER, rbo);
            glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, SCR_WIDTH, SCR_HEIGHT);
            glBindRenderbuffer(GL_RENDERBUFFER, 0); // unbind rbo

            glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);
            if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
            {
                std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;
            }
            glBindFramebuffer(GL_FRAMEBUFFER, 0);

            // load object textures
            stbi_set_flip_vertically_on_load(true);
            cubeTexture  = TextureFromFile("/container.jpg", resourceDirectory);
            stbi_set_flip_vertically_on_load(false);
            transparentTexture = TextureFromFile("/blending_transparent_window.png", resourceDirectory);

            // initialize cube positions
            cubePositions.push_back(glm::vec3(-1.0f, 0.0f, -1.0f));
            cubePositions.push_back(glm::vec3(2.0f, 0.0f, 0.0f));

            // initialize vegetation positions
            vegetationPositions.push_back(glm::vec3(-1.5f, 0.0f, -0.48f));
            vegetationPositions.push_back(glm::vec3(1.5f, 0.0f, 0.51f));
            vegetationPositions.push_back(glm::vec3(0.0f, 0.0f, 0.7f));
            vegetationPositions.push_back(glm::vec3(-0.3f, 0.0f, -2.3f));
            vegetationPositions.push_back(glm::vec3(0.5f, 0.0f, -0.6f));
        }

        void initSky(std::string& resourceDir)
        {
            std::vector<std::string> faces
            {
                "right.jpg",
                "left.jpg",
                "top.jpg",
                "bottom.jpg",
                "front.jpg",
                "back.jpg"
            };
            skyBoxTex = loadCubemap(resourceDir + "/skybox", faces);

            float skyboxVertices[] = {
                // positions          
                -1.0f,  1.0f, -1.0f,
                -1.0f, -1.0f, -1.0f,
                 1.0f, -1.0f, -1.0f,
                 1.0f, -1.0f, -1.0f,
                 1.0f,  1.0f, -1.0f,
                -1.0f,  1.0f, -1.0f,
            
                -1.0f, -1.0f,  1.0f,
                -1.0f, -1.0f, -1.0f,
                -1.0f,  1.0f, -1.0f,
                -1.0f,  1.0f, -1.0f,
                -1.0f,  1.0f,  1.0f,
                -1.0f, -1.0f,  1.0f,
            
                 1.0f, -1.0f, -1.0f,
                 1.0f, -1.0f,  1.0f,
                 1.0f,  1.0f,  1.0f,
                 1.0f,  1.0f,  1.0f,
                 1.0f,  1.0f, -1.0f,
                 1.0f, -1.0f, -1.0f,
            
                -1.0f, -1.0f,  1.0f,
                -1.0f,  1.0f,  1.0f,
                 1.0f,  1.0f,  1.0f,
                 1.0f,  1.0f,  1.0f,
                 1.0f, -1.0f,  1.0f,
                -1.0f, -1.0f,  1.0f,
            
                -1.0f,  1.0f, -1.0f,
                 1.0f,  1.0f, -1.0f,
                 1.0f,  1.0f,  1.0f,
                 1.0f,  1.0f,  1.0f,
                -1.0f,  1.0f,  1.0f,
                -1.0f,  1.0f, -1.0f,
            
                -1.0f, -1.0f, -1.0f,
                -1.0f, -1.0f,  1.0f,
                 1.0f, -1.0f, -1.0f,
                 1.0f, -1.0f, -1.0f,
                -1.0f, -1.0f,  1.0f,
                 1.0f, -1.0f,  1.0f
            };
            // setup skyBox VAO
            glGenVertexArrays(1, &skyBoxVAO);
            glGenBuffers(1, &skyBoxVBO);
            glBindVertexArray(skyBoxVAO);
            glBindBuffer(GL_ARRAY_BUFFER, skyBoxVBO);
            glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
            glEnableVertexAttribArray(0);
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
            glBindVertexArray(0);

        }

        void initGround(std::string& resourceDir)
        {
            float planeVertices[] = {
                // positions          // texture Coords (note we set these higher than 1 (together with GL_REPEAT as texture wrapping mode). this will cause the floor texture to repeat)
                20.0f, -0.5f,  20.0f,  8.0f, 0.0f,
                -20.0f, -0.5f,  20.0f,  0.0f, 0.0f,
                -20.0f, -0.5f, -20.0f,  0.0f, 8.0f,

                20.0f, -0.5f,  20.0f,  8.0f, 0.0f,
                -20.0f, -0.5f, -20.0f,  0.0f, 8.0f,
                20.0f, -0.5f, -20.0f,  8.0f, 8.0f								
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
            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model, glm::vec3(0.0f, -1.21f, 0.0f));

            glBindVertexArray(planeVAO);
            glBindTexture(GL_TEXTURE_2D, planeTexture);
            curS->setMat4("model", model);
            glDrawArrays(GL_TRIANGLES, 0, 6);
            glBindVertexArray(0);
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
        
        void setLightUniforms(std::shared_ptr<Program> prog)
        {
            prog->bind();
            // set directional light uniforms
            prog->setVector3f("dirLight.direction", directionalLight);
            prog->setVector3f("dirLight.ambient", lightColor * 0.2f);
            prog->setVector3f("dirLight.diffuse", lightColor * 0.9f);
            prog->setVector3f("dirLight.specular", lightColor * 1.0f);

            // set point light uniforms
            for (int i = 0; i < NR_POINT_LIGHTS; i++)
            {
                std::string num = std::to_string(i);
                prog->setVector3f("pointLights[" + num + "].position", pointLightPositions[i]);
                prog->setVector3f("pointLights[" + num + "].ambient", lightColor * 0.2f);
                prog->setVector3f("pointLights[" + num + "].diffuse", lightColor * 0.5f);
                prog->setVector3f("pointLights[" + num + "].specular", lightColor * 1.0f);
                prog->setVector3f("pointLights[" + num + "].attenuation", glm::vec3(1.0f, 0.09f, 0.032f));
            }

            // set spot light uniforms
            prog->setVector3f("spotLight.position", camera.Position);
            prog->setVector3f("spotLight.direction", camera.Front);
            prog->setBool("spotLight.isOn", flash_light_isOn);
            prog->setVector3f("spotLight.ambient", lightColor * 0.2f);
            prog->setVector3f("spotLight.diffuse", lightColor * 0.5f);
            prog->setVector3f("spotLight.specular", lightColor * 1.0f);
            prog->setFloat("spotLight.innerCone", glm::cos(glm::radians(12.5f)));
            prog->setFloat("spotLight.outerCone", glm::cos(glm::radians(15.0f)));
            prog->setVector3f("spotLight.attenuation", glm::vec3(1.0f, 0.09f, 0.032f));

            prog->unbind();
        }

        void updateLegPositions()
        {
            glm::mat4 &left = Models[0]->model_matrices[0];
            glm::mat4 &right = Models[0]->model_matrices[1];

            left = glm::mat4(1.0f);
            right = glm::mat4(1.0f);

            left = glm::translate(left, camera.Position);
            right = glm::translate(right, camera.Position);

            left = glm::rotate(left, glm::radians(-1.0f * camera.Yaw - 90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
            right = glm::rotate(right, glm::radians(-1.0f * camera.Yaw - 90.0f), glm::vec3(0.0f, 1.0f, 0.0f));

            left = glm::translate(left, glm::vec3(-0.2f, 0.0f, 0.2f));
            right = glm::translate(right, glm::vec3(0.2f, 0.0f, 0.2f));
            left = glm::rotate(left, glm::radians(200.0f), glm::vec3(0.0f, 1.0f, 0.0f));
            right = glm::rotate(right, glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f));
            left = glm::translate(left, glm::vec3(-0.062f, -0.5f, 0.129f));
            right = glm::translate(right, glm::vec3(-0.062f, -0.5f, 0.129f));
            left = glm::scale(left, glm::vec3(0.3f));
            right = glm::scale(right, glm::vec3(0.3f));
        }

        void updateVars()
        {
            float currentFrame = glfwGetTime();
            deltaTime = currentFrame - lastFrame;
            lastFrame = currentFrame;

            camera.move(deltaTime);

            // move legs
            updateLegPositions();

            setLightUniforms(prog);
            setLightUniforms(chameleonShader);
            chameleonShader->bind();
            chameleonShader->setVector3f("coloring", glm::vec3(0.0f, 1.0f * mixRatio, 1.0f * (1.0f - mixRatio)));
            chameleonShader->unbind();
        }
        void disableStencil()
        {
            glStencilFunc(GL_ALWAYS, 1, 0xFF);
            glStencilMask(0xFF);
        }
        void enableStencil()
        {
            glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
            glStencilMask(0x00);
        }
        void defaultStencil()
        {
            glStencilFunc(GL_ALWAYS, 0, 0xFF);
            glStencilMask(0xFF);
        }

        void drawSky(glm::mat4 view, glm::mat4 projection)
        {
            CHECKED_GL_CALL(glDepthMask(GL_FALSE));
            CHECKED_GL_CALL(glActiveTexture(GL_TEXTURE0 + skyboxTexture));
            skyboxShader->bind();
            view = glm::mat4(glm::mat3(view));
            skyboxShader->setMat4("view", view);
            skyboxShader->setMat4("projection", projection);
            skyboxShader->setInt("skybox", skyboxTexture);
            CHECKED_GL_CALL(glBindVertexArray(skyBoxVAO));
            CHECKED_GL_CALL(glBindTexture(GL_TEXTURE_CUBE_MAP, skyBoxTex));
            CHECKED_GL_CALL(glDrawArrays(GL_TRIANGLES, 0, 36));
            skyboxShader->unbind();
            CHECKED_GL_CALL(glDepthMask(GL_TRUE));
            CHECKED_GL_CALL(glActiveTexture(GL_TEXTURE0));
            
        }
        void drawScene(glm::mat4 view, glm::mat4 projection)
        {
            prog->bind();
            CHECKED_GL_CALL(glActiveTexture(GL_TEXTURE0 + skyboxTexture));
            prog->setInt("skybox", skyboxTexture);
            prog->setFloat("refractiveIndex", 1.52f);
            CHECKED_GL_CALL(glBindTexture(GL_TEXTURE_CUBE_MAP, skyBoxTex));
            CHECKED_GL_CALL(glActiveTexture(GL_TEXTURE0));


            prog->setMat4("view", view);
            prog->setMat4("projection", projection);
            prog->setVector3f("viewPos", camera.Position);

            chameleonShader->bind();
            CHECKED_GL_CALL(glActiveTexture(GL_TEXTURE0 + skyboxTexture));
            chameleonShader->setInt("skybox", skyboxTexture);
            chameleonShader->setFloat("refractiveIndex", 1.52f);
            CHECKED_GL_CALL(glBindTexture(GL_TEXTURE_CUBE_MAP, skyBoxTex));
            CHECKED_GL_CALL(glActiveTexture(GL_TEXTURE0));
            chameleonShader->setMat4("view", view);
            chameleonShader->setMat4("projection", projection);
            chameleonShader->setVector3f("viewPos", camera.Position);

            texProg->bind();
            // render floor
            texProg->setMat4("view", view);
            texProg->setMat4("projection", projection);
            drawGround(texProg);

            chameleonShader->bind();
            for (Model *model : Models)
            {
                model->Draw(chameleonShader);
            }
            // glBindTexture(GL_TEXTURE_2D, cubeTexture);
            // for (int i = 0; i < cubePositions.size(); i++)
            // {
            //     reflectShader->bind();
            //     disableStencil();
            //     glBindVertexArray(cubeVAO);
            //     model = glm::mat4(1.0f);
            //     model = glm::translate(model, cubePositions[i]);
            //     reflectShader->setMat4("model", model);
            //     glDrawArrays(GL_TRIANGLES, 0, 36);
            //     reflectShader->unbind();

                // enableStencil();
                // singleColorProg->bind();
                // glBindVertexArray(cubeVAO);
                // model = glm::scale(model, glm::vec3(1.1f));
                // singleColorProg->setMat4("model", model);
                // glDrawArrays(GL_TRIANGLES, 0, 36);
                // singleColorProg->unbind();
                // defaultStencil();
                // glClear(GL_STENCIL_BUFFER_BIT);
            // }

            drawSky(view, projection);

        }
        void render()
        {
            glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
            glm::mat4 view;
            // first pass
            if (show_rear_view)
            {
                glBindFramebuffer(GL_FRAMEBUFFER, fbo);
                glEnable(GL_DEPTH_TEST);
                glEnable(GL_STENCIL_TEST);
                glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
                glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

                // view/projection transformations
                camera.Reverse();
                view = camera.GetViewMatrix();
                camera.Reverse();
                drawScene(view, projection);
            }

            // texProg->bind();
            // glBindVertexArray(quadVAO);
            // glBindTexture(GL_TEXTURE_2D, transparentTexture);
            // std::map<float, glm::vec3> sorted;
            // for (int i=0; i < vegetationPositions.size(); i++)
            // {
            //     glm::vec3 temp = camera.Position - vegetationPositions[i];
            //     float distance = glm::dot(temp, temp);
            //     sorted[distance] = vegetationPositions[i];
            // }
            // for (std::map<float, glm::vec3>::reverse_iterator it = sorted.rbegin(); it != sorted.rend(); ++it)
            // {
            //     model = glm::mat4(1.0f);
            //     model = glm::translate(model, it->second);				
            //     texProg->setMat4("model", model);
            //     glDrawArrays(GL_TRIANGLES, 0, 6);
            // }
            
            // second pass
            glBindFramebuffer(GL_FRAMEBUFFER, 0); // default frame buffer
            glEnable(GL_DEPTH_TEST);
            glEnable(GL_STENCIL_TEST);
            glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

            view = camera.GetViewMatrix();
            drawScene(view, projection);    // draw rearview mirror

            if (show_rear_view)
            {
                screenShader->bind();
                glm::mat4 model = glm::mat4(1.0f);
                model = glm::translate(model, glm::vec3(0.0f, 0.7f, 0.0f));
                model = glm::scale(model, glm::vec3(0.3f));
                screenShader->setMat4("model", model);
                glBindVertexArray(quadVAO);
                glDisable(GL_DEPTH_TEST);
                glBindTexture(GL_TEXTURE_2D, frame_texture);
                glDrawArrays(GL_TRIANGLES, 0, 6);
            }   
        }

        void shutdown()
            {
                for (Model *model : Models)
                {
                    model->clearBuffers();
                
                    delete model;
                }
                Models.clear();
                glDeleteVertexArrays(1, &skyBoxVAO);
                glDeleteVertexArrays(1, &quadVAO);
                glDeleteVertexArrays(1, &planeVAO);
                glDeleteBuffers(1, &quadVBO);
                glDeleteBuffers(1, &planeVBO);
                glDeleteFramebuffers(1, &fbo);
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
    application->initSky(resourceDir);
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
