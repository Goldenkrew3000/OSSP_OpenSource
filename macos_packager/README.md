# OSSP macOS Packaging Utilities

## What is ```launcher```
Launcher is simply a tool that launches OSSP with the correct environment variables when inside a macOS bundle.<br>
OSSP packages it's own runtime dependency tree with itself as to avoid dependency issues for the
end user. To achieve this though, some hacks have to be made.
OSSP needs some environment arguments to launch, such as ```DYLD_LIBRARY_PATH``` and ```LV2_PATH```
to use the correct sysroot, and for the equalizer functionality respectively.<br>
Normally, macOS sanitizes ```DYLD_*``` environment variables when launching a new program, so
```launcher``` was made to create an entirely new environment pointer for execve() to use.

## Notice
NOTE: This does NOT bypass and/or exploit any security features of macOS / Darwin. This is an intentional
feature, as when running 'protected' binaries (such as the ones found in ```/usr/bin```),
macOS goes even further to sanitize the ```DYLD_*``` environment variables even when utilizing this method.<br>
In saying that, there is ABSOLUTELY NO WAY this would EVER pass App Store verification.

## How to use this?
Firstly, build the launcher.<br>
Then, build OSSP.<br>
Then, copy the actual OSSP binary into the newly created bundle (Bundle/Contents/MacOS/).<br>
Then, copy the sysroot from /opt/ossp into the bundle (Bundle/Contents/Sysroot).<br>
Finally, make the .icns file for the app icon, and copy it into the bundle (Bundle/Contents/Resources).
