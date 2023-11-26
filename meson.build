# Work in progress
project('giv', ['c','cpp','vala'],
  version : '0.9.38',
  license: 'LGPLv3+')

add_global_arguments('-DSPDLOG_FMT_EXTERNAL',
                     language : 'cpp')

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
