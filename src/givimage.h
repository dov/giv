//======================================================================
//  GivImage.h - A multi-layer image for Giv.
//
//  Dov Grobgeld <dov.grobgeld@gmail.com>
//  Sat Nov 17 19:09:38 2007
//----------------------------------------------------------------------

#ifndef GIVIMAGE_H
#define GIVIMAGE_H

#include <stdint.h>
#include <gdk-pixbuf/gdk-pixbuf.h>
#include <glib.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/** 
 * This enum defines the types supported by GivImage
 * 
 * @return 
 */
typedef enum {
    GIVIMAGE_U8,
    GIVIMAGE_U16,
    GIVIMAGE_I16,
    GIVIMAGE_I32,
    GIVIMAGE_FLOAT,
    GIVIMAGE_DOUBLE,
    GIVIMAGE_RGB_U8,
    GIVIMAGE_RGBA_U8,
    GIVIMAGE_RGB_U16,
    GIVIMAGE_RGBA_U16
} GivImageType;

typedef struct {
    guint8 red, green, blue;
} GivImageRgb8 ;

typedef struct {
    guint8 red, green, blue, alpha;
} GivImageRgbAlpha8;

typedef struct {
    guint16 red, green, blue;
} GivImageRgb16;

typedef struct {
    guint16 red, green, blue, alpha;
} GivImageRgbAlpha16;

/** 
 * The GivImage class wraps a general image type that may store multi-dimensional
 * data of various types.
 *
 * It provides the following extra features:
 *
 *  - The image may have string attributes through a map<string,string>.
 *    These attributes should be stored and restored by any IO routines.
 *
 *  - The image provides ref() and unref() methods for simple reference
 *    counting support.
 * 
 */

typedef struct {
    GivImageType img_type;
    int rank;
    int width;
    int height;
    int depth;
    int row_stride;
    int frame_stride;
    gboolean one_bit;
    union {
        guint8 *buf;
        guint16 *sbuf;
        gint16 *isbuf;
        gint32 *ulbuf;
        float *fbuf;
        double *dbuf;
        GivImageRgb8* rgb8_buf;
        GivImageRgbAlpha8 *rgba8_buf;
        GivImageRgb16* rgb16_buf;
        GivImageRgbAlpha16 *rgba16_buf;
    } buf;
    int ref_count; // reference count
    GHashTable *attribute_map; 
} GivImage;

GivImage *giv_image_new_full(GivImageType img_type,
                             int width,
                             int row_stride,
                             int height,
                             int frame_stride,
                             int rank,
                             int depth);
GivImage *giv_image_new_3d(GivImageType image_type,
                           int width, int height, int depth);
GivImage *giv_image_new(GivImageType image_type,
                        int width,
                        int height);
GivImage *giv_image_new_from_file(const char *filename,
                                  GError **error);
int giv_image_get_is_color(GivImage *img);
int giv_image_get_rank(GivImage *img);
int giv_image_get_width(GivImage *img);
int giv_image_get_row_stride(GivImage *img);
int giv_image_get_height(GivImage *img);
int giv_image_get_depth(GivImage *img);

/** 
 * This should only be used for gray level images and it returns
 * the pixel value as a double.
 * 
 * @param img 
 * @param x_idx 
 * @param y_idx 
 * @param z_idx 
 * 
 * @return 
 */
double giv_image_get_value(GivImage *img,
                           int x_idx,
                           int y_idx,
                           int z_idx);

GivImageRgb16 giv_image_get_rgb_value(GivImage *img,
                                      int x_idx,
                                      int y_idx,
                                      int z_idx);
GivImageRgbAlpha16 giv_image_get_rgba_value(GivImage *img,
                                            int x_idx,
                                            int y_idx,
                                            int z_idx);
guchar *giv_image_get_buf(GivImage *img);

/** 
 * Get the type of the pixels of the image.
 * 
 * @return 
 */
GivImageType giv_image_get_type(GivImage *img);

/** 
 * Increase the objects reference count. This should be done by any method
 * that needs to work on the image.
 * 
 */
void giv_image_ref(GivImage *img);

/** 
 * Decrease the images reference count. If the reference goes to zero, then
 * the object is deleted. This should be done whenever a context no longer
 * needs a reference to the image.
 * 
 */
void giv_image_unref(GivImage *img);

// Some simple manipulations - only for gray images
void giv_image_get_min_max(GivImage *img,
                           // output
                           double* min,
                           double* max);


/** 
 * Get a map to the attribute map of the image.
 * 
 * @return 
 */
GHashTable *giv_image_get_attribute_map(GivImage *img);


/** 
 * Query if an attribute exists in the file.
 * 
 * @param key 
 * 
 * @return 
 */
gboolean giv_image_attribute_exist(GivImage *img,
                                   const char* key);
    
/** 
 * Set an attribute value. Only string values are supported. Other
 * values will have to be serialized to strings by the caller. No
 * known limitations exist on the length of the value. (E.g. a
 * giv string may be stored).
 * 
 * @param key 
 * @param value 
 */
void giv_image_set_attribute(GivImage *img,
                             const gchar* key,
                             const gchar* value);

/** 
 * Get the value associated with an attribute in the file. If the
 * value doesn't exist then an empty string is returned. Don't release.
 * 
 * @param key 
 * 
 * @return 
 */
const char* giv_image_get_attribute(GivImage *img,
                                    const gchar* key);
    
/** 
 * Get an attribute and convert it to a double. If the attribute doesn't
 * exist then the value 0 is returned.
 * 
 * @param key 
 * 
 * @return atof of the attribute or 0
 */
double giv_image_get_attribute_double(GivImage *img,
                                      const gchar* key);
    
/** 
 * Get an attribute and convert it to an int. If the attribute doesn't
 * exist then the value 0 is returned.
 * 
 * @param key 
 * 
 * @return atof of the attribute or 0
 */
int giv_image_attribute_int(GivImage *img,
                            const gchar* key);


/** 
 * Turn a giv image into a pixbuf.
 * 
 * @param img 
 * @param min 
 * @param max 
 * 
 * @return 
 */
GdkPixbuf *giv_image_get_pixbuf(GivImage *img,
                                int slice_idx,
                                double min, double max);

/** 
 * Get the size in bits of the various pixel types.
 * 
 * @param img_type 
 * 
 * @return 
 */
int giv_image_type_get_size(GivImageType img_type);

/** 
 * Set an indicator that this is a one bit image.
 * 
 * @param img
 * @param one_bit - Whether the image is a one bit image
 * 
 * @return 
 */
void giv_image_set_one_bit(GivImage *img, gboolean one_bit);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* GIVIMAGE */
