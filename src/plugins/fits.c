//======================================================================
//  fits.c - A 2D fits reader for giv.
//
//  Dov Grobgeld <dov.grobgeld@gmail.com>
//  Tue Dec 22 18:45:26 2009
//----------------------------------------------------------------------

#include "../givimage.h"
#include <glib.h>
#include <string.h>
#include <math.h>
#include <fitsio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "../givplugin.h"

static giv_plugin_support_t fits_support = {
    TRUE,
    0,
    "SIMPLE\\s*=\\s*T",
    TRUE,
    "fits"
};

giv_plugin_support_t giv_plugin_get_support()
{
    return fits_support;
}

gboolean giv_plugin_supports_file(const char *filename,
                                  guchar *start_chunk,
                                  gint start_chunk_len)
{
    char *filename_down = g_utf8_strdown(filename,-1);
    gboolean is_fits = g_str_has_suffix (filename_down, "fits");
    g_free(filename_down);

    return is_fits;
}

static int
bitpix_to_data_type(int bitpix)
{
    switch (bitpix) {
    case -64 : return GIVIMAGE_DOUBLE;
    case -32 : return GIVIMAGE_FLOAT;
    case 32  : return GIVIMAGE_I32;
    case 16  : return GIVIMAGE_U16;
    case 8   : return GIVIMAGE_U8;
    default  : return 0;
    }
}

static int
giv_image_type_to_fits_type(GivImageType type)
{
    switch (type) {
    case GIVIMAGE_U8:
	return TBYTE;
    case GIVIMAGE_U16:
	return TUSHORT;
    case GIVIMAGE_I16:
	return TSHORT;
    case GIVIMAGE_I32:
	return TLONG;
    case GIVIMAGE_FLOAT:
	return TFLOAT;
    case GIVIMAGE_DOUBLE:
	return TDOUBLE;
    default:
	return -1;
    }
}

GivImage *giv_plugin_load_file(const char *filename,
                               GError **error)
{
    fitsfile *fptr;       /* pointer to the FITS file; defined in fitsio.h */
    long nnaxes, naxes[3];
    int status;
    int width, height, depth;
    GivImageType type;
    int nfound;
    int bitpix;
    GivImage *img = NULL;
    void *flip_data = NULL;
    int wsize;
    int anynull;
    int fpixel   = 1;

    /* Check that file exists without the use of the cfits library
       as it is doing strange things if the file doesn't exists... */
    status = 0;
    {
        struct stat buf;

        if (stat(filename, &buf) != 0) {
            fprintf(stderr, "Failed opening the file %s!\n", filename);
            goto CATCH;
        }
    }

    status = 0;
    if (fits_open_file(&fptr, filename, READONLY, &status) != 0)
	goto CATCH;

    if (fits_read_key(fptr, TLONG, "NAXIS", &nnaxes, NULL, &status) != 0)
	goto CATCH;
    
    /* Read the header data */
    if ( fits_read_keys_lng(fptr, "NAXIS", 1, nnaxes, naxes, &nfound, &status) )
	goto CATCH;
    width = naxes[0];
    height = naxes[1];
    depth = naxes[2];
    if (nnaxes == 2 || depth == 0)
        depth = 1;

    if (nnaxes < 2 || nnaxes > 3) {
        fprintf(stderr, "Sorry. Only support 2D and 3D fits files!\n");
        goto CATCH;
    }

    if (fits_read_key(fptr, TLONG, "BITPIX", &bitpix, NULL, &status) != 0)
	goto CATCH;

    type = bitpix_to_data_type(bitpix);
    wsize = giv_image_type_get_size(type)/8;  /* Size in bytes */
    img =  giv_image_new_full(type,
                              width,
                              width * wsize,
                              height,
                              height * wsize * width,
                              2,
                              depth);
    wsize = giv_image_type_get_size(type)/8;  /* Size in bytes */

    flip_data = malloc(wsize * width * height*depth);
    if ( fits_read_img(fptr, giv_image_type_to_fits_type(type), fpixel, width*height * depth, 0,
		       flip_data, &anynull, &status) )
	goto CATCH;

#ifdef DO_FLIP
    {
        int slice_size, row_idx, z_idx;

        /* Flip the image */
        slice_size = width * wsize * height;
        for (z_idx = 0; z_idx < depth; z_idx++)
            for (row_idx = 0; row_idx < height; row_idx++)
                memcpy(img->buf.buf + (uint64_t)z_idx * slice_size
                       + (height - row_idx - 1) * width * wsize,
                       (guchar*)flip_data + (uint64_t)z_idx * slice_size + row_idx * width * wsize,
                       width * wsize);
    }
#else
    memcpy(img->buf.buf,
           flip_data,
           (uint64_t)width * height * wsize * depth);
#endif
    
    fits_close_file(fptr, &status);            /* close the file */

 CATCH:
    free(flip_data);
    if (status!= 0)
        fits_report_error(stderr, status);  /* print out any error messages */
    
    return img;
}
