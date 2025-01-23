project('giv', ['c','cpp','vala'],
  version : '0.9.38',
  license: 'LGPLv3+')

name='giv'
name_cap = 'Giv'
version = meson.project_version()

PACKAGE_DOC_DIR = get_option('prefix') / 'share/doc/giv'
PACKAGE_PLUGIN_DIR = get_option('prefix') / 'lib/giv-1.0/plugins/'

add_global_arguments('-DSPDLOG_FMT_EXTERNAL',
                     language : 'cpp')

add_global_arguments(['-DPACKAGE_DOC_DIR="'+PACKAGE_DOC_DIR+'" ',
                      '-DPACKAGE_PLUGIN_DIR="'+PACKAGE_PLUGIN_DIR+'" '],
                     language : 'cpp')
add_global_arguments(['-DPACKAGE_DOC_DIR="'+PACKAGE_DOC_DIR+'" ',
                      '-DPACKAGE_PLUGIN_DIR="'+PACKAGE_PLUGIN_DIR+'" '],
                     language : 'c')

gob2_proj = subproject('gob2')
libplis_dep = subproject('libplis',
                         default_options: ['default_library=static']).get_variable('libplis_dep')
spdlog_dep = dependency('spdlog', version : '>=1.14.1', fallback : ['spdlog', 'spdlog_dep'])
fmt_dep = dependency('fmt', version : '>=9.0.0', fallback : ['fmt', 'fmt_dep'])

glm_dep = dependency('glm')
if not glm_dep.found()
  glm_dep = glm_proj.get_variable('glm_dep')
endif

expat_dep = dependency('expat')

gnome = import('gnome')
pkgconfig = import('pkgconfig')

if host_machine.system() == 'windows'
  name_prefix = ''
else
  name_prefix = 'lib'
endif

subdir('src')
subdir('doc')

pkgconfig.generate(libgivwidget,
  name: 'givwidget',
  description: 'A library for giv as a widget',
  version: meson.project_version(),
  requires: ['gtk-image-viewer','fmt'],
  filebase: 'giv',
)

fs = import('fs')
builddir = fs.name(meson.current_build_dir())
buildroot = fs.name(meson.current_source_dir())
if host_machine.system() == 'windows'
  arch = 'x86_64-w64-mingw32'
  host = 'w64'

  nsis_cmd = [
    find_program('makensis'),
    '-DOUTDIR='+builddir,
    '-DICON_DIR=.',
    '-DCOMMITID_SHORT='+sha1_short,
    '-DNAME='+name,
    '-DNAME_CAP='+name_cap,
    '-DICON_NAME='+name + '_logo',
    '-DARCH='+arch,
    '-DHOST='+host,
    '-DHOSTBITS=64',
    '-DVERSION='+ version,
    '-DLIBGCCDLL=libgcc_s_seh-1.dll',
    '-DSYSROOT=\\usr\\'+arch+'\\sys-root',
    '@INPUT@',
  ]

  nsis = custom_target('nsis',
                       output: 'install-giv.exe',
                       input: files('giv.nsi'),
                       build_always_stale: true,
                       command: nsis_cmd,
                       depends: [giv, giv_remote_client]
                       )
  alias_target('installer', nsis)
endif
