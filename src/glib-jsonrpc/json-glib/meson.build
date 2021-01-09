jsonglib_sources = [
  'json-array.c',
  'json-builder.c',
  'json-debug.c',
  'json-gboxed.c',
  'json-generator.c',
  'json-gobject.c',
  'json-gvariant.c',
  'json-node.c',
  'json-object.c',
  'json-parser.c',
  'json-path.c',
  'json-reader.c',
  'json-scanner.c',
  'json-serializable.c',
  'json-utils.c',
  'json-value.c']

glib_dep = dependency('glib-2.0')
inc = include_directories('..')

libjsonglib = static_library('jsonglib',
                      jsonglib_sources,
                      c_args: '-DJSON_COMPILATION',
                      include_directories: inc,
                      dependencies : [glib_dep],
                     )

