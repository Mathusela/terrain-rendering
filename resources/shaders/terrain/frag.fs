#version 460

in vec3 fPos;
in vec2 fUV;

uniform sampler2D diffusemap;
uniform sampler2D normalmap;

uniform mat4 lightSpaceMatrix;

layout (location=0) out vec4 fColOut;
layout (location=1) out vec4 fPosDepthOut;
layout (location=2) out vec4 fNormOut;
layout (location=3) out vec4 fLightSpacePosOut;

void main() {
	vec3 col = texture2D(diffusemap, fUV).xyz;
	vec3 normal = texture2D(normalmap, fUV).xyz;
	
	fColOut = vec4(col, 1.0);
	fPosDepthOut = vec4(fPos, gl_FragCoord.z);
	fNormOut = vec4(normal, 1.0);
	fLightSpacePosOut = lightSpaceMatrix * vec4(fPos, 1.0);
}