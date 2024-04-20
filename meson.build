project('giv', ['c','cpp','vala'],
  version : '0.9.38',
  license: 'LGPLv3+')

PACKAGE_DOC_DIR = 'doc'
PACKAGE_PLUGIN_DIR = get_option('prefix') + 'lib/giv-1.0/plugins/'

add_global_arguments('-DSPDLOG_FMT_EXTERNAL',
                     language : 'cpp')

add_global_arguments(['-DPACKAGE_DOC_DIR="'+PACKAGE_DOC_DIR+'" ',
                      '-DPACKAGE_PLUGIN_DIR="'+PACKAGE_PLUGIN_DIR+'" '],
                     language : 'cpp')
add_global_arguments(['-DPACKAGE_DOC_DIR="'+PACKAGE_DOC_DIR+'" ',
                      '-DPACKAGE_PLUGIN_DIR="'+PACKAGE_PLUGIN_DIR+'" '],
                     language : 'c')

gob2_proj = subproject('gob2')

libsimple_dep = dependency(
  'libplis',
  fallback : ['libplis', 'libplis_dep'],
  default_options: ['default_library=static']
)

gnome = import('gnome')
pkgconfig = import('pkgconfig')

subdir('src')

pkgconfig.generate(libgivwidget,
  name: 'givwidget',
  description: 'A library for giv as a widget',
  version: meson.project_version(),
  requires: ['gtk-image-viewer','fmt'],
  filebase: 'giv',
)
