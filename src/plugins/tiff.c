//======================================================================
//  tiff.c - A library using tiffio
//
//  Dov Grobgeld <dov.grobgeld@gmail.com>
//  Mon Feb 21 22:54:09 2011
//----------------------------------------------------------------------

#include <string.h>
#include <stdlib.h>
#include "../givimage.h"
#include "../givplugin.h"
#include "../givregex.h"
#include <glib.h>
#include <math.h>
#include "tiffio.h"

static giv_plugin_support_t tiff_support = {
    TRUE,
    0,
    "MM\\0\\*",
    TRUE,
    "tiff?"
};

giv_plugin_support_t giv_plugin_get_support()
{
    return tiff_support;
}

gboolean giv_plugin_supports_file(const char *filename,
                                  guchar *start_chunk,
                                  gint start_chunk_len)
{
    gboolean is_tiff =  ((start_chunk[0] == 'M'
                          && start_chunk[1] == 'M'
                          && start_chunk[2] == 0
                          && start_chunk[3] == '*')
                         || (start_chunk[0] == 'I'
                             && start_chunk[1] == 'I'
                             && start_chunk[2] == '*'
                             && start_chunk[3] == 0));
    return is_tiff;
}

GivImage *giv_plugin_load_file(const char *filename,
                               GError **error)
{
    GivImage *img=NULL;
    gchar *npy_string;
    guint length;
        
    TIFF* tif = TIFFOpen(filename, "r");

    GivImageType image_type;

    if (tif) {
	uint32 w, h, config=9999, bps=9999, spp=9999;
	size_t npixels;
	uint8* raster;
        gboolean has_colormap = FALSE;
        uint16 *rmap, *gmap, *bmap;

	TIFFGetField(tif, TIFFTAG_IMAGEWIDTH, &w);
	TIFFGetField(tif, TIFFTAG_IMAGELENGTH, &h);
	TIFFGetField(tif, TIFFTAG_PLANARCONFIG, &config);
        TIFFGetField(tif, TIFFTAG_BITSPERSAMPLE, &bps);
        TIFFGetField(tif, TIFFTAG_SAMPLESPERPIXEL, &spp);
        if (TIFFGetField(tif, TIFFTAG_COLORMAP, &rmap, &gmap, &bmap))
            has_colormap = TRUE;
        printf("has_colormap= %d\n", has_colormap);
        
        printf("bps spp=%d %d\n", bps, spp);

	raster = (uint8*) _TIFFmalloc(TIFFScanlineSize(tif));
        if (config == PLANARCONFIG_CONTIG) {
        }
        else if (config == PLANARCONFIG_SEPARATE) {
            printf("PLANARCONFIG_SEPERATE not supported\n");
            return NULL;
        }
        else
            printf("Unknown planar config==%d\n", config);
            
	if (raster != NULL) {
            int row_idx, col_idx, clr_idx, config;
            int clr[3];
            int dst_spp = 1;

            // TBD - Support more types.
            if (spp == 3 || has_colormap) {
                image_type = GIVIMAGE_RGB_U8;
                dst_spp = 3;
            }
            else
                image_type = GIVIMAGE_U8;
            img = giv_image_new(image_type, w, h);
            guchar *dst = img->buf.buf;
            
            // Copy the tiff data to the img structure. This can
            // be made more memory conservative by using scanlines.
            for (row_idx=0; row_idx<h; row_idx++) {
                TIFFReadScanline(tif, raster, row_idx, 0);
                guchar *src_ptr = raster;
                guchar *dst_ptr = dst + row_idx * w * dst_spp;

                // Split between colormap and not colormap per row which
                // is sufficiently fast.
                if (has_colormap) {
                    for (col_idx=0; col_idx<w; col_idx++) {
                        int src_gl = *src_ptr++;
                        *dst_ptr++ = rmap[src_gl] >> 8;
                        *dst_ptr++ = gmap[src_gl] >> 8;
                        *dst_ptr++ = bmap[src_gl] >> 8;
                    }
                }
                else {
                    for (col_idx=0; col_idx<w; col_idx++) {
                        for (clr_idx=0; clr_idx<spp; clr_idx++) {
                            *dst_ptr++ = *src_ptr++;
                        }
                    }
                }
	    }
	    _TIFFfree(raster);
	}
	TIFFClose(tif);
    }

    return img;
}
