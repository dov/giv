//======================================================================
//  webp.cc - Support for the webp format
//
//  Dov Grobgeld <dov.grobgeld@gmail.com>
//  Mon Apr 25 21:47:32 2022
//----------------------------------------------------------------------

#include <string.h>
#include <stdlib.h>
#include "../givimage.h"
#include "../givplugin.h"
#include <glib.h>
#include <math.h>
#include "webp/decode.h"
#include <vector>
#include <string>
#include <fstream>

using namespace std;

#define GIV_IMAGE_ERROR g_spawn_error_quark ()

static giv_plugin_support_t webp_support = {
    TRUE,
    0,
    "RIFF....WEBP",
    TRUE,
    "webp"
};

giv_plugin_support_t giv_plugin_get_support()
{
    return webp_support;
}

// Read a file to a string
static bool read_file(const char *filename,
               std::string& data)
{
  std::ifstream fp;
  std::ios_base::openmode Mode = std::ios_base::in | std::ios::binary;

  fp.open(filename,Mode);
  if (!fp.good()) 
      return false;

  data.assign((std::istreambuf_iterator<char>(fp)), std::istreambuf_iterator<char>());
  fp.close();

  return true;
}

// The dll interface uses extern C

extern "C" gboolean giv_plugin_supports_file(const char *filename,
                                             guchar *start_chunk,
                                             gint start_chunk_len)
{
    int width, height;
    return WebPGetInfo(start_chunk, start_chunk_len, &width, &height);
}

extern "C" GivImage *giv_plugin_load_file(const char *filename,
                                          GError **error)
{
    GivImage *img=NULL;
    FILE *fp = fopen(filename, "rb");
    if (!fp) 
        return NULL;
    fclose(fp);

    std::string data;
    if (!read_file(filename, data))
      return NULL;
    
    WebPBitstreamFeatures features;
    WebPGetFeatures((uint8_t*)data.data(),
                    data.size(),
                    &features);
    
    GivImageType image_type;
    if (features.has_alpha) 
      image_type = GIVIMAGE_RGBA_U8;
    else
      image_type = GIVIMAGE_RGB_U8;

    img = giv_image_new(image_type, features.width, features.height);

    if (image_type == GIVIMAGE_RGB_U8)
      WebPDecodeRGBInto((uint8_t*)data.data(), data.size(),
                        img->buf.buf, img->row_stride*img->height, img->row_stride);
    else if (image_type == GIVIMAGE_RGBA_U8)
      WebPDecodeRGBAInto((uint8_t*)data.data(), data.size(),
                         img->buf.buf, img->row_stride*img->height, img->row_stride);
    else
      return NULL;

    return img;
}
