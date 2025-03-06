#version 330 core
struct Material {
    sampler2D diffuse;
    sampler2D specular;
    sampler2D emission;
    float shine;
};

struct Light {
    // vec4 vector; // not needed for spotlight calculations
    // spotlight variables
    vec3 position;
    vec3 direction;
    float cutoff;
    float outerCutoff;

    // light scaling values
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;

    vec3 attenuation;
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

    // spotlight calculations
    // diffuse
    vec3 norm = normalize(Normal);
    vec3 lightDir;
    float attenuation;
    // if (light.vector.w == 0.0)
    // {
    //     // directional lighting
    //     lightDir = normalize(vec3(-light.vector));
    //     // no attenuation for directional lighting
    //     attenuation = 1.0;
    // }
    // else if (light.vector.w == 1.0)
    // {
    // point lighting
    lightDir = normalize(vec3(light.position) - FragPos);
    // spotlight calculations
    float theta = dot(lightDir, normalize(-light.direction));
    if (theta > light.outerCutoff)
    {
        float epsilon = light.cutoff - light.outerCutoff;
        float intensity = clamp((theta - light.outerCutoff) / epsilon, 0.0, 1.0);
        // do lighting calculations
        // compute attenuation for point light
        float distance = length(vec3(light.position) - FragPos);
        attenuation = 1.0 / (light.attenuation.x + light.attenuation.y * distance + light.attenuation.z * (distance * distance));
        // }
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
        emission = vec3(0.0);

        // apply light attenuation
        ambient *= attenuation;
        diffuse *= attenuation;
        specular *= attenuation;

        // apply spotlight intensity
        diffuse *= intensity;
        specular *= intensity;

        // sum them up
        vec3 result = ambient  + diffuse  + specular + emission;
        FragColor = vec4(result, 1.0);
    }
    else
        FragColor = vec4(light.ambient * vec3(texture(material.diffuse, TexCoords)), 1.0);
    
}