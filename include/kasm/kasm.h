#ifndef KASM_KASM_H
#define KASM_KASM_H

#include "kasm/version.h"

#ifdef __cplusplus
extern "C" {
#endif

/** Return the version string generated from the CMake project version. */
const char *kasm_version(void);

#ifdef __cplusplus
}
#endif

#endif
