# Work in progress
project('giv', ['c','cpp','vala'],
  version : '0.1.0',
  license: 'LGPLv3+')
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
