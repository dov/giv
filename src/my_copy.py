#!/usr/bin/env python3 

import os
from pathlib import Path

# Use the modernized Meson environment variables
build_dir = Path(os.getenv('MESON_BUILD_DESTDIR', os.getenv('MESON_DIST_ROOT', ''))) 
# Usually, for a script run during build, you want:
build_dir = Path(os.getenv('MESON_BUILD_ROOT', os.getenv('MESON_PRIVATE_DIR', '')))

# The current standard for paths:
build_root = Path(os.getenv('MESON_BUILD_ROOT'))
source_root = Path(os.getenv('MESON_SOURCE_ROOT'))
subdir = os.getenv('MESON_SUBDIR', '')

build_dir = build_root / subdir
source_dir = source_root / subdir
