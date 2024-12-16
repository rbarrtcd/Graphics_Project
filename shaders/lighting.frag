#version 330 core

in vec2 TexCoord;

out vec4 FragColor;

uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D gColour;
uniform samplerCube depthMap;

uniform vec3 lightPosition;
uniform vec3 lightColour;
uniform float lightIntensity;
uniform float lightRange;


uniform vec3 viewPos;



float calculateShadow(vec3 fragPos) {
    vec3 fragToLight =  fragPos - lightPosition ;
    vec3 inverter = normalize(fragToLight);

    float closestDepth = (texture(depthMap, inverter).r);

    float currentDepth = length(fragToLight)/lightRange;

    float shadow = (currentDepth-0.0005f > closestDepth) ? 0.2 : 1.0;

    return shadow;
}

void main()
{
    // Retrieve data from G-buffer
    vec3 fragPos = texture(gPosition, TexCoord).rgb;
    vec3 normal = texture(gNormal, TexCoord).rgb;

    vec3 albedo = texture(gColour, TexCoord).rgb;

    // Calculate lighting
    vec3 lightDir = (lightPosition - fragPos);
    float distance = length(lightDir);
    lightDir = normalize(lightDir);
    float lambert = max(dot(normal, lightDir), 0.0)/pow(distance, 2);

    vec3 diffuse = lambert * (lightIntensity * lightColour) * albedo;
    vec3 toneMapped = diffuse / (1.0 + diffuse);


    FragColor = vec4(pow(toneMapped, vec3(1.0 / 2.2)), 1.0) * calculateShadow(fragPos);
    //FragColor = vec4(calculateShadow(fragPos),calculateShadow(fragPos),calculateShadow(fragPos), 1.0);
}
