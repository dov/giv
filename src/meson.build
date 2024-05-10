gob2 = gob2_proj.get_variable('gob2_exe')
pymod = import('python')

python = pymod.find_installation()

# Can't use the gob2 generator for giv-widget.gob because
# it hides the include file, which we need below. So we
# must use a custom_target.
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
  'clipper.cpp',
  'SutherlandHodgmanPolygonClipping.cc'
]

subdir('agg')
subdir('glib-jsonrpc')
subdir('gtkimageviewer')

incagg = include_directories('agg')
incgtkimageviewer = include_directories('agg')

gtk_dep = dependency('gtk+-3.0')
gmodule_dep = dependency('gmodule-2.0')

deps = [gtk_dep, fmt_dep, libplis_dep, glm_dep, spdlog_dep, gmodule_dep]

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
  dependencies : deps,
  install : true,
  cpp_args : [
    '-Wno-deprecated-declarations'
  ],
  name_prefix : name_prefix
)

libgiv_image_src = ['givimage.cc',
                    'givplugin.cc',
                    'givimagedll.cc'
                   ]

libgiv_image = shared_library(
  'giv-image',
  libgiv_image_src,
  dependencies : deps,
  install : true,
  name_prefix : name_prefix
)

subdir('plugins')

libgiv_color = static_library(
  'giv-color-map',
  ['giv-color-map.vala'],
  dependencies : [gtk_dep],
  c_args : [
    '-Wno-deprecated-declarations',
    '-Wno-discarded-qualifiers',
    '-Wno-unused-but-set-variable',
    '-Wno-unused-variable'
  ]
)

giv_src = ['giv.cc',
           'dovtk-lasso.c']

gob_gen = generator(gob2,
                    output : ['@BASENAME@.h',
                              '@BASENAME@-private.h',
                              '@BASENAME@.cc'],
                    arguments : ['-o','@BUILD_DIR@','--for-cpp', '@INPUT@'])
                     
giv_src += gob_gen.process(                    
  'giv-win.gob',
  'giv-mark-tree-dialog.gob',
  'giv-info.gob',
  'giv-contrast.gob',
  'giv-histo.gob',
  'giv-calibrate-dialog.gob',
  'giv-settings.gob',
  'giv-settings-editor.gob')

file2c = find_program('file2c.py')

foreach src_trg : [
  ['menu-top-xml.i','menu-top.xml'],
  ['menu-popup-xml.i','menu-popup.xml'],
  ['copyright.i','copyright.markup']]

  target = src_trg[0]
  source = src_trg[1]
  giv_src += custom_target(
    target,
    output : [target],
    input : source,
    command : [file2c, '--source', '@INPUT@', '--target', '@OUTPUT@'],
    )
endforeach
  
gdk_pixbuf_csource = find_program('gdk-pixbuf-csource')

giv_src += custom_target(
  'giv-logo.i',
  output : ['giv-logo.i'],
  input : ['../doc/giv-logo.png'],
  capture : true,
  command : [gdk_pixbuf_csource,  '--name=image_giv_icon_inline', '@INPUT@']
)

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
                 dependencies : deps,
                 cpp_args : [
                   '-Wno-deprecated-declarations'] + cpp_args,
                 install : true,
                 gui_app : true
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

