#version 460

layout (quads, equal_spacing, ccw) in;

in vec2 eUV[];

uniform mat4 projectionMatrix;
uniform mat4 viewMatrix;
uniform vec3 offset;

uniform sampler2D heightmap;
uniform float quadScale;
uniform float maxHeight;

out vec3 fPos;
out vec2 fUV;

void main() {
	float u = gl_TessCoord.x;
	float v = gl_TessCoord.y;

	vec2 uv00 = eUV[0];
	vec2 uv01 = eUV[1];
	vec2 uv10 = eUV[2];
	vec2 uv11 = eUV[3];

	vec2 uv0 = (uv01 - uv00)*u + uv00;
	vec2 uv1 = (uv11 - uv10)*u + uv10;
	vec2 uv = (uv1 - uv0)*v + uv0;

	vec3 p00 = gl_in[0].gl_Position.xyz;
	vec3 p01 = gl_in[1].gl_Position.xyz;
	vec3 p10 = gl_in[2].gl_Position.xyz;
	vec3 p11 = gl_in[3].gl_Position.xyz;

	vec3 p0 = (p01 - p00)*u + p00;
	vec3 p1 = (p11 - p10)*u + p10;
	vec3 p = (p1 - p0)*v + p0;

	vec3 vertexPosition = quadScale*p + offset + maxHeight*texture2D(heightmap, uv).x*vec3(0.0, 1.0, 0.0);

	fPos = vertexPosition;
	fUV = uv;

	gl_Position = projectionMatrix * viewMatrix * vec4(vertexPosition, 1.0);
}