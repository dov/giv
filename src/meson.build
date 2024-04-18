gob2 = find_program('gob2')

PACKAGE_DOC_DIR = '../doc'
PACKAGE_PLUGIN_DIR = '/usr/local/lib/giv-1.0/plugins/'

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
  'clipper.cpp',
  'SutherlandHodgmanPolygonClipping.cc'
]

subdir('agg')
#subdir('plis')
subdir('glib-jsonrpc')
subdir('gtkimageviewer')

incagg = include_directories('agg')
incgtkimageviewer = include_directories('agg')

gtk_dep = dependency('gtk+-3.0')
fmt_dep = dependency('fmt')
spdlog_dep = dependency('spdlog')
gmodule_dep = dependency('gmodule-2.0')
libplis_dep = dependency('plis')

libgivwidget = shared_library(
  'giv-widget',
  giv_widget_sources,
  include_directories : [
    incagg,
    incgtkimageviewer
  ],
  link_with : [
    libagg,
    libgtkimageviewer
  ],
  dependencies : [gtk_dep,fmt_dep, libplis_dep],
  install : true,
  cpp_args : [
    '-Wno-deprecated-declarations'
  ]
    
)

libgiv_image_src = ['givimage.cc',
                    'givplugin.cc',
                    'givimagedll.cc'
                   ]

libgiv_image = shared_library(
  'giv-image',
  libgiv_image_src,
  dependencies : [
    gtk_dep,
    gmodule_dep,
    fmt_dep,
    spdlog_dep],
  install : true
)

subdir('plugins')

libgiv_color = static_library(
  'giv-color-map',
  ['giv-color-map.vala'],
  dependencies : [gtk_dep])

giv_src = ['giv.cc',
           'dovtk-lasso.c']

foreach basename : [
  'giv-win',
  'giv-mark-tree-dialog',
  'giv-info',
  'giv-contrast',
  'giv-histo',
  'giv-calibrate-dialog',
  'giv-settings',
  'giv-settings-editor',
  ] 
  giv_src += custom_target(
    basename + '.cc',
    output : [basename + '.h',basename + '-private.h',basename + '.cc'],
    input : basename +'.gob',
    command : [gob2, '-o', '@OUTDIR@', '--for-cpp', '@INPUT@'],
    )[2]
endforeach

glib_dep = dependency('glib-2.0')
gobject_dep = dependency('gobject-2.0')
gio_dep = dependency('gio-2.0')
incglibjsonrpc = include_directories('glib-jsonrpc')

# Config variables
arch = 'x86_64 GNU/Linux'

r = run_command('git', 'rev-parse', 'HEAD', check: true)
if r.returncode() != 0
  # it failed
endif
sha1 = r.stdout().strip()
# meson version 0.56 will have substr()
sha1_short = sha1[0]+sha1[1]+sha1[2]+sha1[3]+sha1[4]+sha1[5]

r = run_command('git', 'log', '--pretty=%ci', '-n1', check: true)
if r.returncode() != 0
  # it failed
endif
commit_time=r.stdout().strip()

cpp_args = ['-DARCH="' + arch+ '"',
            '-DGIT_COMMIT_ID="'+sha1+'"',
            '-DGIT_COMMIT_TIME="'+commit_time+'"',
            '-DVERSION="'+meson.project_version()+'"'
           ]


giv = executable('giv',
                 giv_src,
                 link_with : [
                   libgivwidget,
                   libgtkimageviewer,
                   libglibjsonrpc,
                   libjsonglib,
                   libgiv_image,
                   libgiv_color,
                 ],
                 include_directories : incglibjsonrpc,
                 dependencies : [gtk_dep, fmt_dep, spdlog_dep, libplis_dep],
                 cpp_args : [
                   '-Wno-deprecated-declarations'] + cpp_args,
                 install : true
                )

giv_remote_client = executable('giv-remote-client',
                               'giv-remote-client.cc',
                               link_with : [libglibjsonrpc, libjsonglib],
                               dependencies : [glib_dep,
                                               gobject_dep,
                                               gio_dep],
                               include_directories : incglibjsonrpc,
                               install : true,
                              )

