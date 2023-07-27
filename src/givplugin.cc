//======================================================================
//  givplugin.c - 
//
//  Dov Grobgeld <dov.grobgeld@gmail.com>
//  Sat Nov  7 21:38:48 2009
//----------------------------------------------------------------------
#include "givplugin.h"
#include <string.h>
#include <stdlib.h>
#include "givplugin.h"
#include <spdlog/spdlog.h>

#define REQ_CHUNK_SIZE 1000

static gboolean giv_image_loaded_loaders = FALSE;
static GSList *givimage_loaders = NULL;

typedef gboolean (* SupportsFile) (const char *filename,
                                   guchar *start_chunk,
                                   gint start_chunk_len);
typedef GivImage* (* LoadFile)(const char *filename,
                               GError **error);

// TBD: Support a comma separated list of plugin directories.
static void rehash_loaders()
{
    GError *error = NULL;
    const gchar * giv_plugin_dir = PACKAGE_PLUGIN_DIR;
#ifdef G_OS_WIN32
    // Hardcoded path relative to installation dir on Windows
    gchar giv_win32_plugin_dir[512];
    sprintf(giv_win32_plugin_dir, "%s/plugins",
            g_win32_get_package_installation_directory_of_module(NULL));
    giv_plugin_dir = giv_win32_plugin_dir;
#endif

    if (getenv("GIV_PLUGIN_DIR"))
        giv_plugin_dir = (const char*)getenv("GIV_PLUGIN_DIR");

    // No matter of we succeed or not, we will not automatically
    // call this function again.
    giv_image_loaded_loaders = TRUE;
    GDir *plugin_dir = g_dir_open(giv_plugin_dir,
                                  0,
                                  &error);

    // No directory found, ok, just ignore it.
    if (error) {
        fprintf(stderr, "plugin dir error: %s\n", error->message);
        g_error_free(error);
        return;
    }

    const gchar *name;
    while( (name=g_dir_read_name(plugin_dir)) ) {
        // Try to load it as a module if it ends with ".dll"
        // or ".so".
        gchar *extension = g_strrstr(name, ".");
        if (!extension)
            continue;
        extension++;
        if (g_ascii_strncasecmp(G_MODULE_SUFFIX,
                                extension,
                                strlen(G_MODULE_SUFFIX))==0) {
            gchar *module_name = g_strndup(name, extension-name-1);
            gchar *module_path = g_strdup_printf("%s/%s",
                                                 giv_plugin_dir,
                                                 name);

            GModule *module = g_module_open (module_path, G_MODULE_BIND_MASK);

            if (module) {
                givimage_loaders = g_slist_prepend(givimage_loaders,
                                                   module);
                spdlog::info("Successfully loaded {}", module_path);
            }
            else {
                fprintf(stderr, "Failed loading module %s\n", module_path);
                spdlog::error("Failed opening {}", module_path);
            }

            g_free(module_path);
            g_free(module_name);
        }
    }
    g_dir_close(plugin_dir);
}

gboolean giv_plugin_supported_file(const char *filename)
{
    gboolean supported = FALSE;
    guchar *chunk = NULL;
    gint chunk_len = 0;
    if (!giv_image_loaded_loaders)
        rehash_loaders();

    // Currently don't use chunk matching as it seriously slows
    // down slow networks!!!
#if 0 
    // Read a chunk from the file
    FILE *fh = fopen(filename, "rb");
    if (!fh)
        return FALSE;

    chunk = g_new0(guchar, REQ_CHUNK_SIZE);
    chunk_len = fread(chunk, 1, REQ_CHUNK_SIZE, fh);
    fclose(fh);
#endif

    GSList *ploaders = givimage_loaders;
    while(ploaders) {
        SupportsFile sup_file;
        GModule *module = (GModule*)(ploaders->data);
        g_module_symbol(module, "giv_plugin_supports_file", (gpointer*)&sup_file);
        if (sup_file && sup_file(filename,
                                 chunk,
                                 chunk_len)) {
            spdlog::info("Found plugin match for {}", filename);
            supported = TRUE;
            break;
        }
        ploaders = ploaders->next;
    }
    if (chunk)
      g_free(chunk);
    
    return supported;
}

GivImage *giv_plugin_load_image(const char *filename,
                                GError **error)
{
    GivImage *img = NULL;
    guchar *chunk = NULL;
    gint chunk_len = 0;
    if (!giv_image_loaded_loaders)
        rehash_loaders();

#if 0
    // Read a chunk from the file
    FILE *fh = fopen(filename, "rb");
    if (!fh)
        return NULL;

    chunk = g_new0(guchar, REQ_CHUNK_SIZE);
    chunk_len = fread(chunk, 1, REQ_CHUNK_SIZE, fh);
    
    fclose(fh);

#endif

    GSList *ploaders = givimage_loaders;
    while(ploaders) {
        SupportsFile sup_file;
        LoadFile load_file;
        GModule *module = (GModule*)(ploaders->data);
        g_module_symbol(module, "giv_plugin_supports_file", (gpointer*)&sup_file);
        g_module_symbol(module, "giv_plugin_load_file", (gpointer*)&load_file);
        if (sup_file && load_file) {
            if (sup_file(filename,
                         chunk,
                         chunk_len)) {
                img = load_file(filename,
                                error);
                if (*error)
                {
                  spdlog::error("Failed loading {} error {}\n",
                                filename, (*error)->message); 
                  
                }
                else if (!img)
                  spdlog::error("Oops! Programming error! Did not find an image in {}, but no error was given!\n", filename); 
                else
                  spdlog::info("Loaded {} with size width={} height={} rank={} depth={} bit_size={}",
                               filename,
                               giv_image_get_width(img),
                               giv_image_get_height(img),
                               giv_image_get_rank(img),
                               giv_image_get_depth(img),
                               giv_image_type_get_size(giv_image_get_type(img)));

                // TBD - handle plugin errors
                break;
            }
        }
        else {
            // TBD - error about invalid module
        }
        ploaders = ploaders->next;
    }
    g_free(chunk);
    
    return img;
}

GQuark
giv_plugin_error_quark (void)
{
  return g_quark_from_static_string ("giv-plugin");
}
