#include <iostream>

#include "application.hpp"

// FIXME: FIX MSAA: https://learnopengl.com/Advanced-OpenGL/Anti-Aliasing
// FIXME: Turn off wireframe when shadowmapping
// FIXME: Shimmering - fix by moving light by discrete texel sizes
// TODO: Change tessellation behaviour when rendering shadowmap
// TODO: Grass rendering

int main() {
	TerrainRendering::Application app;
	app.run();

	return 0;
}