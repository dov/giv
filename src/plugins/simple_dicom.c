//======================================================================
//  dicom.c - A simple dicom loader. Should be replaced with dcmtk
//            based loader.
//
//  Dov Grobgeld <dov.grobgeld@gmail.com>
//  Mon Nov  9 06:28:09 2009
//----------------------------------------------------------------------

#include "../givimage.h"
#include <glib.h>
#include <string.h>
#include <math.h>

/* A lot of Dicom images are wrongly encoded. By guessing the endian
 * we can get around this problem.
 */
#define GUESS_ENDIAN 0

static void toggle_endian2 (guint16          *buf16,
                            gint              length);

static void
guess_and_set_endian2 (guint16 *buf16,
		       int length);

gboolean giv_plugin_supports_file(const char *filename,
                                  guchar *start_chunk,
                                  gint start_chunk_len)
{
    gboolean is_dicom = g_ascii_strncasecmp ((gchar*)start_chunk+0x80,
                                             "DICM",4) == 0;

    return is_dicom;
}

GivImage *giv_plugin_load_file(const char *filename,
                               GError **error)
{
  FILE           *DICOM;
  gchar           buf[500];    /* buffer for random things like scanning */
  gint            width             = 0;
  gint            height            = 0;
  gint            samples_per_pixel = 0;
  gint            bpp               = 0;
  guint8         *pix_buf           = NULL;
  gboolean        toggle_endian     = FALSE;

  /* open the file */
  DICOM = fopen (filename, "rb");

  if (!DICOM)
      return NULL;

  /* Parse the file */
  fread (buf, 1, 128, DICOM); /* skip past buffer */

  /* Check for unsupported formats */
  if (g_ascii_strncasecmp (buf, "PAPYRUS", 7) == 0) {
      fclose(DICOM);
      return NULL;
  }

  fread (buf, 1, 4, DICOM); /* This should be dicom */
  if (g_ascii_strncasecmp (buf,"DICM",4) != 0) {
      fclose(DICOM);
      return NULL;
  }

  while (!feof (DICOM)) {
      guint16  group_word;
      guint16  element_word;
      gchar    value_rep[3];
      guint32  element_length;
      guint32  ctx_ul;
      guint16  ctx_us;
      guint8  *value;
      guint32  tag;
      gboolean do_toggle_endian= FALSE;
      gboolean implicit_encoding = FALSE;

      if (fread (&group_word, 1, 2, DICOM) == 0)
	break;
      group_word = g_ntohs (GUINT16_SWAP_LE_BE (group_word));

      fread (&element_word, 1, 2, DICOM);
      element_word = g_ntohs (GUINT16_SWAP_LE_BE (element_word));

      tag = (group_word << 16) | element_word;
      fread(value_rep, 2, 1, DICOM);
      value_rep[2] = 0;

      /* Check if the value rep looks valid. There probably is a
         better way of checking this...
       */
      if ((/* Always need lookup for implicit encoding */
	   tag > 0x0002ffff && implicit_encoding)
	  /* This heuristics isn't used if we are doing implicit
	     encoding according to the value representation... */
	  || ((value_rep[0] < 'A' || value_rep[0] > 'Z'
	  || value_rep[1] < 'A' || value_rep[1] > 'Z')

	  /* Is this a bug?
	  */
	      && !(value_rep[0] == ' ' && value_rep[1]))
          )
        {
          /* Look up type from the dictionary. At the time we dont
	     support this option... */
          gchar element_length_chars[4];

          /* Store the bytes that were read */
          element_length_chars[0] = value_rep[0];
          element_length_chars[1] = value_rep[1];

	  /* Unknown value rep. It is not used right now anyhow */
	  strcpy (value_rep, "??");

          /* For implicit value_values the length is always four bytes,
             so we need to read another two. */
          fread (&element_length_chars[2], 1, 2, DICOM);

          /* Now cast to integer and insert into element_length */
          element_length =
            g_ntohl (GUINT32_SWAP_LE_BE (*((gint *) element_length_chars)));
      }
      /* Binary value reps are OB, OW, SQ or UN */
      else if (strncmp (value_rep, "OB", 2) == 0
	  || strncmp (value_rep, "OW", 2) == 0
	  || strncmp (value_rep, "SQ", 2) == 0
	  || strncmp (value_rep, "UN", 2) == 0)
	{
	  fread (&element_length, 1, 2, DICOM); /* skip two bytes */
	  fread (&element_length, 1, 4, DICOM);
	  element_length = g_ntohl (GUINT32_SWAP_LE_BE (element_length));
	}
      /* Short length */
      else {
	  guint16 el16;

	  fread (&el16, 1, 2, DICOM);
	  element_length = g_ntohs (GUINT16_SWAP_LE_BE (el16));
      }

      /* Sequence of items - just ignore the delimiters... */
      if (element_length == 0xffffffff)
          continue;

      /* Sequence of items item tag... Ignore as well */
      if (tag == 0xFFFEE000)
          continue;

      /* Read contents. Allocate a bit more to make room for casts to int
       below. */
      value = g_new0 (guint8, element_length + 4);
      fread (value, 1, element_length, DICOM);

      /* Some special casts that are used below */
      ctx_ul = *(guint32 *) value;
      ctx_us = *(guint16 *) value;

      /* Recognize some critical tags */
      if (group_word == 0x0002) {
          switch (element_word) {
            case 0x0010:   /* transfer syntax id */
              if (strcmp("1.2.840.10008.1.2", (char*)value) == 0) {
                  do_toggle_endian = FALSE;
                  implicit_encoding = TRUE;
              }
              else if (strcmp("1.2.840.10008.1.2.1", (char*)value) == 0)
                  do_toggle_endian = FALSE;
              else if (strcmp("1.2.840.10008.1.2.2", (char*)value) == 0)
                  do_toggle_endian = TRUE;
              break;
          }
      }
      else if (group_word == 0x0028) {
	  switch (element_word) {
          case 0x0002:  /* samples per pixel */
	      samples_per_pixel = ctx_us;
	      break;
          case 0x0010:  /* rows */
	      height = ctx_us;
	      break;
          case 0x0011:  /* columns */
	      width = ctx_us;
	      break;
          case 0x0100:  /* bits_allocated */
	      bpp = ctx_us;
	      break;
          case 0x0103:  /* pixel representation */
	      toggle_endian = ctx_us;
	      break;
          }
      }

      /* Pixel data */
      if (group_word == 0x7fe0 && element_word == 0x0010) {
	  pix_buf = value;
      }
      else {
          g_free (value);
      }
  }
  fclose(DICOM);

#if GUESS_ENDIAN
  if (bpp == 16)
    guess_and_set_endian2 ((guint16 *) pix_buf, width * height);
#endif

  // Create the image
  GivImage *img = NULL;
  switch (bpp) {
  case 8:
      img = giv_image_new(GIVIMAGE_U8,width,height);
      memcpy(img->buf.buf, pix_buf, (uint64_t)width*height);
      break;
  case 16:
      img = giv_image_new(GIVIMAGE_U16,width,height);
      memcpy(img->buf.buf, pix_buf, (uint64_t)width*height*2);
  default:
      ;
  }

  g_free(pix_buf);
  
  return img;
}

static void
guess_and_set_endian2 (guint16 *buf16,
		       int length)
{
  guint16 *p          = buf16;
  gint     max_first  = -1;
  gint     max_second = -1;

  while (p<buf16+length)
    {
      if (*(guint8*)p > max_first)
        max_first = *(guint8*)p;
      if (((guint8*)p)[1] > max_second)
        max_second = ((guint8*)p)[1];
      p++;
    }

  if (   ((max_second > max_first) && (G_BYTE_ORDER == G_LITTLE_ENDIAN))
         || ((max_second < max_first) && (G_BYTE_ORDER == G_BIG_ENDIAN)))
    toggle_endian2 (buf16, length);
}

/* toggle_endian2 toggles the endian for a 16 bit entity.  */
static void
toggle_endian2 (guint16 *buf16,
	        gint     length)
{
  guint16 *p = buf16;

  while (p < buf16 + length)
    {
      *p = ((*p & 0xff) << 8) | (*p >> 8);
      p++;
    }
}
