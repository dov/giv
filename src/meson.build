gob2 = find_program('gob2')

PACKAGE_DOC_DIR = '../doc'
PACKAGE_PLUGIN_DIR = '../plugins'

add_global_arguments(['-DPACKAGE_DOC_DIR="'+PACKAGE_DOC_DIR+'" ',
                      '-DPACKAGE_PLUGIN_DIR="'+PACKAGE_PLUGIN_DIR+'" '],
                     language : 'cpp')
add_global_arguments(['-DPACKAGE_DOC_DIR="'+PACKAGE_DOC_DIR+'" ',
                      '-DPACKAGE_PLUGIN_DIR="'+PACKAGE_PLUGIN_DIR+'" '],
                     language : 'c')

giv_widget_src = custom_target('gen-output',
  output : ['giv-widget.h','giv-widget-private.h','giv-widget.cc'],
  input : 'giv-widget.gob',
  command : [gob2, '-o', '@OUTDIR@', '--for-cpp', '@INPUT@'],
  )

giv_widget_sources = [
  giv_widget_src[2],
  'colormaps.cc',
  'WordBoundaries.cc',
  'giv-parser.cc',
  'GivPainterAgg.cc',
  'GivPainterCairo.cc',
  'GivPainterCairoPixbuf.cc',
  'GivRenderer.cc',
  'GivStringArray.c',
  'giv-data.cc',
  'giv-markup.cc',
  'giv_agg_arrowhead.cc',
  'giv-settings.cc',
  'giv-settings-editor.cc',
  'clipper.cpp'
]

subdir('agg')
subdir('plis')
subdir('glib-jsonrpc')
subdir('gtkimageviewer')

incagg = include_directories('agg')
incgtkimageviewer = include_directories('agg')

gtk_dep = dependency('gtk+-3.0')
gmodule_dep = dependency('gmodule-2.0')
libgivwidget = shared_library(
  'giv-widget',
  giv_widget_sources,
  include_directories : [
    incagg,
    incgtkimageviewer
  ],
  link_with : [
    libagg,
    libplis,
    libgtkimageviewer
  ],
  dependencies : [gtk_dep],
  install : true,
)

libgiv_image_src = ['givimage.c',
                    'givplugin.c']

libgiv_image = shared_library(
  'giv-image',
  libgiv_image_src,
  dependencies : [
    gtk_dep,
    gmodule_dep],
  install : true
)

subdir('plugins')

giv_src = ['giv.cc',
           'dovtk-lasso.c',
           'giv-color-map.vala']

foreach basename : ['giv-win',
            'giv-mark-tree-dialog',
            'giv-info',
            'giv-contrast',
            'giv-histo',
            'giv-calibrate-dialog',
            'giv-settings',
            'giv-settings-editor',
            ] 
  giv_src += custom_target(basename + '-output',
    output : [basename + '.h',basename + '-private.h',basename + '.cc'],
    input : basename +'.gob',
    command : [gob2, '-o', '@OUTDIR@', '--for-cpp', '@INPUT@'],
    )[2]
endforeach

glib_dep = dependency('glib-2.0')
gobject_dep = dependency('gobject-2.0')
gio_dep = dependency('gio-2.0')
incglibjsonrpc = include_directories('glib-jsonrpc')

giv = executable('giv',
                 giv_src,
                 link_with : [
                   libgivwidget,
                   libplis,
                   libgtkimageviewer,
                   libglibjsonrpc,
                   libjsonglib,
                   libgiv_image
                 ],
                 include_directories : incglibjsonrpc,
                 dependencies : [gtk_dep],
                )

giv_remote_client = executable('giv-remote-client',
                               'giv-remote-client.cc',
                               link_with : [libglibjsonrpc, libjsonglib],
                               dependencies : [glib_dep,
                                               gobject_dep,
                                               gio_dep],
                               include_directories : incglibjsonrpc,
                              )

