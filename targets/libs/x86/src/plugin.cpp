#include "x86/x86.hpp"

#ifdef _WIN32
#define PLUGIN_EXPORT __declspec(dllexport)
#else
#define PLUGIN_EXPORT __attribute__((visibility("default")))
#endif

extern "C" PLUGIN_EXPORT Cpu* create_cpu() { return new X86(); }
extern "C" PLUGIN_EXPORT void destroy_cpu(Cpu* cpu) { delete cpu; }