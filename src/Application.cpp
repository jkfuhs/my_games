#include "Application.h"

#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

// configuration options
// #define CULL_FACES 

Application::Application(const std::string &shaderDirectory, const std::string &resourceDirectory)
    : resourceDir(resourceDirectory), shaderDir(shaderDirectory), camera(Camera_Type::FREE_CAMERA, glm::vec3(0.0f, 0.0f, 3.0f))
{
    windowManager = new WindowManager();
    windowManager->init(SCR_WIDTH, SCR_HEIGHT, PROJECT_NAME.c_str());
    windowManager->setEventCallbacks(this);
    this->init();
}

void Application::run(std::function<void()> initFunc, std::function<void()> loopFunc)
{
    initFunc();
    while (!glfwWindowShouldClose(windowManager->getHandle()))
    {
        updateVars();
        loopFunc();
        render();
        
        glfwSwapBuffers(windowManager->getHandle());
        glfwPollEvents();
    }
}

void Application::cursorCallback(GLFWwindow *window, double xposIn, double yposIn)
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

void Application::setKeyBind(int key, std::function<void(int)> func)
{
    keybinds[key] = func;
}

void Application::setCameraType(Camera_Type type)
{
    camera.Type = type;
}

void Application::setKeyBindSet(Camera_Type type)
{
    switch (type)
    {
        case Camera_Type::FIRST_PERSON:
            // TODO: implement first person camera controls
            break;
        case Camera_Type::THIRD_PERSON:
            // TODO: implement third person camera controls
            break;
        case Camera_Type::FREE_CAMERA:
            setKeyBind(GLFW_KEY_W, [&](int action) { if (action == GLFW_PRESS) camera.addMotion(glm::vec3(0.0f, 0.0f, -1.0f));
                                                     if (action == GLFW_RELEASE) camera.addMotion(glm::vec3(0.0f, 0.0f, 1.0f)); });
            setKeyBind(GLFW_KEY_S, [&](int action) { if (action == GLFW_PRESS) camera.addMotion(glm::vec3(0.0f, 0.0f, 1.0f));
                                                     if (action == GLFW_RELEASE) camera.addMotion(glm::vec3(0.0f, 0.0f, -1.0f)); });
            setKeyBind(GLFW_KEY_A, [&](int action) { if (action == GLFW_PRESS) camera.addMotion(glm::vec3(-1.0f, 0.0f, 0.0f)); 
                                                     if (action == GLFW_RELEASE) camera.addMotion(glm::vec3(1.0f, 0.0f, 0.0f)); });
            setKeyBind(GLFW_KEY_D, [&](int action) { if (action == GLFW_PRESS) camera.addMotion(glm::vec3(1.0f, 0.0f, 0.0f)); 
                                                     if (action == GLFW_RELEASE) camera.addMotion(glm::vec3(-1.0f, 0.0f, 0.0f)); });
            setKeyBind(GLFW_KEY_SPACE, [&](int action) { if (action == GLFW_PRESS) camera.addMotion(glm::vec3(0.0f, 1.0f, 0.0f)); 
                                                         if (action == GLFW_RELEASE) camera.addMotion(glm::vec3(0.0f, -1.0f, 0.0f)); });
            setKeyBind(GLFW_KEY_V, [&](int action) { if (action == GLFW_PRESS) camera.addMotion(glm::vec3(0.0f, -1.0f, 0.0f));
                                                         if (action == GLFW_RELEASE) camera.addMotion(glm::vec3(0.0f, 1.0f, 0.0f)); });
            break;
    }
}

void Application::keyCallback(GLFWwindow *window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
    {
        glfwSetWindowShouldClose(window, GL_TRUE);
        return;
    }
    
    if (keybinds.find(key) != keybinds.end())
        keybinds[key](action);

}
        //    if (action == GLFW_PRESS)
        //     {
 
        //         switch(key)
        //         {
        //             case GLFW_KEY_ESCAPE:
                        
        //                     glfwSetWindowShouldClose(window, GL_TRUE);
        //                 break;
        //             case GLFW_KEY_UP:
        //                 mixRatio += 0.1;
        //                 mixRatio = (mixRatio > 1.0 ? 1.0 : mixRatio);
        //                 break;
        //             case GLFW_KEY_DOWN:
        //                 mixRatio -= 0.1;
        //                 mixRatio = (mixRatio < 0.0 ? 0.0 : mixRatio);
        //                 break;
        //             case GLFW_KEY_W:
        //                 camera.addMotion(glm::vec3(0.0f, 0.0f, -1.0f));
        //                 walkAnimation = WalkAnimation::FORWARD;
        //             break;
        //             case GLFW_KEY_S:
        //                 camera.addMotion(glm::vec3(0.0f, 0.0f, 1.0f));
        //                 walkAnimation = WalkAnimation::BACKWARD;
        //             break;
        //             case GLFW_KEY_A:
        //                 camera.addMotion(glm::vec3(-1.0f, 0.0f, 0.0f));
        //                 walkAnimation = WalkAnimation::LEFT;
        //             break;
        //             case GLFW_KEY_D:
        //                 camera.addMotion(glm::vec3(1.0f, 0.0f, 0.0f));
        //                 walkAnimation = WalkAnimation::RIGHT;
        //             break;
        //             case GLFW_KEY_SPACE:
        //                 camera.addMotion(glm::vec3(0.0f, 1.0f, 0.0f));
        //             break;
        //             case GLFW_KEY_V:
        //                 camera.addMotion(glm::vec3(0.0f, -1.0f, 0.0f));
        //             break;
        //             case GLFW_KEY_F:
        //                 flash_light_isOn = !flash_light_isOn;
        //             break;
        //             case GLFW_KEY_R:
        //                 show_rear_view = !show_rear_view;
        //             break;

        //         }
        //     }
        //     if (action == GLFW_RELEASE)
        //     {
        //         switch(key)
        //         {
        //             case GLFW_KEY_W:
        //                 camera.addMotion(glm::vec3(0.0f, 0.0f, 1.0f));
        //                 if (walkAnimation == WalkAnimation::FORWARD)
        //                     walkAnimation = WalkAnimation::NONE;
        //                 break;
        //             case GLFW_KEY_S:
        //                 camera.addMotion(glm::vec3(0.0f, 0.0f, -1.0f));
        //                 if (walkAnimation == WalkAnimation::BACKWARD)
        //                     walkAnimation = WalkAnimation::NONE;
        //             break;
        //             case GLFW_KEY_A:
        //                 camera.addMotion(glm::vec3(1.0f, 0.0f, 0.0f));
        //                 if (walkAnimation == WalkAnimation::LEFT)
        //                     walkAnimation = WalkAnimation::NONE;
        //             break;
        //             case GLFW_KEY_D:
        //                 camera.addMotion(glm::vec3(-1.0f, 0.0f, 0.0f));
        //                 if (walkAnimation == WalkAnimation::RIGHT)
        //                     walkAnimation = WalkAnimation::NONE;
        //             break;
        //             case GLFW_KEY_SPACE:
        //                 camera.addMotion(glm::vec3(0.0f, -1.0f, 0.0f));
        //             break;
        //             case GLFW_KEY_V:
        //                 camera.addMotion(glm::vec3(0.0f, 1.0f, 0.0f));
        //             break;
        //         }
        //     }
        // }

void Application::mouseCallback(GLFWwindow *window, int button, int action, int mods)
{
    // do nothing for now
}

void Application::scrollCallback(GLFWwindow *window, double in_deltaX, double in_deltaY)
{
    camera.adjustZoom(in_deltaY);
}

void Application::resizeCallback(GLFWwindow *window, int width, int height)
{
    CHECKED_GL_CALL(glViewport(0, 0, width, height));
}

void Application::initializeShader(const std::string &shaderName, bool verbose, const std::string &vertexShader, const std::string &fragmentShader, const std::vector<std::string> &attributes)
{
    if (shaders.find(shaderName) != shaders.end())
    {
        std::cerr << "Shader already exists: " << shaderName << std::endl;
        return;
    }
    shaders[shaderName] = Program();
    Program &prog = shaders[shaderName];

    prog.setVerbose(verbose);
    prog.setShaderNames(shaderDir + vertexShader, shaderDir + fragmentShader);
    prog.init();
    for (const auto &attribute : attributes)
    {
        prog.addAttribute(attribute);
    }
}

void defaultStencil()
{
    glStencilFunc(GL_ALWAYS, 0, 0xFF);
    glStencilMask(0xFF);
}
        
void Application::init()
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

    std::vector<std::string>attributes;

    // initialize default shader programs

    // // Initialize the GLSL program that we will use for local shading
    attributes = {"aPos", "aNormal", "aTexCoords"};
    initializeShader("default", true, "/simpleVertex.vs", "/simpleFragment.fs", attributes);
    
    // // Initialize shader for light sources
    // attributes = {"aPos"};
    // initializeShader(lightProg, true, "/lightVertex.vs", "/lightFragment.fs", attributes);
    
    // // Initialize shader for textured objects
    // attributes = {"aPos", "aNormal", "aTexCoords"};
    // initializeShader(texProg, true, "/texShader.vs", "/texShader.fs", attributes);

    // // Initialize shader for single color objects
    // attributes = {"aPos"};
    // initializeShader(singleColorProg, true, "/lightVertex.vs", "/singleColorShader.fs", attributes);          
    
    // // Initialize shader for screen rendering
    attributes = {"aPos", "aTexCoords"};
    initializeShader("screenShader", true, "/frameBuffers.vs", "/frameBuffers.fs", attributes);
    
    // // Initialize shader for skybox rendering
    attributes = {"aPos"};
    initializeShader("skyboxShader", true, "/skyboxShader.vs", "/skyboxShader.fs", attributes);

    // // Initialize shader for reflective objects
    // attributes = {"aPos", "aNormal"};
    // initializeShader(reflectShader, true, "/reflectionShader.vs", "/reflectionShader.fs", attributes);

    // // Initialize shader for chameleon objects
    // attributes = {"aPos", "aNormal", "aTexCoords"};
    // initializeShader(chameleonShader, true, "/simpleVertex.vs", "/chameleonShader.fs", attributes);

    initSky();
    initGeom();
}

Model *Application::addModel(const std::string &modelPath, const std::string &shaderName) 
{
    if (shaders.find(shaderName) == shaders.end())
    {
        std::cerr << "Shader not found: " << shaderName << std::endl;
        return nullptr;
    }

    Program &shader = shaders[shaderName];

    Model *model = new Model(resourceDir + modelPath);

    shader.models.push_back(model);

    return model;
}

void Application::initGeom()
{   
    Model *model;
    stbi_set_flip_vertically_on_load(false);

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
}

void Application::initSky()
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

void Application::initGround()
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

void Application::drawGround(std::shared_ptr<Program> &curS)
{
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(0.0f, -1.21f, 0.0f));

    glBindVertexArray(planeVAO);
    glBindTexture(GL_TEXTURE_2D, planeTexture);
    curS->setMat4("model", model);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);
}

void Application::setLightUniforms(Program &prog)
{
    prog.bind();
    // set directional light uniforms
    prog.setVector3f("dirLight.direction", directionalLight.direction);
    prog.setVector3f("dirLight.ambient", directionalLight.ambient);
    prog.setVector3f("dirLight.diffuse", directionalLight.diffuse);
    prog.setVector3f("dirLight.specular", directionalLight.specular);

    // set point light uniforms
    prog.setInt("nPointLights", pointLights.size());
    for (int i = 0; i < pointLights.size(); i++)
    {
        std::string num = std::to_string(i);
        prog.setVector3f("pointLights[" + num + "].position", pointLights[i].position);
        prog.setVector3f("pointLights[" + num + "].ambient", pointLights[i].ambient);
        prog.setVector3f("pointLights[" + num + "].diffuse", pointLights[i].diffuse);
        prog.setVector3f("pointLights[" + num + "].specular", pointLights[i].specular);
        prog.setVector3f("pointLights[" + num + "].attenuation", pointLights[i].attenuation);
    }

    // set spot light uniforms
    prog.setInt("nSpotLights", spotLights.size());
    for (int i = 0; i < spotLights.size(); i++)
    {
        std::string num = std::to_string(i);
        prog.setVector3f("spotLight[" + num + "].position", spotLights[i].position);
        prog.setVector3f("spotLight[" + num + "].direction", spotLights[i].direction);
        prog.setBool("spotLight[" + num + "].isOn", spotLights[i].isOn);
        prog.setVector3f("spotLight[" + num + "].ambient", spotLights[i].ambient);
        prog.setVector3f("spotLight[" + num + "].diffuse", spotLights[i].diffuse);
        prog.setVector3f("spotLight[" + num + "].specular", spotLights[i].specular);
        prog.setFloat("spotLight[" + num + "].innerCone", spotLights[i].innerCone);
        prog.setFloat("spotLight[" + num + "].outerCone", spotLights[i].outerCone);
        prog.setVector3f("spotLight[" + num + "].attenuation", spotLights[i].attenuation);
    }
    prog.unbind();
}



void Application::updateVars()
{
    float currentFrame = glfwGetTime();
    deltaTime = currentFrame - lastFrame;
    lastFrame = currentFrame;

    camera.move(deltaTime);

    // move legs
    setLightUniforms(shaders["default"]);
    // setLightUniforms(chameleonShader);
    // chameleonShader->bind();
    // chameleonShader->setVector3f("coloring", glm::vec3(0.0f, 1.0f * mixRatio, 1.0f * (1.0f - mixRatio)));
    // chameleonShader->unbind();
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


void Application::drawSky(glm::mat4 view, glm::mat4 projection)
{
    Program &skyboxShader = shaders["skyboxShader"];
    CHECKED_GL_CALL(glDepthMask(GL_FALSE));
    CHECKED_GL_CALL(glActiveTexture(GL_TEXTURE0 + skyboxTexture));
    skyboxShader.bind();
    view = glm::mat4(glm::mat3(view));
    skyboxShader.setMat4("view", view);
    skyboxShader.setMat4("projection", projection);
    skyboxShader.setInt("skybox", skyboxTexture);
    CHECKED_GL_CALL(glBindVertexArray(skyBoxVAO));
    CHECKED_GL_CALL(glBindTexture(GL_TEXTURE_CUBE_MAP, skyBoxTex));
    CHECKED_GL_CALL(glDrawArrays(GL_TRIANGLES, 0, 36));
    skyboxShader.unbind();
    CHECKED_GL_CALL(glDepthMask(GL_TRUE));
    CHECKED_GL_CALL(glActiveTexture(GL_TEXTURE0));
}
void Application::drawScene(glm::mat4 view, glm::mat4 projection)
{
    for (auto &shader : shaders)
    {
        shader.second.drawModels(view, projection, camera.Position);
    }
    // prog->bind();
    // CHECKED_GL_CALL(glActiveTexture(GL_TEXTURE0 + skyboxTexture));
    // prog->setInt("skybox", skyboxTexture);
    // prog->setFloat("refractiveIndex", 1.52f);
    // CHECKED_GL_CALL(glBindTexture(GL_TEXTURE_CUBE_MAP, skyBoxTex));
    // CHECKED_GL_CALL(glActiveTexture(GL_TEXTURE0));

    // prog->setMat4("view", view);
    // prog->setMat4("projection", projection);
    // prog->setVector3f("viewPos", camera.Position);

    // chameleonShader->bind();
    // CHECKED_GL_CALL(glActiveTexture(GL_TEXTURE0 + skyboxTexture));
    // chameleonShader->setInt("skybox", skyboxTexture);
    // chameleonShader->setFloat("refractiveIndex", 1.52f);
    // CHECKED_GL_CALL(glBindTexture(GL_TEXTURE_CUBE_MAP, skyBoxTex));
    // CHECKED_GL_CALL(glActiveTexture(GL_TEXTURE0));
    // chameleonShader->setMat4("view", view);
    // chameleonShader->setMat4("projection", projection);
    // chameleonShader->setVector3f("viewPos", camera.Position);

    // texProg->bind();
    // // render floor
    // texProg->setMat4("view", view);
    // texProg->setMat4("projection", projection);
    // drawGround(texProg);

    // chameleonShader->bind();


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

void Application::render()
{
    glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
    glm::mat4 view;
    // first pass
    // if (show_rear_view)
    // {
    //     glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    //     glEnable(GL_DEPTH_TEST);
    //     glEnable(GL_STENCIL_TEST);
    //     glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    //     glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    //     // view/projection transformations
    //     camera.Reverse();
    //     view = camera.GetViewMatrix();
    //     camera.Reverse();
    //     drawScene(view, projection);
    // }

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

    // if (show_rear_view)
    // {
    //     screenShader->bind();
    //     glm::mat4 model = glm::mat4(1.0f);
    //     model = glm::translate(model, glm::vec3(0.0f, 0.7f, 0.0f));
    //     model = glm::scale(model, glm::vec3(0.3f));
    //     screenShader->setMat4("model", model);
    //     glBindVertexArray(quadVAO);
    //     glDisable(GL_DEPTH_TEST);
    //     glBindTexture(GL_TEXTURE_2D, frame_texture);
    //     glDrawArrays(GL_TRIANGLES, 0, 6);
    // }   
}

void Application::shutdown()
{
    glDeleteVertexArrays(1, &skyBoxVAO);
    glDeleteVertexArrays(1, &quadVAO);
    glDeleteVertexArrays(1, &planeVAO);
    glDeleteBuffers(1, &quadVBO);
    glDeleteBuffers(1, &planeVBO);
    glDeleteFramebuffers(1, &fbo);
}

void Application::framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    CHECKED_GL_CALL(glViewport(0, 0, width, height));
}
