#version 460

in vec3 fPos;
in vec2 fUV;

uniform int heightmapWidth;
uniform int heightmapHeight;

uniform sampler2D heightmap;

out vec4 fCol;

void main() {
	float xoffset = 1.0/heightmapWidth;
	float yoffset = 1.0/heightmapHeight;
	float h1x = texture2D(heightmap, fUV - vec2(xoffset, 0.0)).x;
	float h2x = texture2D(heightmap, fUV + vec2(xoffset, 0.0)).x;
	float h1y = texture2D(heightmap, fUV - vec2(0.0, yoffset)).x;
	float h2y = texture2D(heightmap, fUV + vec2(0.0, yoffset)).x;
	vec3 normal = normalize(vec3((h1x-h2x)/(2.0*xoffset), 1.0, (h1y-h2y)/(2.0*yoffset)));

	fCol = vec4(normal, 1.0);
}