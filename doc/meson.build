# Custom target definition
eperl = files('eperl')

install_dir = 'share/doc/giv'

custom_target('giv.phtml',
  input: 'giv.phtml',
  output: 'giv.html',
  command: ['perl', eperl, '-I', '${MESON_SOURCE_DIR}', '@INPUT@'],
  capture: true,
  build_by_default : true,
  install : true,
  install_dir : install_dir
)

image_files = [
  'alpha-example.jpg',
  'example.giv',
  'gallery-cat-contour-sub-pixel.jpg',
  'gallery-cat-contour-sub-pixel.png',
  'gallery-cat-contour.jpg',
  'gallery-cat-pseudo-color.jpg',
  'gallery-convex-hull.jpg',
  'gallery-convex-hull.png',
  'gallery-europe-zoom.jpg',
  'gallery-europe.jpg',
  'gallery-quiver.jpg',
  'gallery-voronoi.png',
  'giv-calib-and-measure.png',
  'giv-logo-150.png',
  'giv-logo.png',
  'giv-m51-with-contrast-tool-screenshot.png',
  'giv-maja.jpg',
  'giv-zoom-in.jpg',
  'maja.png',
  'marks-view-1.png',
  'marks-view-2.png',
  'pregnant-bitch-nomanip.png',
  'pregnant-bitch.png',
  'style-ex.giv',
  'svg-marks-example.png',
  'text-align-screenshot.png',
  'text-align.giv',
  'pick-coordinate.py',
]

install_data(image_files,
             install_dir : install_dir)
