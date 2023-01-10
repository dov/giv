//======================================================================
//  dicom.c - A dicom loader based on dcmtk.
//
//  Dov Grobgeld <dov.grobgeld@gmail.com>
//  Mon Nov  9 06:28:09 2009
//----------------------------------------------------------------------

#include <stdio.h>
#include "dcmtk/dcmdata/dcfilefo.h"
#include "dcmtk/dcmdata/dcdeftag.h"
#include "../givimage.h"
#include "../givplugin.h"
#include <glib.h>
#include <sstream>
#include <string.h>
#include <math.h>
#include "plis/plis.h"

using namespace plis;
using namespace std;

/* A lot of Dicom images are wrongly encoded. By guessing the endian
 * we can get around this problem.
 */
#define GUESS_ENDIAN 0

extern "C" gboolean giv_plugin_supports_file(const char *filename,
                                             guchar *start_chunk,
                                             gint start_chunk_len)
{
    gboolean is_dicom = false;

    if (start_chunk && start_chunk_len >= 0x84)
      is_dicom = g_ascii_strncasecmp ((gchar*)start_chunk+0x80,
                                      "DICM",4) == 0;
    if (!is_dicom)
        is_dicom = g_ascii_strncasecmp((gchar*)filename+strlen(filename)-4,
                                       ".dcm",4)==0;

    return is_dicom;
}

extern "C" GivImage *giv_plugin_load_file(const char *filename,
                                          GError **error)
{
    DcmFileFormat dcm_img;
    OFCondition status = dcm_img.loadFile(filename);
    DcmDataset* ds = dcm_img.getDataset(); // shortcut

    long int width, height, spp, bpp, num_frames=1;
    ds->findAndGetLongInt(DCM_Columns,         width);
    ds->findAndGetLongInt(DCM_Rows,            height);
    ds->findAndGetLongInt(DCM_SamplesPerPixel, spp);
    OFCondition ret1 = ds->findAndGetLongInt(DCM_BitsAllocated,   bpp);
    if (ds->findAndGetLongInt(DCM_NumberOfFrames,  num_frames).status() != OF_ok)
        num_frames = 1;

    guint8 *buf = NULL;
    slip pnm_id;
    if (bpp == 16 && spp == 1) {
	// get pixel data
	ds->findAndGetUint16Array(DCM_PixelData, (const Uint16*&)buf);
    }
    else if (bpp == 8 && spp == 1) {
	// get pixel data
	ds->findAndGetUint8Array(DCM_PixelData, (const Uint8*&)buf);
    }
    else
        printf("Unsupported dicom bpp=%d spp=%d!\n", (int)bpp, (int)spp);

    if (buf == NULL) {
        g_set_error (error,
                     GIV_PLUGIN_ERROR,
                     GLIB_PLUGIN_ERROR_UNKNOWN,
                     ("Failed finding pixel memory in dicom file"));
        return NULL;
    }
        
    // Create the image
    GivImage *img = NULL;
    GivImageType type =  GIVIMAGE_U8;
    if (bpp == 16)
        type = GIVIMAGE_U16;
    int wsize = giv_image_type_get_size(type)/8;  /* Size in bytes */
    int depth = num_frames;

    img =  giv_image_new_full(type,
                              width,
                              width * wsize,
                              height,
                              height * wsize * width,
                              2, // rank - why is it always 2?
                              depth);

    if (img)
        memcpy(img->buf.buf, buf, width*height*num_frames*bpp/8);
    if (!img) 
      return NULL;

    // Insert attributes in the hash map
    DcmObject *dset = &dcm_img;
    ostringstream sout;
    dset->print(sout, DCMTypes::PF_shortenLongTagValues);

    // TBD - split and clean up the sout output
    gchar **lines = g_strsplit(sout.str().c_str(),
                               "\n",
                               -1);
    GString *cleaned_up_string = g_string_new("");
    gchar **p = lines;
    while(*p) {
        if (g_strrstr(*p, "Unknown") == NULL) {
            g_string_append(cleaned_up_string, *p);
            g_string_append(cleaned_up_string, "\n");
        }
        p++;
    }
    giv_image_set_attribute(img, "attributes", cleaned_up_string->str);
    g_string_free(cleaned_up_string, TRUE);

    return img;
}

