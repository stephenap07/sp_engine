#include <unordered_map>
#include <string>
#include <iostream>

#include <SDL2/SDL_opengl.h>
#include <SDL2/SDL_image.h>
#include <cassert>

#include "asset.h"

namespace sp {

static TextureCache texture_cache;

TextureCache::~TextureCache()
{
	for (auto it = cache.begin(); it != cache.end(); it++) {
		glDeleteTextures(1, &it->second);
	}
	cache.clear();
}

GLuint MakeTexture(const std::string &image_file)
{
	TextureCache::TextureMap::const_iterator search =
	    texture_cache.cache.find(image_file);

	if (search != texture_cache.cache.end()) {
		return search->second;
	}

	GLuint id;

	glGenTextures(1, &id);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, id);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	SDL_Surface *surface = IMG_Load(image_file.c_str());
	// assert(surface != nullptr);
	if (surface == nullptr) return 0;

	GLenum format;
	switch (surface->format->BytesPerPixel) {
		case 4:
			format = (surface->format->Rmask == 0x000000ff)
			          ? GL_RGBA : GL_BGRA;
			break;
		case 3:
			format = (surface->format->Rmask == 0x000000ff)
			          ? GL_RGB : GL_BGR;
			break;
		default:
			assert(false);
	}

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, surface->w,
	             surface->h, 0, format, GL_UNSIGNED_BYTE,
	             surface->pixels);
	SDL_FreeSurface(surface);

	glBindTexture(GL_TEXTURE_2D, 0);

	texture_cache.cache.insert({image_file, id});

	return id;
}

}
