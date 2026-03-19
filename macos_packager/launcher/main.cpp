/*
 * OpenSubSonicPlayer Launcher
 * Goldenkrew3000 2026
 * License: GNU General Public License 3.0
 * Darwin XNU / macOS OSSP Bundle Launcher
 */

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
    printf("[OSSPL] Bundle Path: %s\n", bundle_path.c_str());

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

    // Make new environment
    char envp0[PATH_MAX];
    char envp1[PATH_MAX];
    char envp2[PATH_MAX];
    char envp3[PATH_MAX];

    strncpy(envp0, c_dyld_library_path, PATH_MAX);
    strncpy(envp1, c_lv2_path, PATH_MAX);
    strncpy(envp2, c_home_env_b, PATH_MAX);
    strncpy(envp3, c_pwd_env, PATH_MAX);
    free(c_dyld_library_path);
    free(c_lv2_path);
    free(c_home_env_b);
    free(c_pwd_env);

    // NOTE: I have to package the environment variables this way because if they are set through **environ,
    // macOS sanitizes it and OSSP doesn't get it
    char* envp[] = {
        envp0,
        envp1,
        envp2,
        envp3,
        NULL
    };

    // Print new environment to console
    printf("[OSSPL] New environment:\n");
    printf("envp0: %s\n", envp[0]);
    printf("envp1: %s\n", envp[1]);
    printf("envp2: %s\n", envp[2]);
    printf("envp3: %s\n", envp[3]);

    // Create OSSP path and run
    std::string ossp_path = macho_path;
    removePath(ossp_path);
    ossp_path += "/ossp";
    printf("OSSP Path: %s\n", ossp_path.c_str());
    execve(ossp_path.c_str(), argv, envp);

    return 0;
}