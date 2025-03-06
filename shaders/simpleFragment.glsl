#version 330 core
struct Material {
    sampler2D diffuse;
    sampler2D specular;
    sampler2D emission;
    float shine;
};

struct Light {
    vec3 position;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

in vec3 Normal;
in vec3 FragPos;
in vec2 TexCoords;

uniform Material material;
uniform Light light;

uniform vec3 viewPos;

out vec4 FragColor;

void main()
{
    // ambient
    vec3 ambient = light.ambient * vec3(texture(material.diffuse, TexCoords));;

    // diffuse
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(light.position - FragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = light.diffuse * diff * vec3(texture(material.diffuse, TexCoords));

    // specular
    vec3 specularMap = vec3(texture(material.specular, TexCoords));
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec;
    if (material.shine > 0)
        spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shine);
    else
        spec = 1.0;
    vec3 specular = light.specular * spec * specularMap;

    // emission lighting
    vec3 emissionMask = step(vec3(1.0f), vec3(1.0f) - specularMap);
    vec3 emission = emissionMask * vec3(texture(material.emission, TexCoords));

    vec3 result = ambient  + diffuse  + specular + emission;
    FragColor = vec4(result, 1.0);
}