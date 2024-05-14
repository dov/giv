#!/usr/bin/env python3 

# Copy a file from the a build dir to the source dir

import os, sys, shutil
from pathlib import Path

# get absolute input and output paths
build_dir = Path(os.getenv('MESON_BUILD_ROOT')) / os.getenv('MESON_SUBDIR')
source_dir = Path(os.getenv('MESON_SOURCE_ROOT')) / os.getenv('MESON_SUBDIR')

fn = sys.argv[1]
input_path = build_dir / fn
output_path = source_dir / fn
print(f'{input_path=} {output_path=}')
shutil.copyfile(input_path, output_path)
