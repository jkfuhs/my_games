#include "Application.h"

const std::string RESOURCE_DIR = "D:/my_games/resources/";
const std::string SHADER_DIR = "D:/my_games/shaders/";

enum class WalkAnimation
{
    NONE,
    FORWARD,
    BACKWARD,
    LEFT,
    RIGHT
};

WalkAnimation walkAnimation = WalkAnimation::NONE;
Application * application = nullptr;

float mixRatio = 0.2f;
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

/*
void updateLegPositions(float currentTime)
{
    glm::mat4 &left = Models[0]->model_matrices[0];
    glm::mat4 &right = Models[0]->model_matrices[1];

    left = glm::mat4(1.0f);
    right = glm::mat4(1.0f);

    left = glm::translate(left, camera.Position);
    right = glm::translate(right, camera.Position);

    left = glm::rotate(left, glm::radians(-1.0f * camera.Yaw - 90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    right = glm::rotate(right, glm::radians(-1.0f * camera.Yaw - 90.0f), glm::vec3(0.0f, 1.0f, 0.0f));

    const float walkAnimationFreq = 4.0f;
    const float walkAnimationAmp = 0.1f;
    static float footPosition = 0.0f;
    static float footPosition2 = footPosition;
    
    //TODO: remove float overflow

    switch(walkAnimation)
    {
        case WalkAnimation::FORWARD:
            footPosition += deltaTime * walkAnimationFreq;
            
            footPosition2 = footPosition + glm::radians(180.0f);
        break;
        case WalkAnimation::BACKWARD:
            footPosition -= deltaTime * walkAnimationFreq;
            footPosition2 = footPosition + glm::radians(180.0f);
        break;
        // case WalkAnimation::LEFT:
        //     left = glm::rotate(left, glm::radians(-30.0f), glm::vec3(1.0f, 0.0f, 0.0f));
        //     right = glm::rotate(right, glm::radians(-30.0f), glm::vec3(1.0f, 0.0f, 0.0f));
        // break;
        // case WalkAnimation::RIGHT:
        //     left = glm::rotate(left, glm::radians(30.0f), glm::vec3(1.0f, 0.0f, 0.0f));
        //     right = glm::rotate(right, glm::radians(30.0f), glm::vec3(1.0f, 0.0f, 0.0f));
        // break;
        default:
        break;
    }
    left =  glm::translate(left, walkAnimationAmp * glm::vec3(0.0f,  -1.0f * glm::cos(footPosition) + 1.0, glm::sin(footPosition)));
    right = glm::translate(right, walkAnimationAmp * glm::vec3(0.0f,  -1.0f * glm::cos(footPosition2) + 1.0, glm::sin(footPosition2)));


    left = glm::translate(left, glm::vec3(-0.2f, 0.0f, 0.2f));
    right = glm::translate(right, glm::vec3(0.2f, 0.0f, 0.2f));
    left = glm::rotate(left, glm::radians(200.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    right = glm::rotate(right, glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    left = glm::translate(left, glm::vec3(-0.062f, -0.5f, 0.129f));
    right = glm::translate(right, glm::vec3(-0.062f, -0.5f, 0.129f));
    left = glm::scale(left, glm::vec3(0.3f));
    right = glm::scale(right, glm::vec3(0.3f));
}
*/

void init()
{
    application->setKeyBindSet(Camera_Type::FREE_CAMERA);
    application->addModel("backpack/backpack.obj");
}

void loop()
{

}


int main()
{

    const std::string resourceDir = RESOURCE_DIR;
    const std::string shaderDir = SHADER_DIR;
    application = new Application(shaderDir, resourceDir);
    application->run(init, loop);

    // de-allocate all resources
    application->shutdown();
    glfwTerminate();
    return 0;
}