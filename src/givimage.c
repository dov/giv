//======================================================================
//  givimage.c - 
//
//  Dov Grobgeld <dov.grobgeld@gmail.com>
//  Fri Oct 16 10:55:08 2009
//----------------------------------------------------------------------

#include <stdlib.h>
#include <string.h>
#include "givimage.h"
#include "givplugin.h"
#include <gdk-pixbuf/gdk-pixbuf.h>
#include "givregex.h"

#define GIV_IMAGE_ERROR g_spawn_error_quark ()

static guint8 clip_u8(double f)
{
    if (f < 0)
        return 0;
    if (f > 255)
        return 255;
    return (int)f;
}

GQuark
giv_image_error_quark (void)
{
  return g_quark_from_static_string ("giv-image-error-quark");
}

GivImage *giv_image_new_full(GivImageType img_type,
                             int width,
                             int row_stride,
                             int height,
                             int frame_stride,
                             int rank,
                             int depth)
{
    GivImage *img = g_new0(GivImage, 1);
    img->rank = rank;
    img->ref_count = 1;
    img->img_type = img_type;
    img->width = width;
    img->row_stride = row_stride;
    img->frame_stride = frame_stride;
    img->height= height;
    if (depth < 1)
        depth = 1;
    img->depth = depth;
    int buf_size = frame_stride * depth;

    // This should probably be aligned to be faster...
    img->buf.buf = (guint8*)g_new0(guint8*, buf_size);
    img->attribute_map = g_hash_table_new_full(g_str_hash,
                                               g_str_equal,
                                               g_free,
                                               g_free);

    return img;
}

GivImage *giv_image_new(GivImageType img_type,
                        int width,
                        int height)
{
    int row_stride = giv_image_type_get_size(img_type)/8 * width;
    return giv_image_new_full(img_type,
                              width,
                              row_stride,
                              height,
                              height * row_stride,
                              2,
                              1);
}

GivImage *giv_image_new_from_file(const char *filename,
                                  GError **error)
{
    GivImage *img = NULL;
    // TBD - run through a list of loaders. Right now just
    // use gdkpixbuf.
    gchar *extension = g_strrstr(filename, ".");
    extension++;

    if ((img = giv_plugin_load_image(filename,
                                     error)) != NULL) {
    }
    else if (*error) {
    }
    else if (giv_regex_match_simple("png"
                                  "|jpe?g"
                                  "|p[bgp]m"
                                  "|bmp"
                                  "|svg"
                                  ,
                                  extension,
                                  GIV_REGEX_CASELESS,
                                  0)) {
        GdkPixbuf *pixbuf = gdk_pixbuf_new_from_file(filename,
                                                     error);
        
        if (*error)
            return img;

        gboolean is_mono = TRUE;

        // Check if the file is monochrome.
        int pix_idx;
        int width = gdk_pixbuf_get_width(pixbuf);
        int height = gdk_pixbuf_get_height(pixbuf);
        guint8 *buf = gdk_pixbuf_get_pixels(pixbuf);
        int row_stride = gdk_pixbuf_get_rowstride(pixbuf);
        
        for(pix_idx=0; pix_idx<width * height; pix_idx++) {
            if (buf[0] != buf[1]
                || buf[0] != buf[2]) {
                is_mono = FALSE;
                break;
            }
            buf+= 3;
        }

        if (is_mono) {
            GivImageType img_type = GIVIMAGE_U8;
            img = giv_image_new_full(img_type,
                                     width,
                                     width,
                                     height,
                                     width * height,
                                     2,
                                     1);

            guchar *src_buf = gdk_pixbuf_get_pixels(pixbuf);
            guchar *dst_buf = img->buf.buf;
            int row_idx, col_idx;
            int ncolors = 3;
            if (gdk_pixbuf_get_has_alpha(pixbuf))
                ncolors = 4;
            for (row_idx=0; row_idx<height; row_idx++) {
                guchar *dst_row = dst_buf + row_idx*width;
                guchar *src_row = src_buf + row_idx*row_stride;
                for (col_idx=0; col_idx<width; col_idx++) 
                    dst_row[col_idx] = src_row[col_idx*ncolors];
            }
        }
        else {
            GivImageType img_type = GIVIMAGE_RGB_U8;
            if (gdk_pixbuf_get_has_alpha(pixbuf)) 
                img_type = GIVIMAGE_RGBA_U8;
            img = giv_image_new_full(img_type,
                                     width,
                                     row_stride,
                                     height,
                                     row_stride * height,
                                     2,
                                     1);
            memcpy(img->buf.buf,
                   gdk_pixbuf_get_pixels(pixbuf),
                   row_stride * height);
        }
        gdk_pixbuf_unref(pixbuf);
    }
    // Space separated value. A simple text format parser. Still
    // doesn't support comments. Fix this!
    else if (giv_regex_match_simple("ssv",
                                  extension,
                                  GIV_REGEX_CASELESS,
                                  0)) {
        gchar *ssv_string;
        guint length;
        
        g_file_get_contents(filename, &ssv_string, &length, error);
        gchar **lines = giv_regex_split_simple("\r?\n",
                                               ssv_string,
                                               0, 0);
        int num_lines = g_strv_length(lines);

        // Count lines while skipping comments
        int height = 0;
        int line_idx;
        for (line_idx = 0; line_idx<num_lines; line_idx++) {
            if (lines[line_idx][0] == '#'
                || strlen(lines[line_idx]) == 0)
                continue;
            height++;
        }

        gint width = -1;
        float *fbuf;
        int row_idx = 0;
        for (line_idx=0; line_idx<num_lines; line_idx++) {
            int col_idx;
            if (lines[line_idx][0] == '#'
                || strlen(lines[line_idx]) == 0)
                continue;
            // comma or space split
            gchar *p = lines[line_idx];

            // skip whitespace
            while(*p == ' ')
                p++;
            gchar **fields = giv_regex_split_simple("(?:,|;|\\s)\\s*",
                                                    p,
                                                    0,0);
            if (row_idx==0) {
                width = g_strv_length(fields);
                img = giv_image_new(GIVIMAGE_FLOAT,
                                    width, height);
                fbuf = img->buf.fbuf;
            }
            for (col_idx=0; col_idx<width; col_idx++) 
                fbuf[row_idx*width + col_idx] = atof(fields[col_idx]);
            g_strfreev(fields);
            row_idx++;

        }
        g_strfreev(lines);
        g_free(ssv_string);
    }
#if 0
    else if (giv_regex_match_simple("npy",
                                    extension,
                                    GIV_REGEX_CASELESS,
                                    0)) {
        gchar *npy_string;
        guint length;
        
        g_file_get_contents(filename, &npy_string, &length, error);

        // Various checks that it is format we support
        gboolean header_ok = (g_strstr_len(npy_string, 6, "\223NUMPY") == npy_string );
        gboolean ver_ok = (npy_string[6] == 1
                           && npy_string[7] == 0);
        gint header_len = *((guint16*)(npy_string+8));

        // Use regex to parse the header. Should update this to allow
        // user attributes.
        GivRegex *regex = giv_regex_new ("^\\{\\s*"
                                       "'descr':\\s*\\'(.*?)\\'\\s*,\\s*"
                                       "'fortran_order':\\s*(\\w+)\\s*,\\s*"
                                       "'shape':\\s*\\(\\s*(\\d+)\\s*,\\s*(\\d+)\\s*\\),?\\s*"
                                       "\\}", 0, 0, error);
        if (*error) {
            printf("Programming GivRegEx error: %s\n", (*error)->message);
            exit(-1);
        }
        GivMatchInfo *match_info = NULL;
        gboolean is_match = giv_regex_match_full(regex,
                                                 npy_string+10,
                                                 header_len,
                                                 0,
                                                 (GivRegexMatchFlags)0,
                                                 &match_info,
                                                 error);
        gboolean is_supported_type = TRUE;
        gboolean is_fortran_type = FALSE;
        gint width=-1, height = -1;
        GivImageType image_type;
        if (is_match) {
            gchar *match_string = giv_match_info_fetch(match_info, 1);

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
            
            match_string = giv_match_info_fetch(match_info, 2);
            is_fortran_type = strcmp(match_string, "True") == 0;
            g_free(match_string);
            
            match_string = giv_match_info_fetch(match_info, 3);
            height = atoi(match_string);
            g_free(match_string);
            
            match_string = giv_match_info_fetch(match_info, 4);
            width = atoi(match_string);
            g_free(match_string);
        }

        giv_regex_unref(regex);
        giv_match_info_free(match_info);
        
        if  (!is_match
             || !header_ok
             || !is_supported_type
             || is_fortran_type
            ) {
            *error = g_error_new(GIV_IMAGE_ERROR, -1, "Invalid npy file!");
            g_free(npy_string);
            return NULL;
        }

        img = giv_image_new(image_type,
                            width, height);

        // Copy the data
        //        printf("image: type size width height= %d %d\n", image_type, giv_image_type_get_size(image_type), width, height);

        memcpy(img->buf.buf,
               npy_string + 10 + header_len,
               giv_image_type_get_size(image_type) * width * height / 8);
        g_free(npy_string);
    }
#endif
    else {
        *error = g_error_new(GIV_IMAGE_ERROR, -1, "Foo: Unknown filetype %s!", extension);
    }
    
    return img;
}

void giv_image_ref(GivImage *img)
{
    img->ref_count++;
}
    
void giv_image_unref(GivImage *img)
{
    img->ref_count--;

    if (img->ref_count !=0)
        return;
    g_hash_table_unref(img->attribute_map);
    g_free(img);
}

int giv_image_type_get_size(GivImageType img_type)
{
    switch (img_type) {
    case GIVIMAGE_U8:
        return 8;
    case GIVIMAGE_U16:
        return 16;
    case GIVIMAGE_I16:
        return 16;
    case GIVIMAGE_I32:
        return 32;
    case GIVIMAGE_FLOAT:
        return 32;
    case GIVIMAGE_DOUBLE:
        return 64;
    case GIVIMAGE_RGB_U8:
        return 24;
    case GIVIMAGE_RGBA_U8:
        return 32;
    case GIVIMAGE_RGB_U16:
        return 16*3;
    case GIVIMAGE_RGBA_U16:
        return 16*4;
    }
    return -1;
}

int giv_image_get_is_color(GivImage *img)
{
    return (img->img_type == GIVIMAGE_RGB_U8
            || img->img_type == GIVIMAGE_RGBA_U8
            || img->img_type == GIVIMAGE_RGB_U16
            || img->img_type == GIVIMAGE_RGBA_U16);
}

int giv_image_get_rank(GivImage *img)
{
    return img->rank;
}

int giv_image_get_width(GivImage *img)
{
    return img->width;
}

int giv_image_get_row_stride(GivImage *img)
{
    return img->row_stride;
}

int giv_image_get_height(GivImage *img)
{
    return img->height;
}

int giv_image_get_depth(GivImage *img)
{
    return img->depth;
}
        

double giv_image_get_value(GivImage *img,
                           int x_idx,
                           int y_idx,
                           int z_idx)
{
    guchar *row_start = img->buf.buf
        + img->frame_stride * z_idx
        + img->row_stride * y_idx;
    switch (img->img_type) {
    case GIVIMAGE_U8:
        return ((guint8*)row_start)[x_idx];
    case GIVIMAGE_U16:
        return ((guint16*)row_start)[x_idx];
    case GIVIMAGE_I16:
        return ((gint16*)row_start)[x_idx];
    case GIVIMAGE_I32:
        return ((gint*)row_start)[x_idx];
    case GIVIMAGE_FLOAT:
        return ((gfloat*)row_start)[x_idx];
    case GIVIMAGE_DOUBLE:
        return ((gdouble*)row_start)[x_idx];
    case GIVIMAGE_RGB_U8:
        return (((GivImageRgb8*)row_start)[x_idx]).red;
    case GIVIMAGE_RGBA_U8:
        return (((GivImageRgbAlpha8*)row_start)[x_idx]).red;
    case GIVIMAGE_RGB_U16:
        {
            GivImageRgb16 rgb = (((GivImageRgb16*)row_start)[x_idx]);
            int max = rgb.red;
            if (rgb.green > max)
                max = rgb.green;
            if (rgb.blue > max)
                max = rgb.blue;
            return max;
        }
    case GIVIMAGE_RGBA_U16:
        return (((GivImageRgbAlpha16*)row_start)[x_idx]).red;
    default:
        return -1;
    }
}

GivImageRgb16 giv_image_get_rgb_value(GivImage *img,
                                      int x_idx,
                                      int y_idx,
                                      int z_idx)
{
    GivImageRgb16 rgb = {0,0,0};
    int byte_size = giv_image_type_get_size(img->img_type)/8;
    int idx = (z_idx * img->frame_stride
               + y_idx * img->row_stride)/byte_size
               + x_idx;
    switch (img->img_type) {
    case GIVIMAGE_U8:
        rgb.red = img->buf.buf[idx];
        break;
    case GIVIMAGE_RGB_U8:
        rgb.red = img->buf.rgb8_buf[idx].red;
        rgb.green = img->buf.rgb8_buf[idx].green;
        rgb.blue = img->buf.rgb8_buf[idx].blue;
        break;
    case GIVIMAGE_RGBA_U8:
        rgb.red = img->buf.rgba8_buf[idx].red;
        rgb.green = img->buf.rgba8_buf[idx].green;
        rgb.blue = img->buf.rgba8_buf[idx].blue;
        break;
    case GIVIMAGE_RGB_U16:
        rgb.red = img->buf.rgb16_buf[idx].red;
        rgb.green = img->buf.rgb16_buf[idx].green;
        rgb.blue = img->buf.rgb16_buf[idx].blue;
        break;
    case GIVIMAGE_RGBA_U16:
        rgb.red = img->buf.rgba16_buf[idx].red;
        rgb.green = img->buf.rgba16_buf[idx].green;
        rgb.blue = img->buf.rgba16_buf[idx].blue;
        break;
    default:
        ;
    }
    return rgb;
}

GivImageRgbAlpha16 giv_image_get_rgba_value(GivImage *img,
                                            int x_idx,
                                            int y_idx,
                                            int z_idx)
{
    GivImageRgbAlpha16 rgba = {0,0,0,0};
    int byte_size = giv_image_type_get_size(img->img_type)/8;
    int idx = (z_idx * img->frame_stride
               + y_idx * img->row_stride)/byte_size
               + x_idx;

    switch (img->img_type) {
    case GIVIMAGE_U8:
        rgba.red = img->buf.buf[idx];
        break;
    case GIVIMAGE_RGB_U8:
        rgba.red = img->buf.rgb8_buf[idx].red;
        rgba.green = img->buf.rgb8_buf[idx].green;
        rgba.blue = img->buf.rgb8_buf[idx].blue;
        break;
    case GIVIMAGE_RGBA_U8:
        rgba.red = img->buf.rgba8_buf[idx].red;
        rgba.green = img->buf.rgba8_buf[idx].green;
        rgba.blue = img->buf.rgba8_buf[idx].blue;
        rgba.alpha = img->buf.rgba8_buf[idx].alpha;
        break;
    default:
        ;
    }
    return rgba;
}

// Some simple manipulations - only for gray images
void giv_image_get_min_max(GivImage *img,
                           // output
                           double* min,
                           double* max)
{
    int width = img->width;
    int height = img->height;
    int depth = img->depth;
    int x_idx, y_idx, z_idx;
    *min = DBL_MAX;
    *max = -DBL_MAX;
    
    for (z_idx=0; z_idx<depth; z_idx++) {
        for (y_idx=0; y_idx<height; y_idx++) {
            for (x_idx=0; x_idx<width; x_idx++) {
                double val = giv_image_get_value(img,
                                                 x_idx, y_idx, z_idx);
                if (val > *max)
                    *max = val;
                if (val < *min)
                    *min =val;
            }
        }
    }
}

GHashTable *giv_image_get_attribute_map(GivImage *img)
{
    return img->attribute_map;
}

gboolean giv_image_attribute_exists(GivImage *img,
                                    const char *key)
{
    gchar *val = (gchar*)g_hash_table_lookup(img->attribute_map,
                                             key);
    return val != NULL;
}

void giv_image_set_attribute(GivImage *img,
                             const gchar* key,
                             const gchar* value)
{
    g_hash_table_replace(img->attribute_map,
                         g_strdup(key),
                         g_strdup(value));
}

const gchar* giv_image_get_attribute(GivImage *img,
                                     const gchar* key)
{
    return (gchar*)g_hash_table_lookup(img->attribute_map, key);
}

double giv_image_get_attribute_double(GivImage *img,
                                      const gchar* key)
{
    return atof(giv_image_get_attribute(img,key));
}

int giv_image_get_attribute_int(GivImage *img,
                                const gchar* key)
{
    return atoi(giv_image_get_attribute(img,key));
}

GdkPixbuf *giv_image_get_pixbuf(GivImage *img,
                                int slice_idx,
                                double min, double max)
{
    if (!img)
        return NULL;
    int width = img->width;
    int height = img->height;
    int bits_per_sample = 8;
    int num_ch_per_pixel = 1;
    gboolean is_rgb = (img->img_type == GIVIMAGE_RGB_U8
                       || img->img_type == GIVIMAGE_RGBA_U8
                       || img->img_type == GIVIMAGE_RGB_U16
                       || img->img_type == GIVIMAGE_RGBA_U16);
    
    gboolean has_alpha = (img->img_type == GIVIMAGE_RGBA_U8
                          || img->img_type == GIVIMAGE_RGBA_U16);
    
    GdkPixbuf *pixbuf = gdk_pixbuf_new(GDK_COLORSPACE_RGB,
                                       has_alpha,
                                       bits_per_sample,
                                       width,
                                       height);
    guchar *pbuf = gdk_pixbuf_get_pixels(pixbuf);
    guchar *buf = img->buf.buf;
    int pb_rowstride = gdk_pixbuf_get_rowstride(pixbuf);
    int row_idx, col_idx;
    if (is_rgb)
        num_ch_per_pixel=3;
    if (has_alpha)
        num_ch_per_pixel++;

    // For color images just copy the data without any scaling...
    if (is_rgb) {
        int row_stride = img->row_stride;

        // Silently ignore 16 bit images at the moment
        if (img->img_type == GIVIMAGE_RGB_U16) {
            for (row_idx=0; row_idx<height; row_idx++) {
                guchar *row = pbuf + row_idx * pb_rowstride;
                for (col_idx=0; col_idx<width; col_idx++) {
                    GivImageRgb16 rgb16
                        = giv_image_get_rgb_value(img,
                                                  col_idx,
                                                  row_idx,
                                                  slice_idx);
                    row[col_idx * 3] = clip_u8((rgb16.red-min)*255/(max-min));
                    row[col_idx * 3+1] = clip_u8((rgb16.green-min)*255/(max-min));
                    row[col_idx * 3+2] = clip_u8((rgb16.blue-min)*255/(max-min));
                }
            }
        }
        else {
            for (row_idx=0; row_idx<height; row_idx++) {
                for (col_idx=0; col_idx<width; col_idx++) {
                    int ch_idx;
                    for (ch_idx=0; ch_idx<num_ch_per_pixel; ch_idx++) {
                        pbuf[row_idx * pb_rowstride
                             + col_idx * num_ch_per_pixel
                             + ch_idx]
                            = buf[row_idx * row_stride
                                  +col_idx * num_ch_per_pixel
                                  + ch_idx]; 
                    }
                }
            }
        }
    }
    else {
        // This should be ammended to support arbitrary color lookup
        // tables...
        for (row_idx=0; row_idx<height; row_idx++) {
            for (col_idx=0; col_idx<width; col_idx++) {
                double val = giv_image_get_value(img,
                                                 col_idx, row_idx, slice_idx);
                int ch_idx;
                if (max == min)
                    val = 128;
                else
                    val = clip_u8(255.0 * (val-min)/(max-min));
                for (ch_idx=0; ch_idx<3; ch_idx++) 
                    pbuf[row_idx * pb_rowstride
                         + col_idx * 3
                         + ch_idx] = (int)(val);
            }
        }
    }
    return pixbuf;
}
