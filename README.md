# OSSP (OpenSubsonicPlayer)

## Notice
OSSP is under heavy development and is NOT ready for daily use.<br>
Also, I am NOT RESPONSIBLE if you damage your hearing and/or any of your equipment using OSSP.<br>
OSSP does NOT apply ANY restrictions on the volume/bass levels and this can QUICKY lead to DAMAGED EQUIPMENT AND/OR HEARING.<br>
YOU ARE RESPONSIBLE FOR YOUR OWN CHOICES. Keep it safe, please, you cannot repair damaged hearing.<br>

## What is OSSP?
OSSP ('OpenSubsonic Player') is a music player application for UNIX®/Unix-like Operating Systems.<br>
It's features include:
- Support for a great deal of music formats
- Comprehensive error handling
- Scrobbling to ListenBrainz and LastFM
- A comprehensive configuration scheme
- Advanced audio pipelining, allowing control of EQ, Pitch, and Reverberation
- Fast and responsive
- Advanced Security Handling of Stored Credentials and Web Requests

OSSP has been privately tested under:
- Linux glibc / musl / x86_64 / aarch64
- NetBSD x86_64
- OpenBSD x86_64
- iOS / macOS Catalyst aarch64
- 32-bit platforms have major issues at this time

OSSP itself is extremely portable, and should be usable on any UNIX®/Unix-like platform with little to no patching, but the Gstreamer player logic is less portable (Specifically LSP Plugins).

## Information
The ```libopensubsonic``` library has been implemented as per the specification located at [OpenSubsonic Netlify](https://opensubsonic.netlify.app/docs/api-reference/)

