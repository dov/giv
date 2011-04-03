//======================================================================
//  GivImage.h - A multi-layer image for Giv.
//
//  Dov Grobgeld <dov.grobgeld@gmail.com>
//  Sat Nov 17 19:09:38 2007
//----------------------------------------------------------------------

/**
 * @file   GivImage.h
 * @author Dov Grobgeld <dov@orbotech.com>
 * @date   Wed Jan 11 18:29:35 2006
 * 
 * @brief  
 * 
 * 
 */
#ifndef GIVIMAGE_H
#define GIVIMAGE_H

#include <string>
#include <map>
#include <glib.h>

/** 
 * This enum defines the types supported by GivImage
 * 
 * @return 
 */
enum GivImageType {
    GIVIMAGE_U8,
    GIVIMAGE_U16,
    GIVIMAGE_I16,
    GIVIMAGE_I32,
    GIVIMAGE_FLOAT,
    GIVIMAGE_DOUBLE,
    GIVIMAGE_RGB_U8
};

struct GivImageRgb {
    guint8 red, green, blue;
};

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
class GivImage {
 public:
    GivImage();
    GivImage(GivImageType ft, int width, int height, int depth=0, int ch_idx=0);
    ~GivImage();

    /** 
     * Get number of dimensions. This is always a number between 2 and 4.
     * 
     * 
     * @return 
     */
    int get_rank();

    /** 
     * Get image width.
     * 
     * 
     * @return 
     */
    int get_width();

    /** 
     * Get image height.
     * 
     * 
     * @return 
     */
    int get_height();

    /** 
     * Get the number of channels of the image. This is equal to the number
     * of colors of the image.
     * 
     * @return 
     */
    int get_num_channels();

    /** 
     * Get the depth of the image.
     * 
     * @return 
     */
    int get_depth();

    /** 
     * Get the value of a pixel.
     * 
     * @param col_idx 
     * @param row_idx 
     * @param depth_idx 
     * @param ch_idx 
     * 
     * @return 
     */
    double get_pixel_value(int col_idx,
                           int row_idx,
                           int depth_idx=0,
                           int ch_idx=0);
    /** 
     * Get a reference to the pixel data. The user has to cast this memory
     * on her own in order according to the result of get_type().
     * 
     * @return 
     */
    void *get_buf();

    /** 
     * Get the type of the pixels of the image.
     * 
     * @return 
     */
    GivImageType get_type();

    /** 
     * Set and reallocate the memory of the image.
     * 
     * @param ft 
     * @param width 
     * @param height 
     * @param depth 
     * @param num_channels 
     */
    void set_size(GivImageType ft, int width, int height, int depth=0, int num_channels=0);

    /** 
     * Get the size in bits of the type stored by a pixel in the image.
     * 
     * @param ft 
     * 
     * @return 
     */
    int get_type_size(GivImageType ft);

    /** 
     * Increase the objects reference count. This should be done by any method
     * that needs to work on the image.
     * 
     */
    void ref();

    /** 
     * Decrease the images reference count. If the reference goes to zero, then
     * the object is deleted. This should be done whenever a context no longer
     * needs a reference to the image.
     * 
     */
    void unref();

    // Some simple manipulations
    void get_min_max(int ch_idx,
                     // output
                     double& min,
                     double& max);

    /** 
     * Get a map to the attribute map of the image.
     * 
     * @return 
     */
    std::map<std::string,std::string>& get_attribute_map();

    /** 
     * Query if an attribute exists in the file.
     * 
     * @param key 
     * 
     * @return 
     */
    bool attribute_exist(std::string key);
    
    /** 
     * Set an attribute value. Only string values are supported. Other
     * values will have to be serialized to strings by the caller. No
     * known limitations exist on the length of the value. (E.g. a
     * giv string may be stored).
     * 
     * @param key 
     * @param value 
     */
    void set_attribute(std::string key,
                       std::string value);

    /** 
     * Get the value associated with an attribute in the file. If the
     * value doesn't exist then an empty string is returned.
     * 
     * @param key 
     * 
     * @return 
     */
    std::string get_attribute(std::string key);
    
    /** 
     * Get an attribute and convert it to a double. If the attribute doesn't
     * exist then the value 0 is returned.
     * 
     * @param key 
     * 
     * @return atof of the attribute or 0
     */
    double get_attribute_double(std::string key);
    
    /** 
     * Get an attribute and convert it to an int. If the attribute doesn't
     * exist then the value 0 is returned.
     * 
     * @param key 
     * 
     * @return atof of the attribute or 0
     */
    int get_attribute_int(std::string key);

    /** 
     * Like get_attribute() but the user provides their own default
     * value if the key doesn't exist.
     * 
     * @param key 
     * @param default_value 
     * 
     * @return 
     */
    double get_attribute_double_default(std::string key,
                                        double default_value);
    /** 
     * Like get_attribute_int() but the user provides their own default
     * value if the key doesn't exist.
     * 
     * @param key 
     * @param default_value 
     * 
     * @return 
     */
    double get_attribute_int(std::string key,
                             int default_value);
    
 private:
    GivImageType img_type;
    int rank;
    int width;
    int height;
    int num_channels; // Number of color channels
    int depth;
    union {
        guint8 *buf;
        guint16 *sbuf;
        gint16 *isbuf;
        gint32 *ulbuf;
        float *fbuf;
        double *dbuf;
        GivImageRgb *rgb_buf;
    } buf;
    int ref_count; // reference count
    std::map<std::string, std::string> attribute_map;
};

#endif /* GIVIMAGE */

