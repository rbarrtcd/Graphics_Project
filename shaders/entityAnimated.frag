#version 330 core

in vec4 fragColor;
in vec3 fragNormal;
in vec2 fragUV;
in vec3 fragPosition;

layout (location = 0) out vec4 outputColour;
layout (location = 1) out vec3 worldPosition;
layout (location = 2) out vec3 frag_norm_vec;
layout (location = 3) out float emit;

uniform sampler2D myTexture;

void main()
{
	vec4 texel = vec4(texture(myTexture, fragUV).rgb, 1.0);
	vec4 colouredTexture = texel * fragColor;
	outputColour = colouredTexture;
	frag_norm_vec = normalize(fragNormal);
	worldPosition = fragPosition;
	emit = 0.0;
}