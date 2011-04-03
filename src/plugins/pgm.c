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

static giv_plugin_support_t pgm_support = {
    TRUE,
    0,
    "P5",
    TRUE,
    "pgm"
};

static double sqr(double x) {
    return x*x;
}

giv_plugin_support_t giv_plugin_get_support()
{
    return pgm_support;
}

gboolean giv_plugin_supports_file(const char *filename,
                                  guchar *start_chunk,
                                  gint start_chunk_len)
{
    return g_strstr_len((const gchar*)start_chunk,
                        start_chunk_len,
                        "P5") != NULL;
}

GivImage *giv_plugin_load_file(const char *filename)
{
    return NULL;
#if 0
    int row_idx, col_idx;
    int w = 100, h=100;
    GivImage *img = giv_image_new(GIVIMAGE_FLOAT,w,h);

    
    for (row_idx=0; row_idx<h; row_idx++) {
        for (col_idx=0; col_idx<w; col_idx++) {
            double r = sqrt(sqr(row_idx-h/2)+sqr(col_idx-w/2)) * 6.28/(w/4)+1e-6;
            img->buf.fbuf[row_idx * w + col_idx] = sin(r)/r;
        }
    }
    return img;
#endif
}
