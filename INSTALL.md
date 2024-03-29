# Background

Building giv isn't that easy, due to the large number of of dependencies, including some patches and some obsolete tools. Especially building it for windows is hard because of the cross compiling the dependencies.

It is my goal to simplify this in the future, but currently the goal of this document is to describe how to build.

The building steps below have been tested on Fedora, but they probably work with small modifications under any Linux system.

# Prerequisites

Both the linux and the cross compilation build require the following dependencies:

- https://github.com/dov/gob2/tree/cpp-new - The `cpp-new` branch. 
- vala - "Any" resent version will do
- scons 

# Linux

The prefered building system is scons under Linux. automake is currently broken, and I started porting to meson, but I haven't finished it. 

## Prerequisites for giv

### libplis

This is a small c++ string library that I have been using for various projects. To install do:

```
git clone http://github.com/dov/libplis
cd libplis
./autogen.sh --prefix=/usr/local; make -j 8 install
```

## Prerequisites for the plugins

- cfitsio-devel
- dcmtk
- libwebp

These are all available by dnf

## Building giv

Giv may be built:

To build giv do in the top directory

```
scons -j 8 
scons -j 8 install
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

### fmt
    - Compilation:
```
unzip  ~/hd/Download/fmt-9.1.0.zip
cd fmt-9.1.0
mkdir build_mingw64
cd build_mingw64
cmake -DFMT_TEST=OFF -DCMAKE_TOOLCHAIN_FILE=/home/dov/git/dov-env/cmake/mingw-w64-x86_64.cmake -DCMAKE_INSTALL_PREFIX=/usr/local/mingw64 ..
```

### glm
    - Currently the main branch is broken with regards to installation. The following pull request fixes it. Use this glm version:
    - https://github.com/g-truc/glm/pull/1117
    - Compilation

```
wget https://github.com/Tachi107/glm/archive/refs/heads/cmake-install-improvements.zip
unzip cmake-install-improvements.zip
cd glm-cmake-install-improvements
mkdir build_mingw64
cd build_mingw64
cmake -DGLM_TEST_ENABLE=OFF -DCMAKE_TOOLCHAIN_FILE=/home/dov/git/dov-env/cmake/mingw-w64-x86_64.cmake -DCMAKE_INSTALL_PREFIX=/usr/local/mingw64 ..
make install
```
