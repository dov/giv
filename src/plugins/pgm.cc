//======================================================================
//  pgm.c - A dummy pgm loader.
//
//  Dov Grobgeld <dov.grobgeld@gmail.com>
//  Sun Nov  8 21:42:35 2009
//----------------------------------------------------------------------

#include "../givimage.h"
#include "../givplugin.h"
#include <glib.h>
#include <math.h>
#include <stdint.h>
#include <string>
#include <fstream>
#include <fmt/core.h>

using namespace std;

static giv_plugin_support_t pgm_support = {
    TRUE,
    0,
    "P5",
    TRUE,
    "pgm"
};

giv_plugin_support_t giv_plugin_get_support()
{
    return pgm_support;
}

extern "C" gboolean giv_plugin_supports_file(const char *filename,
                                  guchar *start_chunk,
                                  gint start_chunk_len)
{
  return filename[0] != 0 && (g_str_has_suffix(filename, ".pgm")
                              || g_str_has_suffix(filename, ".PGM"));
}

extern "C" GivImage *giv_plugin_load_file(const char *filename)
{
  int width, height;
  string line;

  // open a pgm file with fstream and read its type, assert
  // that it is a P5 file, then read the width and height
  // and the bit depth of the image and assign it to
  // GivImageType image_type; and let it be either GIVIMAGE_U8
  // or GIVIMAGE_U16 depending on the bit depth of the image.
  // Then  read the pixels and assign them to the image
  // img->buf.buf. Finally return the image.

  fstream file(filename, ios::in | ios::binary);
  if (!file.is_open())
    return NULL;

  getline(file, line);
  if (line != "P5")
    return NULL;

  // skip comments
  while (file.peek() == '#')
    getline(file, line);

  file >> width >> height;
  file.ignore(1);

  int max_val;
  GivImageType image_type;
  file >> max_val;
  file.ignore(1);
  if (max_val == 65535)
    image_type = GIVIMAGE_U16;
  else if (max_val == 255)
    image_type = GIVIMAGE_U8;
  else
    return NULL;

  GivImage *img = giv_image_new(image_type, width, height);
  if (img == NULL)
    return NULL;

  if (image_type == GIVIMAGE_U8)
    file.read((char*)img->buf.buf, width*height);
  else
    {
      uint16_t *buf = (uint16_t*)img->buf.buf;
      for (int i = 0; i < width*height; i++)
        {
          uint16_t val;
          file.read((char*)&val, 2);
          // swap endianess
          val = (val >> 8) | (val << 8);
          buf[i] = val;
        }
    }
    
  return img;
}
