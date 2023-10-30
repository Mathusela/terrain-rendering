#include "texture.hpp"

#include <extern/glad/glad.h>
#define STB_IMAGE_IMPLEMENTATION
#include <extern/stb/stb_image.h>

#include <iostream>

using namespace TerrainRendering;

ImageDimensions TerrainRendering::load_image_to_texture_UNSIGNED_SHORT(unsigned int texture, std::string imagePath) {
	int width, height, colorChannelCount;
    unsigned short* image = stbi_load_16(imagePath.c_str(), &width, &height, &colorChannelCount, 0);

    GLenum imageFormat = (colorChannelCount == 4) ? GL_RGBA : GL_RGB;
	GLenum internalFormat = (colorChannelCount == 4) ? GL_RGBA16 : GL_RGB16;

#ifndef NDEBUG
    if (!image) {
        std::cout << "FAILED TO LOAD IMAGE" << std::endl;
    }
#endif

    glBindTexture(GL_TEXTURE_2D, texture);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, imageFormat, GL_UNSIGNED_SHORT, image);
    glGenerateMipmap(GL_TEXTURE_2D);

	glBindTexture(GL_TEXTURE_2D, 0);

    stbi_image_free(image);

	return {width, height};
}

ImageDimensions TerrainRendering::load_image_to_texture_UNSIGNED_BYTE(unsigned int texture, std::string imagePath) {
    int width, height, colorChannelCount;
    unsigned char* image = stbi_load(imagePath.c_str(), &width, &height, &colorChannelCount, 0);

    GLenum imageFormat = (colorChannelCount == 4) ? GL_RGBA : GL_RGB;

#ifndef NDEBUG
    if (!image) {
        std::cout << "FAILED TO LOAD IMAGE" << std::endl;
    }
#endif

    glBindTexture(GL_TEXTURE_2D, texture);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glTexImage2D(GL_TEXTURE_2D, 0, imageFormat, width, height, 0, imageFormat, GL_UNSIGNED_BYTE, image);
    glGenerateMipmap(GL_TEXTURE_2D);

	glBindTexture(GL_TEXTURE_2D, 0);

    stbi_image_free(image);

	return {width, height};
}