glibjsonrpc_sources = [
  'glib-jsonrpc-json.c',
  'glib-jsonrpc-server.c',
  'glib-jsonrpc-client.c']

subdir('json-glib')
incjsonglib = include_directories('json-glib')
glib_dep = dependency('glib-2.0')
gobject_dep = dependency('gobject-2.0')
libglibjsonrpc = static_library('glibjsonrpc',
                         glibjsonrpc_sources,
                         install : false,
                         include_directories : incjsonglib,
                         dependencies : [glib_dep, gobject_dep],
                        )
  
