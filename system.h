#ifndef _SP_SYSTEM_H_
#define _SP_SYSTEM_H_

namespace sp {

struct SystemInfo
{
    void QuerySystemInformation()
    {
        num_cpus = SDL_GetCPUCount();
        ram = SDL_GetSystemRAM();
        l1_cache = SDL_GetCPUCacheLineSize();

        const char *_vendor = reinterpret_cast<const char*>(glGetString(GL_VENDOR));
        const char *_renderer = reinterpret_cast<const char*>(glGetString(GL_RENDERER));
        const char *_version = reinterpret_cast<const char*>(glGetString(GL_VERSION));

        strcpy(platform, SDL_GetPlatform());
        strcpy(vendor, _vendor);
        strcpy(renderer, _renderer);
        strcpy(version, _version);
    }

    int num_cpus;
    int ram;
    int l1_cache;
    char platform[128];
    char vendor[128];
    char renderer[128];
    char version[128];
};

} // namespace sp

#endif
