inc = ['..']

png_dep = dependency('libpng')
zip_dep = dependency('libzip')
tiff_dep = dependency('libtiff-4')

foreach plugin_dep : [['npy', 'npy.c',  []],
                      ['tiff','tiff.c', [tiff_dep]],
                      ['png', 'png.c',  [png_dep]],
                      ['ora', 'ora.cc', [zip_dep, png_dep]]]
  shared_library(plugin_dep[0],
                 plugin_dep[1],
                 link_with:[libgiv_image],
                 dependencies : [plugin_dep[2],gtk_dep],
                 install_dir : 'lib/giv-1.0/plugins'
                )
endforeach
                        
                      
