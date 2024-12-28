#version 330 core

in vec2 TexCoord;

out vec4 FragColor;

uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D gColour;
uniform sampler2D gEmit;

uniform sampler2D depthMap;  // Single depth map for spotlight
uniform mat4 LVP;

uniform vec3 lPosition;  // Spotlight position
uniform vec3 lColour;    // Spotlight color
uniform float lIntensity;  // Spotlight intensity
uniform float lRange;      // Spotlight range
uniform vec3 lDirection;   // Spotlight direction (forward)

float TILE_SIZE = 640;
int CITY_SIZE = 10;

float calculateShadow(vec3 fragPos, vec3 lightPosition, float lightRange) {
    // Calculate light direction (towards the spotlight)
    vec3 fragToLight = fragPos - lightPosition;
    vec3 lightDir = normalize(lDirection);  // Spotlight direction

    // Project fragment position onto the spotlight direction (for depth comparison)
    float currentDepth = length(fragToLight) / lightRange;  // Depth at the fragment's position

    vec4 fragPosLightSpace = LVP * vec4(fragPos, 1.0);
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;

    // Transform [-1, 1] to [0, 1]
    projCoords = projCoords * 0.5 + 0.5;
    //if (projCoords.x < 0.0 || projCoords.x > 1.0 || projCoords.y < 0.0 || projCoords.y > 1.0){
    //    return 1.0;
    //}
    // Sample depth from the spotlight depth map
    float closestDepth = texture(depthMap, projCoords.xy).r;

    // Shadow bias to prevent shadow acne
    float shadowBias = 0.0015;

    // Compare current depth with the closest depth from the depth map
    float shadow = (currentDepth - shadowBias > closestDepth) ? 0.6 : 1.0;

    return shadow;
}

void main() {
    float emit = texture(gEmit, TexCoord).r;
    if (emit < 0.5) {
        // Retrieve data from G-buffer
        vec3 fragPos = texture(gPosition, TexCoord).rgb;
        vec3 normal = texture(gNormal, TexCoord).rgb;
        vec3 albedo = texture(gColour, TexCoord).rgb;
        fragPos.x = mod(fragPos.x, (CITY_SIZE*TILE_SIZE));
        if (fragPos.x < 0){
            fragPos.x += (CITY_SIZE*TILE_SIZE);
        }
        fragPos.z = mod(fragPos.z, (CITY_SIZE*TILE_SIZE));
        if (fragPos.z < 0){
            fragPos.z += (CITY_SIZE*TILE_SIZE);
        }
        vec3 finalColor = vec3(0.0);

        // Light properties for the single spotlight
        vec3 lightPosition = lPosition;
        vec3 lightColor = lColour;
        float lightIntensity = lIntensity;
        float lightRange = lRange;

        // Calculate shadow for the spotlight
        float shadow = calculateShadow(fragPos, lightPosition, lightRange);
        //float shadow = 1.0;

            // Calculate lighting (simple Lambertian diffuse shading)
            vec3 lightDir = lightPosition - fragPos;
            float distance = length(lightDir);
            lightDir = normalize(lightDir);

            // Lambertian diffuse term (diffuse = max(dot(normal, lightDir), 0) / distance^2)
            float lambert = max(dot(normal, lightDir), 0.01);
            vec3 diffuse = lambert * (lightIntensity * lightColor) * albedo;

            // Apply shadow
            finalColor = diffuse;


        //if (length(finalColor) < 0.5) {
        //    correctedColor = normalize(albedo) * 0.5;
        //}
        // Tone mapping and gamma correction
        vec3 toneMapped = finalColor / (1.0 + finalColor);
        vec3 correctedColor = pow(toneMapped, vec3(1.0 / 2.2));

        //vec3 lightDir = lightPosition - fragPos;
        FragColor = vec4(correctedColor, 1.0) * shadow;
        //FragColor = vec4(normalize(lightPosition - fragPos),1.0);

    } else {
        // Handle emissive objects (e.g., light sources themselves)
        vec3 color = texture(gColour, TexCoord).rgb;
        if (length(color) < 0.05) {
            color = normalize(color) * 0.05;
        }

        FragColor = vec4(color, 1.0);
    }
}
