plis_sources = [
'plis.cc'
]

pcre_dep = dependency('libpcre')
libplis = static_library('plis',
                         plis_sources,
                         dependencies : [pcre_dep],
                         install : false)
