#version 460

in layout(location = 0) vec3 vPos;
in layout(location = 1) vec2 vUV;

uniform sampler2D heightmap;
uniform float maxHeight;

out vec2 cUV;
out float height;

void main() {
	cUV = vUV;

	height = maxHeight*texture2D(heightmap, vUV).x;

	gl_Position = vec4(vPos, 1.0);
}