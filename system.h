#ifndef _SP_SYSTEM_H_
#define _SP_SYSTEM_H_

namespace sp {

struct SystemInfo
{
    void Init()
    {
        strcpy(platform, SDL_GetPlatform());
        num_cpus = SDL_GetCPUCount();
        ram = SDL_GetSystemRAM();
        l1_cache = SDL_GetCPUCacheLineSize();

        const GLubyte *_vendor = glGetString(GL_VENDOR);
        const GLubyte *_renderer = glGetString(GL_RENDERER);
        const GLubyte *_version = glGetString(GL_VERSION);

        memcpy(vendor, _vendor, sizeof(_vendor) + 1);
        memcpy(renderer, _renderer, sizeof(_renderer) + 1);
        memcpy(version, _version, sizeof(_version) + 1);
    }

    char platform[128];
    int num_cpus;
    int ram;
    int l1_cache;
    GLubyte vendor[128];
    GLubyte renderer[128];
    GLubyte version[128];
};

} // namespace sp

#endif
