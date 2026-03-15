# OSSP (OpenSubsonicPlayer)

## AI Notice
This project does not have ANY AI code other than a few snippets that will be removed in the future.<br>
Yes I comment a LOT, and unfortunately this has become a red flag for excessive AI use.<br>
I am a forgetful person, and have a habit of dropping projects for months at a time, so having
many comments allows me to come back and instantly understand what I was doing.<br>

## Notice
OSSP is under heavy development and is NOT ready for daily use.<br>
Also, I am NOT RESPONSIBLE if you damage your hearing and/or any of your equipment using OSSP.<br>
OSSP does NOT apply ANY restrictions on the volume/bass levels and this can QUICKY lead to DAMAGED EQUIPMENT AND/OR HEARING.<br>
YOU ARE RESPONSIBLE FOR YOUR OWN CHOICES. Keep it safe, please, you cannot repair damaged hearing.<br>

## What is OSSP?
OSSP ('OpenSubsonic Player') is a music player application for UNIX®/Unix-like Operating Systems.<br>
It's features include:
- Support for a great deal of music formats (Except OGG as of now...)
- Comprehensive error handling
- Scrobbling to both ListenBrainz and LastFM (Even at the same time)
- A comprehensive JSON configuration scheme
- Advanced audio pipelining, allowing control of EQ, Pitch, and Reverberation
- Fast and responsive
- Low memory usage (No electron here!!)
- Advanced Security Handling of Stored Credentials and Web Requests (In the future)

## Architecture
At the moment, OSSP has a fairly unique architecture. It's frontend client (the QT interface) and the actual music player run as completely
separate applications and use IPC (Specifically Unix Sockets) to communicate using a custom (but relatively simple) protocol.<br>
This means that if you wanted to develop a TUI interface for OSSP, and have the program run completely with no window manager / desktop environment,
that is completely possible.

## Building
Please look at the ```building``` folder for more information. Due to certain configuration choices made by package maintainers, building OSSP is not exactly easy, although most of the process for supported platforms is automated, and the dependency tree is kept completely separate from system packages to avoid conflicts.<br>

OSSP has support for the following operating systems:
- Linux (GLibc / musl, x86_64 / aarch64)
  - musl needs a cryptography patch, not upstreamed
- NetBSD (10.1, x86_64)
  - LSP Plugins needs to be modified to build on NetBSD
- OpenBSD (7.8, x86_64)
  - Many, many patches needed to dependencies
- iOS / macOS
- 32-bit platforms currently have major issues, and big-endian platforms will not work

OSSP itself is extremely portable, and should be portable to any UNIX®/Unix-like platform with only a little work.

## Information
The ```libopensubsonic``` library has been implemented as per the specification located at [OpenSubsonic Netlify](https://opensubsonic.netlify.app/docs/api-reference/)

