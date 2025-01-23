# Background

Building giv isn't that easy, due to the large number of of dependencies, including some patches and some obsolete tools. Especially building it for windows is hard because of the cross compiling the dependencies.

It is my goal to simplify this in the future, but currently the goal of this document is to describe how to build.

The building steps below have been tested on Fedora, but they probably work with small modifications under any Linux system.

# Prerequisites

Both the linux and the cross compilation build require the following dependencies:

## On Ubuntu

- Run the following command
```
sudo apt install libgtk-3-dev bison flex pkg-config libtool \
valac scons libfmt-dev libzip-dev libspdlog-dev libcfitsio-dev \
libdcmtk-dev libpcre3-dev gcc g++ libsndfile-dev \
libglm-dev python3-pip cmake
```

## Ubuntu 20.04.4

Compiling under Ubuntu 20.04.4 requires updating meson. This may be e.g. be done as follows:

```
pip3 install --user meson==0.56.0
~/.local/bin/meson setup build
```

The rest of the build is equal to the Linux compilation below

# Linux

The prefered building system is meson under Linux. 

## Prerequisites for the plugins

- cfitsio-devel
- dcmtk
- libwebp

These are all available by dnf or apt. However, if the dependencies are not found, the plugins are skipped automatically during build.

## Building giv

After all the above plugin have been installed, giv may be built:

```
meson setup build
cd build
ninja
ninja install
```

# Windows

The windows build and installer are cross compiled with mingw libraries under Linux. The following are the requirements for building giv under Fedora:

```
sudo dnf -y install mingw64-filesystem
sudo dnf -y install mingw64-gcc
sudo dnf -y install mingw64-gcc-c++
sudo dnf -y install mingw32-nsis
sudo dnf -y install mingw64-pcre
sudo dnf -y install mingw64-gtk3
sudo dnf -y install mingw64-libzip
sudo dnf -y install mingw64-libwebp
```

The following libraries are not available as binary downloads and may be installed from source as follows described below:

## Cross compile with meson

```
env PKG_CONFIG_PATH= meson setup build_mingw64 --cross-file cross_mingw64.txt -Ddefault_library=shared -Dbuildtype=release
ninja -C build_mingw64

```
## Building Prerequisites

### libplis

```
git clone http://github.com/dov/libplis
# The following is needed to generate the "compile" file
./autogen.sh --prefix=/usr/local 
mingw64-configure --prefix=/usr/local/mingw64 --exec_prefix=/usr/local/mingw64 --libdir=/usr/local/mingw64/lib --includedir=/usr/local/mingw64/include
make install
```

### dcmtk

- TBD - (I got it to work by tweaking with the CMakeCache.txt file, but I don't have a reproducible method. Mail me for my CMakeCache.txt file if you want)


###  cfitsio
    - Using cmake toolchain
```
wget http://heasarc.gsfc.nasa.gov/FTP/software/fitsio/c/cfitsio-4.2.0.tar.gz
tar xf cfitsio-4.2.0.tar.gz
cd cfitsio-4.2.0
mkdir build_mingw64
cd build_mingw64
cmake -DCMAKE_TOOLCHAIN_FILE=/home/dov/git/dov-env/cmake/mingw-w64-x86_64.cmake -DCMAKE_INSTALL_PREFIX=/usr/local/mingw64 ..
```
   - This fails because of missing zlibrary
   - Edit `CMakeCache.txt`
   - Search for the missing library and change them, e.g.:
```
//Path to a file.
ZLIB_INCLUDE_DIR:PATH=/usr/x86_64-w64-mingw32/sys-root/mingw/include/

//Path to a library.
ZLIB_LIBRARY_DEBUG:FILEPATH=/usr/x86_64-w64-mingw32/sys-root/mingw/lib/libz.a

//Path to a library.
ZLIB_LIBRARY_RELEASE:FILEPATH=/usr/x86_64-w64-mingw32/sys-root/mingw/lib/libz.a
```
  - After this change, ~cmake~ may be run again and it worked

