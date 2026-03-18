/*
 * OpenSubSonicPlayer Launcher
 * Goldenkrew3000 2026
 * License: GNU General Public License 3.0
 * Darwin XNU / macOS OSSP Bundle Launcher
 */

// Yes I know this is some of the worst written code, like ever
// I will clean this up later, it just has to work

#include <iostream>
#include <string>
#include <sstream>
#include <cstring>
#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include "macho.h"

void removePath(std::string& str) {
    size_t pos = str.find_last_of('/');
    if (pos != std::string::npos) {
        str.erase(pos);
    }
}

int main(int argc, char** argv) {
    printf("OSSP Launcher 1.0a\n");

    // Get location of the MachO executable
    int rc = 0;
    char* c_macho_path = NULL;
    rc = dyld_get_executable_path(&c_macho_path);
    printf("MachO Path: %s\n", c_macho_path);
    std::string macho_path(c_macho_path);
    free(c_macho_path);

    // Get path of Bundle
    std::string bundle_path = macho_path;
    removePath(bundle_path);
    removePath(bundle_path);
    removePath(bundle_path);
    printf("Bundle Path: %s\n", bundle_path.c_str());

    // Create DYLD_LIBRARY_PATH - 'Bundle/sysroot/lib'
    std::string dyld_library_path = bundle_path;
    dyld_library_path += "/sysroot/lib";
    dyld_library_path = "DYLD_LIBRARY_PATH=" + dyld_library_path;
    char* c_dyld_library_path = strdup(dyld_library_path.c_str());

    // Create LV2_PATH - 'Bundle/sysroot/lib/lv2'
    std::string lv2_path = bundle_path;
    lv2_path += "/sysroot/lib/lv2";
    lv2_path = "LV2_PATH=" + lv2_path;
    char* c_lv2_path = strdup(lv2_path.c_str());

    // Move current HOME env variable to new env
    char* c_home_env = getenv("HOME");
    std::string home_env(c_home_env);
    std::string home_env_orig = home_env;
    home_env = "HOME=" + home_env;
    char* c_home_env_b = strdup(home_env.c_str());

    // Make new PWD env variable
    std::string pwd_env = home_env_orig + "/.config/ossp";
    pwd_env = "PWD=" + pwd_env;
    char* c_pwd_env = strdup(pwd_env.c_str());
    printf("New PWD: %s\n", c_pwd_env);

    char test[256];
    strcpy(test, c_dyld_library_path);
    char test2[256];
    strcpy(test2, c_lv2_path);
    char test3[256];
    strcpy(test3, c_home_env_b);
    char test4[256];
    strcpy(test4, c_pwd_env);
    free(c_dyld_library_path);
    free(c_lv2_path);
    free(c_home_env_b);
    free(c_pwd_env);

    // NOTE: I have to package the environment variables this way because if they are set through **environ,
    // macOS sanitizes it and OSSP doesn't get it
    char* envp[] = {
        test3,
        test,
        test2,
        test4,
        NULL
    };

    printf("--------------------------------\n");
    printf("%s\n", envp[0]);
    printf("%s\n", envp[1]);
    printf("%s\n", envp[2]);
    printf("%s\n", envp[3]);
    printf("--------------------------------\n");

    // Create OSSP path and run
    std::string ossp_path = macho_path;
    removePath(ossp_path);
    ossp_path += "/ossp";
    printf("OSSP Path: %s\n", ossp_path.c_str());
    execve(ossp_path.c_str(), argv, envp);

    return 0;
}