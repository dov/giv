/**
 * @file   GivImage.cpp
 * @author Dov Grobgeld <dov@orbotech.com>
 * @date   Wed Jan 11 18:45:11 2006
 * 
 * @brief  
 * 
 * 
 */
#include <stdio.h>
#include "GivImage.h"
#include <math.h>

GivImage::GivImage()
{
    width = height = depth = num_channels = rank = 0;
    buf.buf = NULL;
    ref_count = 1;
}

GivImage::~GivImage()
{
    // Do I have to cast??
    if (buf.buf)
        delete [] buf.buf;
}

int
GivImage::get_type_size(GivImageType ft)
{
    switch (ft) {
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
    }
    return 0;
}

void
GivImage::ref()
{
    ref_count++;
}

void
GivImage::unref()
{
    ref_count--;
    if (ref_count == 0)
        delete this;
}

GivImageType
GivImage::get_type()
{
    return img_type;
}

void
GivImage::set_size(GivImageType ft,
                   int width,
                   int height,
                   int depth,
                   int num_channels)
{
    int size = width * height;
    rank = 2;
    
    if (depth > 0) {
        rank++;
        size *= depth;
    }
    if (num_channels > 0) {
        // Have to upgrade depth to 1
        if (depth == 0) {
            depth = 1;
            rank++;
        }
            
        rank++;
        size *= num_channels;
    }

    buf.buf = new guint8[size * get_type_size(ft)/8];
    this->img_type = ft;
    this->width = width;
    this->height = height;
    this->depth = depth;
    this->num_channels = num_channels;
}

void *
GivImage::get_buf()
{
    return buf.buf;
}

int 
GivImage::get_rank()
{
    return rank;
}

int 
GivImage::get_width()
{
    return width;
}

int 
GivImage::get_height()
{
    return height;
}

int 
GivImage::get_depth()
{
    return depth;
}

int 
GivImage::get_num_channels()
{
    return num_channels;
}

void
GivImage::get_min_max(int channel_idx,
                      // output
                      double& min,
                      double& max)
{
    int x_idx, y_idx, z_idx;
    int z_max = depth, ch_max = num_channels;

    if (rank < 4)
        ch_max = 1;
    if (rank < 3)
        z_max = 1;

    min = HUGE;
    max = -HUGE;
    for (z_idx=0; z_idx < z_max; z_idx++) {
        for (y_idx=0; y_idx < height; y_idx++) {
            for (x_idx=0; x_idx < width; x_idx++) {
                double gl = get_pixel_value(x_idx, y_idx, z_idx, channel_idx);
                
                if (gl < min)
                    min = gl;
                if (gl > max)
                    max = gl;
            }
        }
    }
}

double GivImage::get_pixel_value(int col_idx, int row_idx, int depth_idx, int ch_idx)
{
    int idx = col_idx + width * (row_idx + height * (depth_idx  + ch_idx * depth));

    switch (img_type) {
    case GIVIMAGE_U8:
        return buf.buf[idx];
    case GIVIMAGE_U16:
        return buf.sbuf[idx];
    case GIVIMAGE_I16:
        return buf.isbuf[idx];
    case GIVIMAGE_I32:
        return buf.ulbuf[idx];
    case GIVIMAGE_FLOAT:
        return buf.fbuf[idx];
    case GIVIMAGE_DOUBLE:
        return buf.dbuf[idx];
    case GIVIMAGE_RGB_U8: 
        return
            (buf.rgb_buf[idx].red << 16)
            | (buf.rgb_buf[idx].green << 8)
            | (buf.rgb_buf[idx].blue);
    }
    return 0;
}

std::map<std::string,std::string>&
GivImage::get_attribute_map()
{
    return attribute_map;
}

void
GivImage::set_attribute(std::string key,
                    std::string value)
{
    attribute_map[key] = value;
}

