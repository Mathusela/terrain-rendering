#include "framebuffer.hpp"

#include <extern/glad/glad.h>

using namespace TerrainRendering;

Framebuffer::Framebuffer() {
	glCreateFramebuffers(1, &id);
}

Framebuffer::~Framebuffer() noexcept {
	glDeleteFramebuffers(1, &id);
}

void Framebuffer::attachComponent(const FramebufferAttachment& attachment) {
	if (attachment.isTexture) {
		// Is texture
		glNamedFramebufferTexture(id, attachment.attachmentType, attachment.texture, 0);
	} else {
		// Is renderbuffer 
		glNamedFramebufferRenderbuffer(id, attachment.attachmentType, GL_RENDERBUFFER, attachment.renderbuffer->id);
	}
}

void Framebuffer::setDrawAttachments(std::vector<unsigned int> attachments) {
	glNamedFramebufferDrawBuffers(id, attachments.size(), &attachments[0]);
}

void Framebuffer::bind() {
	glBindFramebuffer(GL_FRAMEBUFFER, id);
}

void Framebuffer::unbind() {
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Framebuffer::clear() {
	bind();
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	unbind();
}

unsigned int Framebuffer::operator()() {
	return id;
}

FramebufferAttachment::FramebufferAttachment(Renderbuffer* rbo, unsigned int attachmentType): renderbuffer(rbo), attachmentType(attachmentType) {
	isTexture = false;

}

FramebufferAttachment::FramebufferAttachment(unsigned int texture, unsigned int attachmentType): texture(texture), attachmentType(attachmentType) {
	isTexture = true;
}

Renderbuffer::Renderbuffer(unsigned int width, unsigned int height, unsigned int internalFormat) {
	glCreateRenderbuffers(1, &id);
	glNamedRenderbufferStorage(id, internalFormat, width, height);
}

Renderbuffer::~Renderbuffer() noexcept {
	glDeleteRenderbuffers(1, &id);
}