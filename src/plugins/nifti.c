//======================================================================
//  nii.c - A 2D/3D NIfTI reader for giv.
//
//  Dov Grobgeld <dov.grobgeld@gmail.com>
//  2025-07-26
//----------------------------------------------------------------------

#include "../givimage.h"
#include <glib.h>
#include <string.h>
#include <math.h>
#include <nifti1_io.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "../givplugin.h"

static giv_plugin_support_t nii_support = {
    TRUE,
    0,
    "NIFTI",
    TRUE,
    "nii"
};

giv_plugin_support_t giv_plugin_get_support()
{
    return nii_support;
}

gboolean giv_plugin_supports_file(const char *filename,
                                  guchar *start_chunk,
                                  gint start_chunk_len)
{
    char *filename_down = g_utf8_strdown(filename, -1);
    gboolean is_nii = g_str_has_suffix(filename_down, ".nii") ||
                      g_str_has_suffix(filename_down, ".nii.gz");
    g_free(filename_down);
    return is_nii;
}

static int
nifti_to_giv_type(int nifti_dtype)
{
    switch (nifti_dtype) {
    case DT_UINT8:   return GIVIMAGE_U8;
    case DT_INT16:   return GIVIMAGE_I16;
    case DT_UINT16:  return GIVIMAGE_U16;
    case DT_INT32:   return GIVIMAGE_I32;
    case DT_UINT32:  return GIVIMAGE_U32;
    case DT_FLOAT32: return GIVIMAGE_FLOAT;
    case DT_FLOAT64: return GIVIMAGE_DOUBLE;
    default:         return -1;
    }
}

GivImage *giv_plugin_load_file(const char *filename,
                               GError **error)
{
    struct stat buf;
    nifti_image *nim = NULL;
    GivImage *img = NULL;
    int type, wsize, width, height, depth;

    // Check file exists
    if (stat(filename, &buf) != 0) {
        fprintf(stderr, "Failed opening the file %s!\n", filename);
        goto CATCH;
    }

    nim = nifti_image_read(filename, 1);
    if (!nim) {
        fprintf(stderr, "Failed to read NIfTI file %s\n", filename);
        goto CATCH;
    }

    // Only support 2D/3D for now
    if (nim->ndim < 2 || nim->ndim > 3) {
        fprintf(stderr, "Sorry. Only support 2D and 3D NIfTI files!\n");
        goto CATCH;
    }

    width  = nim->nx;
    height = nim->ny;
    depth  = nim->nz > 0 ? nim->nz : 1;

    type = nifti_to_giv_type(nim->datatype);
    if (type < 0) {
        fprintf(stderr, "Unsupported NIfTI data type: %d\n", nim->datatype);
        goto CATCH;
    }

    wsize = giv_image_type_get_size(type) / 8;
    img = giv_image_new_full(type,
                             width,
                             width * wsize,
                             height,
                             height * wsize * width,
                             2,
                             depth);

    // Copy data to GivImage buffer
    memcpy(img->buf.buf, nim->data, wsize * width * height * depth);

CATCH:
    if (nim)
        nifti_image_free(nim);

    if (!img && error) {
        *error = g_error_new_literal(g_quark_from_string("giv-plugin-nii"), 1, "Failed to load NIfTI image");
    }

    return img;
}
