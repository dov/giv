//======================================================================
//  npy.c - Read npy matrixes.
//
//  Dov Grobgeld <dov.grobgeld@gmail.com>
//  Sun Jan 10 18:35:02 2010
//----------------------------------------------------------------------

#include <string.h>
#include <stdlib.h>
#include "../givimage.h"
#include "../givplugin.h"
#include <glib.h>
#include <math.h>

static giv_plugin_support_t npy_support = {
    TRUE,
    0,
    "\223NUMPY",
    TRUE,
    "npy"
};

giv_plugin_support_t giv_plugin_get_support()
{
    return npy_support;
}

gboolean giv_plugin_supports_file(const char *filename,
                                  guchar *start_chunk,
                                  gint start_chunk_len)
{
    if (start_chunk)
      return g_strstr_len((const gchar*)start_chunk,
                          start_chunk_len,
                          "\223NUMPY") != NULL;
    else {
        char *filename_down = g_utf8_strdown(filename,-1);
        gboolean is_npy = g_str_has_suffix (filename_down, ".npy");
        g_free(filename_down);
        return is_npy;
    }
}

GivImage *giv_plugin_load_file(const char *filename,
                               GError **error)
{
    GivImage *img;
    gchar *npy_string;
    gsize length;
        
    g_file_get_contents(filename, &npy_string, &length, error);

    // Various checks that it is format we support
    gboolean header_ok = (g_strstr_len(npy_string, 6, "\223NUMPY") == npy_string );
    gint header_len = *((guint16*)(npy_string+8));
    
    // Use regex to parse the header. Should update this to allow
    // user attributes.
    GRegex *regex = g_regex_new ("^\\{\\s*"
                                 "'descr':\\s*\\'(.*?)\\'\\s*,\\s*"
                                 "'fortran_order':\\s*(\\w+)\\s*,\\s*"
                                 "'shape':\\s*\\(\\s*(\\d+)\\s*,\\s*(\\d+)(?:,\\s*(\\d+))?\\s*\\),?\\s*"
                                 "\\}", 0, 0, error);
    if (*error) 
        return NULL;

    GMatchInfo *match_info = NULL;
    gboolean is_match = g_regex_match_full(regex,
                                           npy_string+10,
                                           header_len,
                                           0,
                                           (GRegexMatchFlags)0,
                                           &match_info,
                                           error);
    if (*error) 
        return NULL;

    gboolean is_supported_type = TRUE;
    gboolean is_fortran_type = FALSE;
    gint dim0_size=-1, dim1_size=-1, dim2_size=-1;
    gint width=-1, height = -1, depth=-1;
    GivImageType image_type;
    if (is_match) {
        gchar *match_string = g_match_info_fetch(match_info, 1);
        
        if (strcmp(match_string, "<f8")==0) 
            image_type = GIVIMAGE_DOUBLE;
        else if (strcmp(match_string, "<f4")==0) 
            image_type = GIVIMAGE_FLOAT;
        else if (strcmp(match_string, "<i4")==0) 
            image_type = GIVIMAGE_I32;
        else if (strcmp(match_string, "<i2")==0) 
            image_type = GIVIMAGE_I16;
        else if (strcmp(match_string, "<u2")==0) 
            image_type = GIVIMAGE_U16;
        else if (strcmp(match_string, "|u1")==0) 
            image_type = GIVIMAGE_U8;
        else
            is_supported_type = FALSE;
        
        g_free(match_string);
        
        match_string = g_match_info_fetch(match_info, 2);
        is_fortran_type = strcmp(match_string, "True") == 0;
        g_free(match_string);
        
        match_string = g_match_info_fetch(match_info, 3);
        dim0_size = atoi(match_string);
        g_free(match_string);
        
        match_string = g_match_info_fetch(match_info, 4);
        dim1_size = atoi(match_string);
        g_free(match_string);

        match_string = g_match_info_fetch(match_info, 5);
        if (match_string && strlen(match_string))
            dim2_size = atoi(match_string);
        g_free(match_string);

    }
    
    g_regex_unref(regex);
    g_match_info_free(match_info);
    
    if  (!is_match
         || !header_ok
         || !is_supported_type
         || is_fortran_type
         ) {
        //        *error = g_error_new(GIV_IMAGE_ERROR, -1, "Invalid npy file!");
        g_free(npy_string);
        return NULL;
    }

    if (dim2_size>0) {
        depth = dim0_size;
        height = dim1_size;
        width = dim2_size;
        int wsize = giv_image_type_get_size(image_type)/8;  /* Size in bytes */

        img =  giv_image_new_full(image_type,
                                  width,
                                  width * wsize,
                                  height,
                                  height * wsize * width,
                                  2, // rank
                                  depth);

    }
    else {
        height = dim0_size;
        width = dim1_size;
        depth=1;
        img = giv_image_new(image_type,
                            width, height);
    }
    
    // Copy the data
        //        printf("image: type size width height= %d %d\n", image_type, giv_image_type_get_size(image_type), width, height);
    
    guint64 buf_size = (guint64)giv_image_type_get_size(image_type) * width * height /8*depth;
    memcpy(img->buf.buf,
           npy_string + 10 + header_len,
           buf_size);
    g_free(npy_string);

    return img;
}
