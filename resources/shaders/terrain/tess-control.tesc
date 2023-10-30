#version 460

#define MIN_TESSELLATION 5
#define MAX_TESSELLATION 64
#define MIN_TESS_DISTANCE 4
#define MAX_TESS_DISTANCE 50

layout (vertices = 4) out;

in vec2 cUV[];
in float height[];

uniform mat4 viewMatrix;
uniform vec3 offset;
uniform float quadScale;

out vec2 eUV[];

float interpFunction(float x) {
	return sqrt(x);
}

void main() {
	gl_out[gl_InvocationID].gl_Position = gl_in[gl_InvocationID].gl_Position;
	eUV[gl_InvocationID] = cUV[gl_InvocationID];
	
	if (gl_InvocationID == 0) {
		vec4 viewSpace00 = viewMatrix * vec4(quadScale*gl_in[0].gl_Position.xyz + offset + height[0]*vec3(0.0, 1.0, 0.0), 1.0);
		vec4 viewSpace01 = viewMatrix * vec4(quadScale*gl_in[1].gl_Position.xyz + offset + height[1]*vec3(0.0, 1.0, 0.0), 1.0);
		vec4 viewSpace10 = viewMatrix * vec4(quadScale*gl_in[2].gl_Position.xyz + offset + height[2]*vec3(0.0, 1.0, 0.0), 1.0);
		vec4 viewSpace11 = viewMatrix * vec4(quadScale*gl_in[3].gl_Position.xyz + offset + height[3]*vec3(0.0, 1.0, 0.0), 1.0);

		float distance00 = clamp((abs(viewSpace00.z) - MIN_TESS_DISTANCE)/(MAX_TESS_DISTANCE - MIN_TESS_DISTANCE), 0.0, 1.0);
		float distance01 = clamp((abs(viewSpace01.z) - MIN_TESS_DISTANCE)/(MAX_TESS_DISTANCE - MIN_TESS_DISTANCE), 0.0, 1.0);
		float distance10 = clamp((abs(viewSpace10.z) - MIN_TESS_DISTANCE)/(MAX_TESS_DISTANCE - MIN_TESS_DISTANCE), 0.0, 1.0);
		float distance11 = clamp((abs(viewSpace11.z) - MIN_TESS_DISTANCE)/(MAX_TESS_DISTANCE - MIN_TESS_DISTANCE), 0.0, 1.0);

		float tessLevel0 = mix( MAX_TESSELLATION, MIN_TESSELLATION, interpFunction(min(distance10, distance00)) );
		float tessLevel1 = mix( MAX_TESSELLATION, MIN_TESSELLATION, interpFunction(min(distance00, distance01)) );
		float tessLevel2 = mix( MAX_TESSELLATION, MIN_TESSELLATION, interpFunction(min(distance01, distance11)) );
		float tessLevel3 = mix( MAX_TESSELLATION, MIN_TESSELLATION, interpFunction(min(distance11, distance10)) );

		gl_TessLevelOuter[0] = tessLevel0;
		gl_TessLevelOuter[1] = tessLevel1;
		gl_TessLevelOuter[2] = tessLevel2;
		gl_TessLevelOuter[3] = tessLevel3;

		gl_TessLevelInner[0] = max(tessLevel1, tessLevel3);
		gl_TessLevelInner[1] = max(tessLevel0, tessLevel2);
	}
}