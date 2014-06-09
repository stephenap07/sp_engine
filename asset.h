#ifndef _SP_ASSET_H_
#define _SP_ASSET_H_

#include <unordered_map>

namespace sp {

struct TextureCache {
	~TextureCache();

	typedef std::unordered_map<std::string, GLuint> TextureMap;
	TextureMap cache;
};

GLuint MakeTexture(const std::string &image_file);

}

#endif
