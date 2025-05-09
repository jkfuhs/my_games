#version 330 core

uniform sampler2D texture1;
uniform sampler2D texture2;
uniform sampler2D texture3;
uniform sampler2D texture4;
uniform sampler2D texture5;

in vec2 TexCoords;
out vec4 FragColor;

void main()
{
    FragColor = texture(texture1, TexCoords);
}