//======================================================================
//  png.c - Load 8-bit and 16-bit png images
//
//  Dov Grobgeld <dov.grobgeld@gmail.com>
//  Wed Apr  6 18:49:17 2011
//----------------------------------------------------------------------
#include <string.h>
#include <stdlib.h>
#include "../givimage.h"
#include "../givplugin.h"
#include <glib.h>
#include <math.h>
#include "png.h"

#define GIV_IMAGE_ERROR g_spawn_error_quark ()

static giv_plugin_support_t png_support = {
    TRUE,
    0,
    "\211PNG",
    TRUE,
    "png"
};

giv_plugin_support_t giv_plugin_get_support()
{
    return png_support;
}

gboolean giv_plugin_supports_file(const char *filename,
                                  guchar *start_chunk,
                                  gint start_chunk_len)
{
    return !png_sig_cmp(start_chunk, 0, start_chunk_len);
}

GivImage *giv_plugin_load_file(const char *filename,
                               GError **error)
{
    GivImage *img=NULL;
    FILE *fp = fopen(filename, "rb");
    if (!fp) 
        return NULL;
    
    // Check again that this is a png file
    const int number = 8;
    guchar header[9];
    fread(header, 1, number, fp);
    gboolean is_png = !png_sig_cmp(header, 0, number);
    if (!is_png) {
        return NULL;
    }
    
    png_structp png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL,NULL,NULL);
    if (!png_ptr)
        return NULL;

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

    png_uint_32 width, height;
    int bit_depth, color_type, interlace_type, compression_type, filter_method;

    png_init_io(png_ptr, fp);
    png_set_sig_bytes(png_ptr, 8);
    png_read_info(png_ptr, info_ptr);

    png_get_IHDR(png_ptr, info_ptr, &width, &height,
       &bit_depth, &color_type, &interlace_type,
       &compression_type, &filter_method);

#if 0
    printf("width height bit_depth color_type = %d %d %d %d\n",
           (int)width, (int)height,
           bit_depth, color_type);
#endif
    
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

    png_read_end(png_ptr, NULL);
    g_free(row_pointers);
    png_destroy_read_struct(&png_ptr, &info_ptr, NULL);

    return img;
}
