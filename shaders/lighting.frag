#version 330 core
#extension GL_ARB_texture_cube_map_array : enable
in vec2 TexCoord;

out vec4 FragColor;

uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D gColour;


const int MAX_LIGHTS = 25;


uniform samplerCube  depthMaps[MAX_LIGHTS];
uniform vec3 lPosition[MAX_LIGHTS];
uniform vec3 lColour[MAX_LIGHTS];
uniform float lIntensity[MAX_LIGHTS];
uniform float lRange[MAX_LIGHTS];

uniform int numLights;
uniform vec3 viewPos;

float calculateShadow(vec3 fragPos, vec3 lightPosition, float lightRange, const int lightIndex) {
    vec3 fragToLight = fragPos - lightPosition;
    vec3 norma = normalize(fragToLight);
    //I know this is really stupid and innefficient
    //but openGL doesn't let you access sample arrays with variables
    //only hardcoded values
    //And I could not for the life of me figure out how to get around that
    //So im stuck with this mess
    float closestDepth = 0.0;
    if (lightIndex == 0) { closestDepth = texture(depthMaps[0], norma).r; }
    else if (lightIndex == 1) { closestDepth = texture(depthMaps[1], norma).r; }





    float currentDepth = length(fragToLight) / lightRange;
    float shadow = (currentDepth - 0.0005 > closestDepth) ? 0.2 : 1.0;

    return (shadow);
}

void main() {
    // Retrieve data from G-buffer
    vec3 fragPos = texture(gPosition, TexCoord).rgb;
    vec3 normal = texture(gNormal, TexCoord).rgb;
    vec3 albedo = texture(gColour, TexCoord).rgb;


    vec3 finalColor = vec3(0.0);

        for (int i = 0; i < numLights; ++i) {
            // Light properties
            vec3 lightPosition = lPosition[i];
            vec3 lightColor = lColour[i];
            float lightIntensity = lIntensity[i];
            float lightRange = lRange[i];

            // Calculate lighting
            vec3 lightDir = lightPosition - fragPos;
            float distance = length(lightDir);
            lightDir = normalize(lightDir);

            float lambert = max(dot(normal, lightDir), 0.0) / pow(distance, 2);
            vec3 diffuse = lambert * (lightIntensity * lightColor) * albedo;

            // Apply shadow

            float shadow = calculateShadow(fragPos, lightPosition, lightRange, i);
            finalColor += diffuse * shadow;

            //finalColor += diffuse;
        }

        // Tone mapping and gamma correction
    vec3 toneMapped = finalColor / (1.0 + finalColor);
    FragColor = vec4(pow(toneMapped, vec3(1.0 / 2.2)), 1.0);

    //FragColor = vec4(albedo, 1.0);
}
