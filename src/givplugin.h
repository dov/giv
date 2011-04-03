#ifndef GIVPLUGIN_H
#define GIVPLUGIN_H

#include <glib.h>
#include "givimage.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

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
giv_plugin_support_t *get_plugin_support();

GivImage *giv_plugin_load_image(const char *filename,
                                GError **error);

gboolean giv_plugin_supported_file(const char *filename);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* GIVPLUGIN */
