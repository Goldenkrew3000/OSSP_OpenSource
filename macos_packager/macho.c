/*
 * OpenSubSonicPlayer Launcher
 * Goldenkrew3000 2026
 * License: GNU General Public License 3.0
 * Darwin XNU / macOS OSSP Bundle Launcher
 * INFO: macOS doesn't seem to allow usage of mach-o/dyld.h from C++
 */

#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <limits.h>
#include <mach-o/dyld.h>

int dyld_get_executable_path(char** path) {
    char* macho_path[PATH_MAX];
	uint32_t macho_path_sz = sizeof(macho_path);

	int dyld_rc = 0;
	dyld_rc = _NSGetExecutablePath(macho_path, &macho_path_sz);
	if (dyld_rc != 0) {
		return 1;
	}

    *path = malloc(PATH_MAX);
    if (*path == NULL) {
        return 1;
    }

    memcpy(*path, macho_path, strlen(macho_path));

    return 0;
}