#version 330 core

in vec2 TexCoord;

out vec4 FragColor;

uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D gColour;

uniform vec3 lightPosition;
uniform vec3 lightColour;
uniform float lightIntensity;

uniform vec3 viewPos;

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


    FragColor = vec4(pow(toneMapped, vec3(1.0 / 2.2)), 1.0);
    //FragColor = vec4(normal, 1.0);
}
