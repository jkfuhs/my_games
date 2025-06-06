#version 330 core
struct Material {
    sampler2D texture_diffuse1;
    sampler2D texture_specular1;
    sampler2D texture_emission1;

    float shine;
    vec3 emission;
    vec3 specular;
};
struct DirLight {
    vec3 direction;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};
struct PointLight
{
    vec3 position;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;

    vec3 attenuation;
};
struct SpotLight
{
    int isOn;
    vec3 position;
    vec3 direction;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;

    vec3 attenuation;
    float innerCone;
    float outerCone;
};


#define NR_MAX_POINT_LIGHTS 100
#define NR_MAX_SPOT_LIGHTS 100

in vec3 Normal;
in vec3 FragPos;
in vec2 TexCoords;

uniform Material material;
uniform DirLight dirLight;
uniform int nPointLights;
uniform PointLight pointLights[NR_MAX_POINT_LIGHTS];
uniform int nSpotLights;
uniform SpotLight spotLights[NR_MAX_SPOT_LIGHTS];
uniform SpotLight spotLight;
uniform float refractiveIndex;
uniform samplerCube skybox;
uniform vec3 viewPos;

out vec4 FragColor;

vec3 CalculateDirLight(DirLight light, vec3 normal, vec3 viewDir)
{
    vec3 lightDir = normalize(-light.direction);
    // diffuse shading
    float diff = max(dot(normal, lightDir), 0.0);
    // specular shading
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shine);
    // combine results
    vec3 ambient = light.ambient    * vec3(texture(material.texture_diffuse1, TexCoords));
    vec3 diffuse = light.diffuse    * diff * vec3(texture(material.texture_diffuse1,  TexCoords));
    vec3 specular = light.specular  * spec * vec3(texture(material.texture_specular1, TexCoords));
    return (ambient + diffuse + specular);
}

vec3 CalculatePointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir)
{
    vec3 lightDir = normalize(light.position - fragPos);
    // diffuse shading
    float diff = max(dot(normal, lightDir), 0.0);
    // specular shading
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shine);
    // attenuation
    float distance = length(light.position - fragPos);
    float attenuation = 1.0 / (light.attenuation.x + light.attenuation.y * distance + 
                                light.attenuation.z * (distance*distance));
    // combine results
    vec3 ambient = light.ambient * vec3(texture(material.texture_diffuse1, TexCoords));
    vec3 diffuse = light.diffuse * diff * vec3(texture(material.texture_diffuse1, TexCoords));
    vec3 specular = light.specular * spec * vec3(texture(material.texture_specular1, TexCoords));
    return ambient + diffuse + specular;
}

vec3 CalculateSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir)
{
    vec3 lightDir = normalize(light.position - fragPos);
    float theta = dot(lightDir, normalize(-light.direction));

    // calculate attenuation
    float distance = length(light.position - fragPos);
    float attenuation = 1.0 / (light.attenuation.x + light.attenuation.y * distance +
                                light.attenuation.z * (distance * distance));

    if (theta > light.outerCone)
    {
        // in flashlight, do calculations
        float epsilon = light.innerCone - light.outerCone;
        float intensity = clamp((theta - light.outerCone) / epsilon, 0.0, 1.0);
        // diffuse shading
        float diff = max(dot(normal, lightDir), 0.0);
        // specular shading
        vec3 reflectDir = reflect(-lightDir, normal);
        float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shine);
        
        // combine results
        vec3 ambient =  light.ambient * vec3(texture(material.texture_diffuse1, TexCoords));
        vec3 diffuse =  light.diffuse * diff * vec3(texture(material.texture_diffuse1, TexCoords));
        vec3 specular =  light.specular * spec * vec3(texture(material.texture_specular1, TexCoords));
        return attenuation * (ambient + intensity * (diffuse + specular));
    }
    else
        // outside flashlight: only calculate ambient light
        return attenuation * light.ambient * vec3(texture(material.texture_diffuse1, TexCoords));
}

vec3 CalculateReflection(vec3 norm, vec3 fragPos, vec3 cameraPos, samplerCube skybox)
{
    vec3 I = normalize(fragPos - cameraPos);
    vec3 R = reflect(I, norm);
    return texture(skybox, R).rgb;
}

vec3 CalculateRefraction(vec3 norm, vec3 fragPos, vec3 cameraPos, samplerCube skybox)
{
    float ratio = 1.00 / refractiveIndex;
    vec3 I = normalize(fragPos - cameraPos);
    vec3 R = refract(I , norm, ratio);
    return texture(skybox, R).rgb;
}

void main()
{
    // properties
    vec3 norm = normalize(Normal);
    vec3 viewDir = normalize(viewPos - FragPos);

    vec3 result = vec3(0.0);

    // add in directional light component
    result += CalculateDirLight(dirLight, norm, viewDir);
    // repeat for each point light
    for (int i = 0; i < nPointLights; i++)
    {
        result += CalculatePointLight(pointLights[i], norm, FragPos, viewDir);
    }

    // spotlight
    for (int i = 0; i < nSpotLights; i++)
    {
        if (spotLight.isOn != 0)
            result += CalculateSpotLight(spotLights[i], norm, FragPos, viewDir);
    }
    
    // emission
    result += material.emission * vec3(texture(material.texture_emission1, TexCoords));

    // reflection
    vec3 reflection = CalculateReflection(norm, FragPos, viewPos, skybox);
    // result += reflection;

    // refraction
    vec3 refraction = CalculateRefraction(norm, FragPos, viewPos, skybox);
    // result += refraction;

    result *= 0.5;
    FragColor = vec4(result, 1.0);
}