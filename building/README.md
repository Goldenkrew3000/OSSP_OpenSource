# Building OSSP (OpenSubsonicPlayer)

## OS-Agnostic Information (UNIX*, Win32 is NOT and WILL NOT EVER be supported)
OSSP

## macOS Information
As all of the dependencies officially support macOS, no patches are required.<br>
The dependency sysroot will be installed at ```/opt/ossp```<br>

To get started, first install the dependencies required for building with brew.<br>
Then, run the ```build_macos_sysroot.sh``` script to build and install the dependencies to ```/opt/ossp```<br>
Alternatively, you can extract ```macos_sysroot.tar.gz``` in ```/opt``` to use the precompiled sysroot.<br>

Required dependencies for building:
 - make (GNU Make)
 - sed (GNU Sed)
 - cmake
 - wget
 - pkgconfig
 - meson
 - ninja
 - sdl2
   - SDL2 is not used for OSSP except for the debug interface, which is not even functional in this environment. Unfortunately, it is still required for building at this time.

The dependencies that are built are:
 - (?) PCRE2
 - (?) Glib
 - (?) lzo
 - (?) cairo
 - (?) OpenSSL
 - (For GStreamer) Soundtouch
 - (For GStreamer) LSP Plugins
 - (For lilv) zix
 - (For lilv) serd
 - (For lilv) sord
 - (For lilv) lv2
 - (For lilv) sratom
 - (For GStreamer) lilv
 - (For GStreamer) libbz2
 - GStreamer (Builds static FFmpeg)

Building and Running OSSP:
(To put build instructions here TODO)

```DYLD_LIBRARY_PATH=/opt/ossp/lib LV2_PATH=/opt/ossp/lib/lv2 <OSSP MachO Executable>```
