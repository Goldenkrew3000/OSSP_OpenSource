#!/bin/bash
# Exit on any error
set -e

# OSSP macOS Sysroot Build Automation
# Goldenkrew / Hojuix 2026
# GNU General Public License 3

# Settings
export OSSP_NPROC=4
export OSSP_SYSROOT=/opt/ossp
export OSSP_BUILDDIR=/opt/ossp_build

# Make directories
echo Sudo is required to make Sysroot and Build folder in /opt, and to set correct permissions
sudo mkdir ${OSSP_SYSROOT}
sudo mkdir ${OSSP_BUILDDIR}
sudo chown -R $(whoami) ${OSSP_SYSROOT}
sudo chown -R $(whoami) ${OSSP_BUILDDIR}

# Fetch PCRE2
cd ${OSSP_BUILDDIR}
mkdir pcre2 && cd pcre2
wget https://github.com/PCRE2Project/pcre2/releases/download/pcre2-10.47/pcre2-10.47.tar.gz
tar -xvf pcre2-10.47.tar.gz
cd pcre2-10.47
mkdir build && cd build
cmake -DCMAKE_INSTALL_PREFIX=${OSSP_SYSROOT} ..
make -j${OSSP_NPROC}
make install

# Set PkgConfig Path
export PKG_CONFIG_PATH=${OSSP_SYSROOT}/lib/pkgconfig

# Fetch Glib
cd ${OSSP_BUILDDIR}
mkdir glib && cd glib
wget https://download.gnome.org/sources/glib/2.87/glib-2.87.3.tar.xz
tar -xvf glib-2.87.3.tar.xz
cd glib-2.87.3
meson setup --prefix=${OSSP_SYSROOT} --wipe build
cd build
ninja
ninja install

# GStreamer requires glib-mkenums in path, otherwise it builds its own Glib, which causes a duplicate conflict
export PATH=$PATH:${OSSP_SYSROOT}/bin

# Fetch Lzo
cd ${OSSP_BUILDDIR}
mkdir lzo && cd lzo
wget https://www.oberhumer.com/opensource/lzo/download/lzo-2.10.tar.gz
tar -xvf lzo-2.10.tar.gz
cd lzo-2.10
mkdir build && cd build
cmake -DCMAKE_INSTALL_PREFIX=${OSSP_SYSROOT} -DCMAKE_POLICY_VERSION_MINIMUM=3.5 ..
make -j${OSSP_NPROC}
make install

# Patch LZO PkgConfig for Cairo
# Cairo uses both <lzo/*> and <lzo2a.h> includes, but LZO's PkgConfig only allows the use of the latter.
# For an upstream patch, you would patch Cairo instead, but this is an enclosed build system, and this is easier
# lzo2.pc: 'Cflags: -I${includedir}/lzo' -> 'Cflags: -I${includedir}/lzo -I${includedir}'
gsed -i '/^Cflags/c Cflags: -I${includedir}/lzo -I${includedir}' ${OSSP_SYSROOT}/lib/pkgconfig/lzo2.pc

# Fetch Cairo
cd ${OSSP_BUILDDIR}
mkdir cairo && cd cairo
wget https://cairographics.org/releases/cairo-1.18.4.tar.xz
tar -xvf cairo-1.18.4.tar.xz
cd cairo-1.18.4
meson setup --prefix=${OSSP_SYSROOT} -Dxlib=disabled -Dxcb=disabled --wipe build
cd build
ninja
ninja install

# Fetch OpenSSL
cd ${OSSP_BUILDDIR}
mkdir openssl && cd openssl
wget https://github.com/openssl/openssl/releases/download/openssl-3.6.1/openssl-3.6.1.tar.gz
tar -xvf openssl-3.6.1.tar.gz
cd openssl-3.6.1
./config --prefix=${OSSP_SYSROOT}
make -j${OSSP_NPROC}
make install

# Fetch Soundtouch
cd ${OSSP_BUILDDIR}
mkdir soundtouch && cd soundtouch
wget https://www.surina.net/soundtouch/soundtouch-2.4.0.tar.gz
tar -xvf soundtouch-2.4.0.tar.gz
cd soundtouch
mkdir build && cd build
cmake -DCMAKE_INSTALL_PREFIX=${OSSP_SYSROOT} ..
make -j${OSSP_NPROC}
make install

# Build LSP Plugins
cd ${OSSP_BUILDDIR}
mkdir lsp && cd lsp
wget https://github.com/lsp-plugins/lsp-plugins/releases/download/1.2.26/lsp-plugins-src-1.2.26.tar.gz
tar -xvf lsp-plugins-src-1.2.26.tar.gz
cd lsp-plugins
gmake clean
gmake config PREFIX=${OSSP_SYSROOT} FEATURES='lv2'
gmake -j${OSSP_NPROC}
gmake install

# Build Zix
cd ${OSSP_BUILDDIR}
mkdir zix && cd zix
git clone --depth=1 https://github.com/drobilla/zix
cd zix
meson setup --prefix=${OSSP_SYSROOT} --wipe build
cd build
ninja
ninja install

# Build Serd
cd ${OSSP_BUILDDIR}
mkdir serd && cd serd
git clone --depth=1 https://github.com/drobilla/serd
cd serd
meson setup --prefix=${OSSP_SYSROOT} --wipe build
cd build
ninja
ninja install

# Build Sord
cd ${OSSP_BUILDDIR}
mkdir sord && cd sord
git clone --depth=1 https://github.com/drobilla/sord
cd sord
meson setup --prefix=${OSSP_SYSROOT} --wipe build
cd build
ninja
ninja install

# Build LV2
cd ${OSSP_BUILDDIR}
mkdir lv2 && cd lv2
git clone --depth=1 https://github.com/lv2/lv2
cd lv2
meson setup --prefix=${OSSP_SYSROOT} --wipe build
cd build
ninja
ninja install

# Build Sratom
cd ${OSSP_BUILDDIR}
mkdir sratom && cd sratom
git clone --depth=1 https://gitlab.com/lv2/sratom
cd sratom
meson setup --prefix=${OSSP_SYSROOT} --wipe build
cd build
ninja
ninja install

# Build Lilv
cd ${OSSP_BUILDDIR}
mkdir lilv && cd lilv
wget https://github.com/lv2/lilv/archive/refs/tags/v0.26.4.tar.gz
tar -xvf v0.26.4.tar.gz
cd lilv-0.26.4
meson setup --prefix=${OSSP_SYSROOT} --wipe build
cd build
ninja
ninja install

# Build Bzip2
cd ${OSSP_BUILDDIR}
mkdir bzip2 && cd bzip2
git clone --depth=1 https://gitlab.com/federicomenaquintero/bzip2
cd bzip2
mkdir build && cd build
cmake -DCMAKE_INSTALL_PREFIX=${OSSP_SYSROOT} ..
make -j${OSSP_NPROC}
make install

# Build GStreamer
cd ${OSSP_BUILDDIR}
mkdir gstreamer && cd gstreamer
wget https://gitlab.freedesktop.org/gstreamer/gstreamer/-/archive/1.28.1/gstreamer-1.28.1.tar.gz
tar -xvf gstreamer-1.28.1.tar.gz
cd gstreamer-1.28.1
meson setup --prefix=${OSSP_SYSROOT} --wipe build
cd build
ninja
ninja install

echo Please remove /opt/ossp_build manually to remove build objects
