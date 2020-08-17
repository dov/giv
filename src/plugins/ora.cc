// Plugin for the Open raster image format. Currently
// only extracts the merged image from the zip file.

#include "zip.h"
#include "png.h"
#include "../givimage.h"
#include "../givplugin.h"
#include <glib.h>
#include <string.h>
#include <string>

using namespace std;

// A memmem() like function using std. Not the fastest,
// but goodenough.
const guchar *my_memmem(const guchar *haystack,
                      int haystack_size,
                      const guchar *needle,
                      int needle_size)
{
    string hs((char*)haystack, haystack_size);
    string nd((char*)needle, needle_size);
    size_t pos = hs.find(nd);
    if (pos != string::npos)
      return haystack + pos;
    return NULL;
}

#define GIV_IMAGE_ERROR g_spawn_error_quark ()

// This is true for both ora and kra files
static giv_plugin_support_t ora_support = {
    FALSE,
    0,
    "PK\003\004",
    TRUE,
    "ora"
};

extern "C" giv_plugin_support_t giv_plugin_get_support()
{
    return ora_support;
}

extern "C" gboolean giv_plugin_supports_file(const char *filename,
                                  guchar *start_chunk,
                                  gint start_chunk_len)
{
    // An file is most probably an ora file if it is a zip file
    // and it contains an image/openraster string.
    const guchar needle_ora[] = "image/openraster";
    const guchar needle_kra[] = "mimetypeapplication/x-krita";
    return
      // Is this a zip file?
      my_memmem(start_chunk,start_chunk_len,
                (const guchar*)"PK\003\004",4)!=NULL
      // Does it contain a openraster or kra entry?
      // Subtract one since the chunk data is not zero terminated
      && (my_memmem(start_chunk,
                    start_chunk_len,
                    needle_ora,sizeof(needle_ora)-1)!=NULL
          || my_memmem(start_chunk,
                    start_chunk_len,
                       needle_kra,sizeof(needle_kra)-1)!=NULL)
          ;  
}

static int GetZipFile(zip *zh, const string&filename,
                      // output
                      string& contents
                      )
{
  struct zip_stat zstat;
  int ret = zip_stat(zh, filename.c_str(), 0, &zstat);
  if (ret)
    return 0;

  zip_file *zf = zip_fopen(zh, filename.c_str(), 0);
  contents.resize(zstat.size);
  zip_fread(zf, (void*)&contents.front(), contents.size());
  zip_fclose(zf);
  return 1;
}

// TBD - make this multi thread safe by using a struct
static size_t pos = 0;
static void user_read_fn(png_structp png_ptr,
                         png_bytep outBytes,
                         png_size_t byteCountToRead)
{
    png_voidp io_ptr = png_get_io_ptr(png_ptr);
    memcpy(outBytes, (char*)io_ptr+pos, byteCountToRead);
    pos += byteCountToRead;
}

extern "C" GivImage *giv_plugin_load_file(const char *filename,
                               GError **error)
{
    GivImage *img=NULL;
    int zerr {0};
    
    zip *zh = zip_open(filename, ZIP_RDONLY, &zerr);
    if (zerr)
      return NULL;

    string pngbuffer;

    // This is true for both ora nad kra files!
    GetZipFile(zh, "mergedimage.png", pngbuffer);

    png_structp png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL,NULL,NULL);
    if (!png_ptr)
    {
        return NULL;
    }

    // Comments may be stored in the beginning or the end so create
    // a structure for both.
    png_infop info_ptr = png_create_info_struct(png_ptr);
    if (!info_ptr) {
        png_destroy_read_struct(&png_ptr, (png_infopp)NULL, (png_infopp)NULL);
        return NULL;
    }

    png_infop end_info = png_create_info_struct(png_ptr);
    if (!end_info) {
        png_destroy_read_struct(&png_ptr, &info_ptr, (png_infopp)NULL);
        return NULL;
    }


    pos = 0; // Used by the user read function
    png_set_read_fn(png_ptr, (unsigned char*)pngbuffer.c_str(), user_read_fn);

    // The rest is more or less copied from png.c
    // Should really merge these two functions.

    png_set_sig_bytes(png_ptr, 0);
    png_read_info(png_ptr, info_ptr);

    png_uint_32 width, height;
    int bit_depth, color_type, interlace_type, compression_type, filter_method;
    
    png_get_IHDR(png_ptr, info_ptr, &width, &height,
       &bit_depth, &color_type, &interlace_type,
       &compression_type, &filter_method);

    if (color_type == PNG_COLOR_TYPE_PALETTE)
        png_set_palette_to_rgb(png_ptr);

    // Since giv doesn't support gray alpha, we upgrade to rgb
    if (color_type == PNG_COLOR_TYPE_GRAY_ALPHA)
        png_set_gray_to_rgb(png_ptr);

    if (color_type == PNG_COLOR_TYPE_GRAY &&
        bit_depth < 8)
        png_set_expand_gray_1_2_4_to_8(png_ptr);

    if (png_get_valid(png_ptr, info_ptr, PNG_INFO_tRNS))
        png_set_tRNS_to_alpha(png_ptr);

    if (bit_depth == 16)
        png_set_swap(png_ptr);

    // Reread info
    png_read_update_info(png_ptr, info_ptr);
    png_get_IHDR(png_ptr, info_ptr, &width, &height,
       &bit_depth, &color_type, &interlace_type,
       &compression_type, &filter_method);
    

    GivImageType image_type;
    if (color_type == PNG_COLOR_TYPE_GRAY && bit_depth <= 8)
        image_type = GIVIMAGE_U8;
    else if (color_type == PNG_COLOR_TYPE_GRAY && bit_depth == 16)
        image_type = GIVIMAGE_U16;
    else if (color_type == PNG_COLOR_TYPE_RGB && bit_depth == 16)
        image_type = GIVIMAGE_RGB_U16;
    else if (color_type == PNG_COLOR_TYPE_RGB_ALPHA && bit_depth == 16)
        image_type = GIVIMAGE_RGBA_U16;
    else if (color_type == PNG_COLOR_TYPE_RGB_ALPHA && bit_depth == 8)
        image_type = GIVIMAGE_RGBA_U8;
    else
        image_type = GIVIMAGE_RGB_U8;

    img = giv_image_new(image_type, width, height);
    if (!img) {
      *error = g_error_new(GIV_IMAGE_ERROR, -1, "Failed allocating memory for an image of size %dx%d pixels!", width, height);
      return NULL;
    }

    png_bytep *row_pointers = (png_bytep*)g_new0(gpointer, height);

    guchar *dst_buf = img->buf.buf;
    int dst_row_stride = img->row_stride;
    int row_idx;
    for (row_idx=0; row_idx<(int)height; row_idx++)
        row_pointers[row_idx] = dst_buf + dst_row_stride * row_idx;

    png_read_image(png_ptr, row_pointers);

    png_timep mod_time;
    if (png_get_tIME(png_ptr, info_ptr, &mod_time)) {
      gchar *mod_time_str = g_strdup_printf("%04d-%02d-%02d %02d:%02d:%02d",
                                            mod_time->year,
                                            mod_time->month,
                                            mod_time->day,
                                            mod_time->hour,
                                            mod_time->minute,
                                            mod_time->second);
      giv_image_set_attribute(img, "mod_time", mod_time_str);
      g_free(mod_time_str);
    }

    png_textp png_text;
    int num_text;
    if (png_get_text(png_ptr, info_ptr, &png_text, &num_text)) {
        int i;
        for (i=0; i<num_text; i++) 
            giv_image_set_attribute(img, png_text[i].key, png_text[i].text);
    }

    // TBD - Add metadata

    png_read_end(png_ptr, NULL);
    g_free(row_pointers);
    png_destroy_read_struct(&png_ptr, &info_ptr, NULL);

    return img;
}

