//======================================================================
//  dicom.c - A simple dicom loader. Should be replaced with dcmtk
//            based loader.
//
//  Dov Grobgeld <dov.grobgeld@gmail.com>
//  Mon Nov  9 06:28:09 2009
//----------------------------------------------------------------------

#include <stdio.h>
#include "dcmtk/dcmdata/dcfilefo.h"
#include "dcmtk/dcmdata/dcdeftag.h"
#include "../givimage.h"
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

static void toggle_endian2 (guint16          *buf16,
                            gint              length);

static void
guess_and_set_endian2 (guint16 *buf16,
		       int length);

extern "C" gboolean giv_plugin_supports_file(const char *filename,
                                             guchar *start_chunk,
                                             gint start_chunk_len)
{
    gboolean is_dicom = g_ascii_strncasecmp ((gchar*)start_chunk+0x80,
                                             "DICM",4) == 0;
    if (!is_dicom)
        is_dicom = g_ascii_strncasecmp((gchar*)filename+strlen(filename)-4,
                                       ".dcm",4)==0;

    return is_dicom;
}

extern "C" GivImage *giv_plugin_load_file(const char *filename)
{
    DcmFileFormat dcm_img;
    OFCondition status = dcm_img.loadFile(filename);
    DcmDataset* ds = dcm_img.getDataset(); // shortcut

    long int width, height, spp, bpp;
    ds->findAndGetLongInt(DCM_Columns,         width);
    ds->findAndGetLongInt(DCM_Rows,            height);
    ds->findAndGetLongInt(DCM_SamplesPerPixel, spp);
    ds->findAndGetLongInt(DCM_BitsAllocated,   bpp);

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
        printf("Unsupported dicom!\n");

    // Create the image
    GivImage *img = NULL;
    switch (bpp) {
    case 8:
        img = giv_image_new(GIVIMAGE_U8,width,height);
        if (img)
            memcpy(img->buf.buf, buf, width*height);
        break;
    case 16:
        img = giv_image_new(GIVIMAGE_U16,width,height);
        if (img)
            memcpy(img->buf.buf, buf, width*height*2);
    default:
        ;
    }
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

