#version 330 core

in vec2 TexCoord;

out vec4 FragColor;

uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D gColour;
uniform sampler2D gEmit;

const int MAX_LIGHTS = 16;

uniform samplerCube  depthMaps[MAX_LIGHTS];
uniform vec3 lPosition[MAX_LIGHTS];
uniform vec3 lColour[MAX_LIGHTS];
uniform float lIntensity[MAX_LIGHTS];
uniform float lRange[MAX_LIGHTS];

uniform int numLights;

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
    else if (lightIndex == 2) { closestDepth = texture(depthMaps[2], norma).r; }
    else if (lightIndex == 3) { closestDepth = texture(depthMaps[3], norma).r; }
    else if (lightIndex == 4) { closestDepth = texture(depthMaps[4], norma).r; }
    else if (lightIndex == 5) { closestDepth = texture(depthMaps[5], norma).r; }
    else if (lightIndex == 6) { closestDepth = texture(depthMaps[6], norma).r; }
    else if (lightIndex == 7) { closestDepth = texture(depthMaps[7], norma).r; }
    else if (lightIndex == 8) { closestDepth = texture(depthMaps[8], norma).r; }
    else if (lightIndex == 9) { closestDepth = texture(depthMaps[9], norma).r; }
    else if (lightIndex == 10) { closestDepth = texture(depthMaps[10], norma).r; }
    else if (lightIndex == 11) { closestDepth = texture(depthMaps[11], norma).r; }
    else if (lightIndex == 12) { closestDepth = texture(depthMaps[12], norma).r; }
    else if (lightIndex == 13) { closestDepth = texture(depthMaps[13], norma).r; }
    else if (lightIndex == 14) { closestDepth = texture(depthMaps[14], norma).r; }
    else if (lightIndex == 15) { closestDepth = texture(depthMaps[15], norma).r; }





    float shadowBias = 0.01;
    float currentDepth = length(fragToLight) / lightRange;
    float shadow = (currentDepth - shadowBias > closestDepth) ? 0.2 : 1.0;

    return (shadow);
}

void main() {
    float emit = texture(gEmit, TexCoord).r;
    if (emit == 0.0){
        // Retrieve data from G-buffer
        vec3 fragPos = texture(gPosition, TexCoord).rgb;
        vec3 normal = texture(gNormal, TexCoord).rgb;
        vec3 albedo = texture(gColour, TexCoord).rgb;


        vec3 finalColor = vec3(0.0);

        for (int i = 0; i < MAX_LIGHTS; ++i) {
            // Light properties
            vec3 lightPosition = lPosition[i];
            vec3 lightColor = lColour[i];
            float lightIntensity = lIntensity[i];
            float lightRange = lRange[i];
            float shadow = calculateShadow(fragPos, lightPosition, lightRange, i);
            if (shadow == 1.0){

                // Calculate lighting
                vec3 lightDir = lightPosition - fragPos;
                float distance = length(lightDir);
                lightDir = normalize(lightDir);

                float lambert = max(dot(normal, lightDir), 0.02) / pow(distance, 2);
                vec3 diffuse = lambert * (lightIntensity * lightColor) * albedo;

                // Apply shadow


                finalColor += diffuse * shadow;

                //finalColor += diffuse;
            }
        }

        // Tone mapping and gamma correction
        vec3 toneMapped = finalColor / (1.0 + finalColor);
        vec3 correctedColor = pow(toneMapped, vec3(1.0 / 2.2));
        //if (length(correctedColor) < 0.05) {
        //    correctedColor = normalize(albedo) * 0.05;
        //}

        FragColor = vec4(correctedColor, 1.0);

    } else {
        vec3 color = texture(gColour, TexCoord).rgb;
        //if (length(color) < 0.05) {
        //    color = normalize(color) * 0.05;
        //}

        FragColor = vec4(color, 1.0);
    }

}
