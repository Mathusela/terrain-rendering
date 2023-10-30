#ifndef TEXTURE_HPP
#define TEXTURE_HPP

#include <string>

namespace TerrainRendering {

struct ImageDimensions {
	int width;
	int height;
};

ImageDimensions load_image_to_texture_UNSIGNED_BYTE(unsigned int texture, std::string imagePath);
ImageDimensions load_image_to_texture_UNSIGNED_SHORT(unsigned int texture, std::string imagePath);

}	// namespace TerrainRendering

#endif