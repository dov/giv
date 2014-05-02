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
#include <glib.h>
#include <math.h>
#include "tiffio.h"

#define GIV_IMAGE_ERROR g_spawn_error_quark ()

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
        uint32 w, h, config=9999, bps=1, spp=1, sample_format=9999;
	size_t npixels;
	uint8* raster;
        gboolean has_colormap = FALSE;
        uint16 *rmap, *gmap, *bmap;
        uint16 pn=0, num_pages=0;
        uint16 photometric;
        gboolean do_invert = TRUE;
        gboolean one_bit = FALSE;

	TIFFGetField(tif, TIFFTAG_IMAGEWIDTH, &w);
	TIFFGetField(tif, TIFFTAG_IMAGELENGTH, &h);
	TIFFGetField(tif, TIFFTAG_PLANARCONFIG, &config);
        TIFFGetField(tif, TIFFTAG_BITSPERSAMPLE, &bps);
        TIFFGetField(tif, TIFFTAG_SAMPLESPERPIXEL, &spp);
        TIFFGetField(tif, TIFFTAG_SAMPLEFORMAT, &sample_format);
        TIFFGetField(tif, TIFFTAG_PAGENUMBER, &pn, &num_pages);
        TIFFGetField(tif, TIFFTAG_PHOTOMETRIC, &photometric);

        if (TIFFGetField(tif, TIFFTAG_COLORMAP, &rmap, &gmap, &bmap))
            has_colormap = TRUE;

#if 0
        printf("has_colormap= %d\n", has_colormap);
        
        printf("bps spp=%d %d\n", bps, spp);
        printf("pn num_pages = %d %d\n", pn, num_pages);
#endif

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
            if (spp == 3 || spp==4 || has_colormap) {
                if (bps == 8)
                    image_type = GIVIMAGE_RGB_U8;
                else if (bps == 16)
                    image_type = GIVIMAGE_RGB_U16;
                dst_spp = 3;
            }
            else if(spp == 1
                    && bps == 32
                    && sample_format == SAMPLEFORMAT_IEEEFP) {
                image_type = GIVIMAGE_FLOAT;
            }
            else if (bps == 8)
                image_type = GIVIMAGE_U8;
            else if (bps == 16)
                image_type = GIVIMAGE_U16;
            else if (bps == 32)
                image_type = GIVIMAGE_I32;
            else if (bps == 1) {
                image_type = GIVIMAGE_U8;
                one_bit = TRUE;
                if (photometric == PHOTOMETRIC_MINISBLACK)
                  do_invert = FALSE;
            }
            else {
                printf("Unknown Tiff type!\n");
                return NULL;
            }

            img = giv_image_new(image_type, w, h);
            if (!img) {
                *error = g_error_new(GIV_IMAGE_ERROR, -1, "Failed allocating memory for an image of size %dx%d pixels!", w, h);
                return NULL;
            }
            giv_image_set_one_bit(img, one_bit);

            guchar *dst = img->buf.buf;
            int dst_bpp = giv_image_type_get_size(image_type);
            int dst_row_stride = giv_image_get_row_stride(img);

            // Copy the tiff data to the img structure. This can
            // be made more memory conservative by using scanlines.
            for (row_idx=0; row_idx<h; row_idx++) {
                TIFFReadScanline(tif, raster, row_idx, 0);
                guchar *src_ptr = raster;
                guchar *dst_ptr = dst + row_idx * dst_row_stride;

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
                else if (bps == 1) {
                    // This could certainly be speeded up
                    for (col_idx=0; col_idx<w; col_idx++) {
                        int bit_idx = col_idx % 8;
                        int b = ((*src_ptr >> (7-bit_idx))&1);
                        if (do_invert)
                          b = 1-b;
                        *dst_ptr++ = b;
                        if (bit_idx==7)
                          src_ptr++;
                    }
                }
                else {
                    for (col_idx=0; col_idx<w; col_idx++) {
                        for (clr_idx=0; clr_idx<dst_spp*bps/8; clr_idx++) {
                            *dst_ptr++ = *src_ptr++;

                        }
                        // Skip alpha channel
                        int i;
                        for (i=0; i<spp-dst_spp; i++)
                            src_ptr++;
                    }
                }
	    }
	    _TIFFfree(raster);
	}
	TIFFClose(tif);

        char buf[32];
        sprintf(buf,"%d",photometric);
        giv_image_set_attribute(img, "photometric", buf);
    }

    return img;
}
