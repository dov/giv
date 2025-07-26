# subprojects/packagefiles/nifty_clib/meson.build

project('nifty_clib', 'c', version : '1.0.0', default_options: ['c_std=c99'])

nifti_sources = [
  'niftilib/nifti1_io.c',
  'znzlib/znzlib.c',
]

nifti_headers = [
  'niftilib/nifti1_io.h',
  'niftilib/nifti1.h',
  'znzlib/znzlib.h',
]

zlib_dep = dependency('zlib', required: true)

libnifti = library(
  'niftiio',
  nifti_sources,
  include_directories: include_directories('niftilib', 'znzlib'),
  dependencies: [zlib_dep],
  install: true,
  version: '1.0.0'
)

nifti_dep = declare_dependency(
  include_directories: include_directories('niftilib', 'znzlib'),
  link_with: libnifti,
  sources: nifti_headers,
  dependencies: [zlib_dep],
)

# Provide the dependency to parent project
meson.override_dependency('nifti', nifti_dep)
