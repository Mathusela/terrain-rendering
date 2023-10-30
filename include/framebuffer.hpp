#ifndef FRAMEBUFFER_HPP
#define FRAMEBUFFER_HPP

#include "texture.hpp"

#include <vector>

namespace TerrainRendering {

class Renderbuffer {
	unsigned int id;

public:
	Renderbuffer(unsigned int width, unsigned int height, unsigned int internalFormat);
	~Renderbuffer() noexcept;

	friend class Framebuffer;
};

class FramebufferAttachment {
	unsigned int attachmentType;

	union {
		Renderbuffer* renderbuffer;
		unsigned int texture;
	};

	// True if attachment is a texture; False if it is a renderbuffer.
	bool isTexture;

public:
	FramebufferAttachment(Renderbuffer* rbo, unsigned int attachmentType);
	FramebufferAttachment(unsigned int texture, unsigned int attachmentType);

	friend class Framebuffer;
};

class Framebuffer {
	unsigned int id;

public:
	Framebuffer();
	~Framebuffer() noexcept;

	void attachComponent(const FramebufferAttachment& attachment);
	void setDrawAttachments(std::vector<unsigned int> attachments);

	void bind();
	static void unbind();

	void clear();

	unsigned int operator()();
};

}	// namespace TerrainRendering

#endif