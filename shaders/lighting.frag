#version 330 core

in vec3 color;
in vec3 worldPosition;
in vec3 worldNormal;

out vec3 finalColor;

uniform vec3 lightPosition;
uniform vec3 lightIntensity;

uniform sampler2D depthMap;
uniform mat4 lightSpaceMatrix;


float calculateShadow(vec4 fragPosLightSpace) {
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;

    // Transform [-1, 1] to [0, 1]
    projCoords = projCoords * 0.5 + 0.5;

    //Code for if you want a 'lamp shade' effect
    if (projCoords.x < 0.0 || projCoords.x > 1.0 || projCoords.y < 0.0 || projCoords.y > 1.0){
        return 0.2;
    }

    float closestDepth = texture(depthMap, projCoords.xy).r;

    float currentDepth = projCoords.z;

    float shadow = (currentDepth >= closestDepth + 1e-4) ? 0.2 : 1.0;

    return shadow;
}

void main() {


    vec4 fragPosLightSpace = lightSpaceMatrix * vec4(worldPosition, 1.0);
    float shadow = calculateShadow(fragPosLightSpace);

    //Lights everything above the light
    //shadow = (worldPosition.y >= lightPosition.y) ? 1.0 : shadow;

    vec3 lightDir = lightPosition - worldPosition;
    float distance = length(lightDir);
    lightDir = normalize(lightDir);
    float lambert = max(dot(worldNormal, lightDir), 0.0)/pow(distance, 2);
    vec3 diffuse = lambert * lightIntensity * color;
    vec3 toneMapped = diffuse / (1.0 + diffuse);
    finalColor = pow(toneMapped, vec3(1.0 / 2.2)) * shadow;
}
