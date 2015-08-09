#ifndef _SP_ASSET_H_
#define _SP_ASSET_H_

#include <unordered_map>
#include <SDL2/SDL.h>

#define MAX_TEXTURE_MIPS 14

namespace sp
{

struct TextureCache {
    ~TextureCache();

    typedef std::unordered_map<std::string, GLuint> TextureMap;
    TextureMap cache;
};

GLuint MakeTexture(const std::string &image_file, GLenum target);
GLuint MakeTextureFromSurface(const std::string &name, SDL_Surface *surface,
                              GLenum target);
}

#endif
