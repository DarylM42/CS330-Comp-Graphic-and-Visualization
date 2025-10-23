#version 440 core

struct Material {
    vec3 ambientColor;
    float ambientStrength;
    vec3 diffuseColor;
    vec3 specularColor;
    float shininess;
};

struct PointLight {
    vec3 position;
    vec3 ambientColor;
    vec3 diffuseColor;
    vec3 specularColor;
    float focalStrength;
    float specularIntensity;
    vec3 spotDirection;
    float cutoff;
    float outerCutoff;
};

in vec3 fragmentPosition;
in vec3 fragmentVertexNormal;
in vec2 fragmentTextureCoordinate;

out vec4 outFragmentColor;

uniform bool bUseTexture;
uniform bool bUseLighting;
uniform vec4 objectColor;
uniform sampler2D objectTexture;
uniform vec3 viewPosition;
uniform vec2 UVscale;
uniform PointLight lightSources[8];
uniform Material material;
uniform int numActiveLights;

void main()
{
    if (bUseLighting && numActiveLights > 0) {
        vec3 normal = normalize(fragmentVertexNormal);
        PointLight light = lightSources[0];
        vec3 lightDir = normalize(light.position - fragmentPosition);
        
        // Calculate directional lighting intensity
        float intensity = max(dot(normal, lightDir), 0.0);
        
        // Apply lighting with yellow/golden tone for warm directional effect
        vec3 result = vec3(intensity, intensity * 0.9, intensity * 0.1);
        
        if (bUseTexture) {
            vec4 texColor = texture(objectTexture, fragmentTextureCoordinate * UVscale);
            outFragmentColor = vec4(result * texColor.rgb, texColor.a);
        } else {
            outFragmentColor = vec4(result * objectColor.rgb, objectColor.a);
        }
    } else {
        // No lighting - just display texture or color
        if (bUseTexture) {
            outFragmentColor = texture(objectTexture, fragmentTextureCoordinate * UVscale);
        } else {
            outFragmentColor = objectColor;
        }
    }
}