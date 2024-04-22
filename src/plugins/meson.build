inc = ['..']

png_dep = dependency('libpng')
zip_dep = dependency('libzip')
tiff_dep = dependency('libtiff-4')

plugins = [['npy', 'npy.c',  []],
           ['tiff','tiff.c', [tiff_dep]],
           ['png', 'png.c',  [png_dep]],
           ['ora', 'ora.cc', [zip_dep, png_dep]]]

cfitsio_dep = dependency('cfitsio', required : false)
if cfitsio_dep.found()
    plugins += [['fits','fits.c', [cfitsio_dep]]]
endif

dcmtk_dep = dependency('dcmtk', required : false)
if dcmtk_dep.found()
    plugins += [['dcm','dicom.cc', [dcmtk_dep]]]
endif

webp_dep = dependency('libwebp', required : false)
if webp_dep.found()
    plugins += [['webp','webp.cc', [webp_dep]]]
endif

foreach plugin_dep : plugins
  shared_library(plugin_dep[0],
                 plugin_dep[1],
                 link_with:[libgiv_image],
                 include_directories : inc,
                 dependencies : [plugin_dep[2],gtk_dep, libplis_dep],
                 install_dir : 'lib/giv-1.0/plugins',
                 install : true
                )
endforeach
                        
                      
