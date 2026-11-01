#include "framework/loader.hpp"
#ifdef _WIN32
#include <windows.h>
#else
#include <dlfcn.h>
#endif

Loader::~Loader() {
    if (instance && destroy) destroy(instance);
#ifdef _WIN32
    if (library) FreeLibrary(static_cast<HMODULE>(library));
#else
    if (library) dlclose(library);
#endif
}

bool Loader::open(const std::string& path, std::string& error) {
    if (library) { error = "Plugin already open"; return false; }
#ifdef _WIN32
    library = LoadLibraryA(path.c_str());
    if (!library) { error = "Could not load plugin"; return false; }
    auto create = reinterpret_cast<Cpu* (*)()>(GetProcAddress(static_cast<HMODULE>(library), "create_cpu"));
    destroy = reinterpret_cast<void (*)(Cpu*)>(GetProcAddress(static_cast<HMODULE>(library), "destroy_cpu"));
#else
    library = dlopen(path.c_str(), RTLD_NOW);
    if (!library) { error = dlerror(); return false; }
    auto create = reinterpret_cast<Cpu* (*)()>(dlsym(library, "create_cpu"));
    destroy = reinterpret_cast<void (*)(Cpu*)>(dlsym(library, "destroy_cpu"));
#endif
    if (!create || !destroy) { error = "Plugin is missing its factory functions"; return false; }
    instance = create();
    if (!instance) { error = "Plugin did not create a CPU"; return false; }
    return true;
}
