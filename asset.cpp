#include <unordered_map>
#include <string>
#include <iostream>

#include <SDL2/SDL_opengl.h>
#include <SDL2/SDL_image.h>
#include <cassert>

#include "asset.h"
#include "logger.h"

#if SDL_BYTEORDER == SDL_BIG_ENDIAN
    #define RMASK 0xff000000
    #define GMASK 0x00ff0000
    #define BMASK 0x0000ff00
    #define AMASK 0x000000ff
#else
    #define RMASK 0x000000ff
    #define GMASK 0x0000ff00
    #define BMASK 0x00ff0000
    #define AMASK 0xff000000
#endif

namespace sp {

static TextureCache texture_cache;

//------------------------------------------------------------------------------

TextureCache::~TextureCache()
{
	for (auto it = cache.begin(); it != cache.end(); it++) {
		glDeleteTextures(1, &it->second);
	}
	cache.clear();
}

GLuint MakeTextureFromSurface(const std::string &name, SDL_Surface *surface, GLenum target)
{
	TextureCache::TextureMap::const_iterator search =
	    texture_cache.cache.find(name);

	if (search != texture_cache.cache.end()) {
		return search->second;
	}

	assert(surface != nullptr);

	GLuint id;

	glGenTextures(1, &id);
	glBindTexture(target, id);

	GLenum format;
    GLint internal_format = GL_RGB8;
	switch (surface->format->BytesPerPixel) {
        case 1:
            format = (surface->format->Rmask == 0x000000ff)
			          ? GL_RGB : GL_BGR;
            break;
		case 4:
			format = (surface->format->Rmask == 0x000000ff)
			          ? GL_RGBA : GL_BGRA;
            internal_format = GL_RGBA8;
			break;
		case 3:
			format = (surface->format->Rmask == 0x000000ff)
			          ? GL_RGB : GL_BGR;
			break;
		default:
            sp::log::ErrorLog("Invalid format for surface: %d\n", surface->format->BytesPerPixel);
			assert(false);
	}

    int bpp = surface->format->BitsPerPixel;
    // TODO: Remove hard coded values
    int img_w = 512;
    int img_h = 512;

    switch(target) {
        case GL_TEXTURE_1D:
            glTexImage1D(
                GL_TEXTURE_1D,
                0, internal_format,
                surface->w, 0,
                format, GL_UNSIGNED_BYTE,
                surface->pixels);
            break;

        case GL_TEXTURE_2D:
            glTexParameteri(target, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(target, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri(target, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(target, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

            glTexImage2D(
                target,
                0,
                internal_format,
                surface->w, surface->h, 0,
                format, GL_UNSIGNED_BYTE,
                surface->pixels);

            break;

        case GL_TEXTURE_CUBE_MAP:
            glTexParameteri(target, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            glTexParameteri(target, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            glTexParameteri(target, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(target, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            glTexParameteri(target, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

            {
                SDL_Surface *sub_surface = SDL_CreateRGBSurface(0, img_w, img_h, bpp,
                                                                RMASK, GMASK, BMASK, AMASK); 
                SDL_Rect source_rect = {
                    512, 512, 512, 512 
                };
                if (SDL_BlitSurface(surface, &source_rect, sub_surface, nullptr) != 0) {
                    std::cerr << SDL_GetError() << std::endl;
                }

                assert(sub_surface != nullptr);

                glTexImage2D(
                    GL_TEXTURE_CUBE_MAP_POSITIVE_Z,
                    0,
                    GL_RGBA8,
                    img_w, img_h, 0,
                    format, GL_UNSIGNED_BYTE,
                    sub_surface->pixels);

                SDL_FreeSurface(sub_surface);
            }

            {
                SDL_Surface *sub_surface = SDL_CreateRGBSurface(0, img_w, img_h, bpp,
                                                                RMASK, GMASK, BMASK, AMASK); 
                SDL_Rect source_rect = {
                    0, 513, 512, 512 
                };
                if (SDL_BlitSurface(surface, &source_rect, sub_surface, nullptr) != 0) {
                    std::cerr << SDL_GetError() << std::endl;
                }

                assert(sub_surface != nullptr);

                glTexImage2D(
                    GL_TEXTURE_CUBE_MAP_NEGATIVE_X,
                    0,
                    GL_RGBA8,
                    img_w, img_h, 0,
                    format, GL_UNSIGNED_BYTE,
                    sub_surface->pixels);

                SDL_FreeSurface(sub_surface);
            }


            {
                SDL_Surface *sub_surface = SDL_CreateRGBSurface(0, img_w, img_h, bpp,
                                                                RMASK, GMASK, BMASK, AMASK); 
                SDL_Rect source_rect = {
                    512, 0, 512, 512 
                };
                if (SDL_BlitSurface(surface, &source_rect, sub_surface, nullptr) != 0) {
                    std::cerr << SDL_GetError() << std::endl;
                }

                assert(sub_surface != nullptr);

                glTexImage2D(
                    GL_TEXTURE_CUBE_MAP_POSITIVE_Y,
                    0,
                    GL_RGBA8,
                    img_w, img_h, 0,
                    format, GL_UNSIGNED_BYTE,
                    sub_surface->pixels);

                SDL_FreeSurface(sub_surface);
            }


            {
                SDL_Surface *sub_surface = SDL_CreateRGBSurface(0, img_w, img_h, bpp,
                                                                RMASK, GMASK, BMASK, AMASK); 
                SDL_Rect source_rect = {
                    2 * 512, 513, 512, 512 
                };
                if (SDL_BlitSurface(surface, &source_rect, sub_surface, nullptr) != 0) {
                    std::cerr << SDL_GetError() << std::endl;
                }

                assert(sub_surface != nullptr);

                glTexImage2D(
                    GL_TEXTURE_CUBE_MAP_POSITIVE_X,
                    0,
                    GL_RGBA8,
                    img_w, img_h, 0,
                    format, GL_UNSIGNED_BYTE,
                    sub_surface->pixels);

                SDL_FreeSurface(sub_surface);
            }


            {
                SDL_Surface *sub_surface = SDL_CreateRGBSurface(0, img_w, img_h, bpp,
                                                                RMASK, GMASK, BMASK, AMASK); 
                SDL_Rect source_rect = {
                    512, 2 * 512, 512, 512 
                };
                if (SDL_BlitSurface(surface, &source_rect, sub_surface, nullptr) != 0) {
                    std::cerr << SDL_GetError() << std::endl;
                }

                assert(sub_surface != nullptr);

                glTexImage2D(
                    GL_TEXTURE_CUBE_MAP_NEGATIVE_Y,
                    0,
                    GL_RGBA8,
                    img_w, img_h, 0,
                    format, GL_UNSIGNED_BYTE,
                    sub_surface->pixels);

                SDL_FreeSurface(sub_surface);
            }


            {
                SDL_Surface *sub_surface = SDL_CreateRGBSurface(0, img_w, img_h, bpp,
                                                                RMASK, GMASK, BMASK, AMASK); 
                SDL_Rect source_rect = {
                    3 * 512, 513, 512, 512 
                };
                if (SDL_BlitSurface(surface, &source_rect, sub_surface, nullptr) != 0) {
                    std::cerr << SDL_GetError() << std::endl;
                }

                assert(sub_surface != nullptr);

                glTexImage2D(
                    GL_TEXTURE_CUBE_MAP_NEGATIVE_Z,
                    0,
                    GL_RGBA8,
                    img_w, img_h, 0,
                    format, GL_UNSIGNED_BYTE,
                    sub_surface->pixels);

                SDL_FreeSurface(sub_surface);
            }

            break;

        default:
            break;

    }

	glBindTexture(target, 0);
	texture_cache.cache.insert({name, id});

    return id;
}

//------------------------------------------------------------------------------

GLuint MakeTexture(const std::string &image_file, GLenum target)
{
	SDL_Surface *surface = IMG_Load(image_file.c_str());
    GLuint id = MakeTextureFromSurface(image_file, surface, target);
	SDL_FreeSurface(surface);

	return id;
}

} // namespace sp
