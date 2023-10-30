#version 460

in layout(location = 0) vec3 vPos;
in layout(location = 1) vec2 vUV;

out vec3 fPos;
out vec2 fUV;

void main() {
	fPos = vPos;
	fUV = vUV;

	gl_Position = vec4(vPos, 1.0);
}