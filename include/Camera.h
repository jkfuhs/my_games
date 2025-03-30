#ifndef CAMERA_H
#define CAMERA_H

#include <glad/glad.h>
#include "../lib/glm/glm.hpp"
#include "../lib/glm/gtc/matrix_transform.hpp"

#define ZOOM_MIN     1.0f
#define ZOOM_MAX     45.0f
#define PITCH_MIN   -89.0f
#define PITCH_MAX    89.0f

enum Camera_Movement {
    FORWARD,
    BACKWARD,
    LEFT,
    RIGHT
};

// Default camera values
const float YAW             = -90.0f;
const float PITCH           = 0.0f;
const float SPEED           = 2.5f;
const float SENSITIVITY     = 0.1f;
const float ZOOM            = 45.0f;

class Camera
{
public:
    // camera Attributes
    glm::vec3 Position;
    glm::vec3 Front;
    glm::vec3 Up;
    glm::vec3 Right;
    glm::vec3 WorldUp;
    glm::vec3 Motion;
    // Euler angles
    float Yaw;
    float Pitch;
    // camera options
    float MovementSpeed;
    float MouseSensitivity;
    float Zoom;

    // constructor with vectors
    Camera(glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f), float yaw = YAW, float pitch = PITCH) 
        : Front(glm::vec3(0.0f, 0.0f, -1.0f)),
          Motion(glm::vec3(0.0f)),
          MovementSpeed(SPEED), 
          MouseSensitivity(SENSITIVITY), 
          Zoom(ZOOM)
    {
        Position = position;
        WorldUp = up;
        Yaw = yaw;
        Pitch = pitch;
        updateCameraVectors();
    }
    // constructor with scalar values
    Camera(float posX, float posY, float posZ, float upX, float upY, float upZ, float yaw, float pitch) 
    : Front(glm::vec3(0.0f, 0.0f, -1.0f)),
      Motion(glm::vec3(0.0f)),
      MovementSpeed(SPEED), 
      MouseSensitivity(SENSITIVITY), 
      Zoom(ZOOM)
    {
        Position = glm::vec3(posX, posY, posZ);
        WorldUp = glm::vec3(upX, upY, upZ);
        Yaw = yaw;
        Pitch = pitch;
        updateCameraVectors();
    }

    // returns the view matrix calculated using Euler Angles and the LookAt Matrix
    glm::mat4 GetViewMatrix()
    {
        return myLookAt(Position, Position + Front, Up);
    }

    // translate the camera position in the specified direction and speed
    void translateCamera(glm::vec3 direction, float speed, float deltaTime)
    {
        direction = normalize(direction);
        Position += direction * speed * deltaTime;
    }

    // translate the camera in the specified direction RELATIVE to Camera->Front
    void moveCamera(glm::vec3 direction, float speed, float deltaTime)
    {
        direction = normalize(direction);
        Position -= direction.z * Front * speed * deltaTime;
        Position += direction.x * Right * speed * deltaTime;
    }

    void Reverse()
    {
        Front *= -1.0f;
        Right *= -1.0f;
    }

    // set Position to specified value
    void setCameraPos(glm::vec3 position)
    {
        Position = position;
    }

    void rotateCamera(float delta_yaw, float delta_pitch, GLboolean constrainPitch = true)
    {
        Yaw     += delta_yaw;
        Pitch   += delta_pitch;

        // if pitch is out of bounds, clamp it
        if (constrainPitch)
        {
            if (Pitch > PITCH_MAX)
                Pitch = PITCH_MAX;
            if (Pitch < PITCH_MIN)
                Pitch = PITCH_MIN;
        }

        // update Front, Right, and Up Vectors
        updateCameraVectors();
    }

    void adjustZoom(float zoom_offset)
    {
        Zoom -= zoom_offset;
        if (Zoom < ZOOM_MIN)
            Zoom = ZOOM_MIN;
        if (Zoom > ZOOM_MAX)
            Zoom = ZOOM_MAX;
    }

    void addMotion(glm::vec3(direction))
    {
        Motion += direction * MovementSpeed;
    }

    // moves the camera based on the values in Motion
    void move(float deltaTime)
    {
        glm::vec3 front = Front;
        front.y = 0;
        glm::vec3 right = Right;
        right.y = 0;
        if (Motion != glm::vec3(0))
        {
            Position -= Motion.z * front * deltaTime;
            Position += Motion.x * right * deltaTime;
            Position += Motion.y * WorldUp * deltaTime;
        }
    }
private:
    // updates Camera's Euler Angles from Yaw and Pitch
    void updateCameraVectors()
    {
        // recalculate front vector from Yaw and Pitch
        glm::vec3 front;
        front.x = cos(glm::radians(Yaw)) * cos(glm::radians(Pitch));
        front.y = sin(glm::radians(Pitch));
        front.z = sin(glm::radians(Yaw)) * cos(glm::radians(Pitch));
        Front = glm::normalize(front);
        // recalculate Right and Up vectors
        Right   = glm::normalize(glm::cross(Front, WorldUp));
        Up      = glm::normalize(glm::cross(Right, Front));
    }

    glm::mat4 myLookAt(glm::vec3 position, glm::vec3 target, glm::vec3 up)
    {
        glm::mat4 a = glm::mat4(glm::transpose(glm::mat3(Right, up, position - target)));
        glm::mat4 b = glm::mat4(glm::mat3(1.0f));
        b[3] = glm::vec4(-1.0f * position, 1.0f);
        return a * b;
    }

};


#endif //CAMERA_H