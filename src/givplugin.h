#ifndef GIVPLUGIN_H
#define GIVPLUGIN_H

#include <glib.h>
#include "givimage.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

// In giv-image.h
#ifdef _WIN32
  #ifdef GIV_EXPORTS
    #define GIV_API __declspec(dllexport)
  #else
    #define GIV_API __declspec(dllimport)
  #endif
#else
  #define GIV_API
#endif

#define GIV_PLUGIN_ERROR               (giv_plugin_error_quark ())
GIV_API GQuark giv_plugin_error_quark (void);
enum {
  GLIB_PLUGIN_ERROR_UNKNOWN
};

// A structure for matching contents.
typedef struct {
    // Can the format be matched by contents?
    gboolean by_contents;

    // Start of matching
    gint contents_offset;

    // Regular expression for matching
    const gchar *content_match_expression;

    // Can the format be matched by filename
    gboolean by_filename;

    const gchar *filename_match_expression;
} giv_plugin_support_t;    

/** 
 * Get a freshly allocated plugin structure. TBD.
 * 
 * 
 * @return 
 */
GIV_API giv_plugin_support_t *get_plugin_support();

GIV_API GivImage *giv_plugin_load_image(const char *filename,
                                GError **error);

GIV_API gboolean giv_plugin_supported_file(const char *filename);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* GIVPLUGIN */
