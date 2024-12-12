#version 330 core

in vec3 fragColor;
in vec2 fragUV;
in vec3 fragNormal;
in vec3 fragPosition;

uniform sampler2D textureSampler;

out vec4 gPosition;  // World position output
out vec4 gNormal;    // Normal output
out vec4 gAlbedo;    // Color output

void main() {
	// Pass the position, normal, and albedo to the G-buffer
	gPosition = vec4(fragPosition, 1.0);  // World space position
	gNormal = vec4(normalize(fragNormal), 1.0);  // Normal
	gAlbedo = vec4(fragColor, 1.0);
}
