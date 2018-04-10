/* json-glib-format - Formats JSON data
 * 
 * This file is part of JSON-GLib
 *
 * Copyright © 2013  Emmanuele Bassi
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library. If not, see <http://www.gnu.org/licenses/>.
 *
 * Author:
 *   Emmanuele Bassi  <ebassi@gnome.org>
 */

#include "config.h"

#ifdef G_OS_UNIX
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#endif
#include <fcntl.h>
#ifdef G_OS_WIN32
#include <windows.h>
#endif

#include <stdlib.h>
#include <stdio.h>
#include <locale.h>
#include <errno.h>

#include <glib.h>
#include <glib/gi18n.h>
#include <glib/gstdio.h>
#include <json-glib/json-glib.h>

#if defined (G_OS_WIN32) && !defined (HAVE_UNISTD_H)
#include <io.h>

#define STDOUT_FILENO 1
#endif

static char **files = NULL;
static char *output = NULL;
static gboolean prettify = FALSE;
static int indent_spaces = 2;

static GOptionEntry entries[] = {
  { "prettify", 'p', 0, G_OPTION_ARG_NONE, &prettify, N_("Prettify output"), NULL },
  { "indent-spaces", 'i', 0, G_OPTION_ARG_INT, &indent_spaces, N_("Indentation spaces"), N_("SPACES") },
  { "output", 'o', 0, G_OPTION_ARG_FILENAME, &output, N_("Output file"), N_("FILE") },
  { G_OPTION_REMAINING, 0, 0, G_OPTION_ARG_FILENAME_ARRAY, &files, NULL, N_("FILE…") },
  { NULL },
};

static gboolean
format (JsonParser    *parser,
        JsonGenerator *generator,
        GFile         *file)
{
  GInputStream *in;
  GError *error;
  gboolean res = TRUE;
  gboolean parse_res;
  gboolean close_res;
  char *data, *p;
  gsize len;
  int fd = -1;

  error = NULL;

  in = (GInputStream *) g_file_read (file, NULL, &error);
  if (in == NULL)
    {
      /* Translators: the first %s is the program name, the second one
       * is the URI of the file, the third is the error message.
       */
      g_printerr (_("%s: %s: error opening file: %s\n"),
                  g_get_prgname (), g_file_get_uri (file), error->message);
      g_error_free (error);
      return FALSE;
    }

  parse_res = json_parser_load_from_stream (parser, in, NULL, &error);
  if (!parse_res)
    {
      /* Translators: the first %s is the program name, the second one
       * is the URI of the file, the third is the error message.
       */
      g_printerr (_("%s: %s: error parsing file: %s\n"),
                  g_get_prgname (), g_file_get_uri (file), error->message);
      g_clear_error (&error);
      res = FALSE;
      goto out;
    }

  json_generator_set_root (generator, json_parser_get_root (parser));
  data = json_generator_to_data (generator, &len);

  if (output == NULL)
    fd = STDOUT_FILENO;
  else
    {
      int sv_errno;

      fd = g_open (output, O_CREAT | O_WRONLY, 0666);
      if (fd < 0)
        {
          sv_errno = errno;

          g_printerr (_("%s: %s: error opening file: %s\n"),
                      g_get_prgname (), output, g_strerror (sv_errno));
          res = FALSE;
          goto out;
        }
    }

  p = data;

  while (len > 0)
    {
      gssize written = write (fd, p, len);

      if (written == -1 && errno != EINTR)
        {
          /* Translators: the first %s is the program name, the
           * second one is the URI of the file.
           */
          g_printerr (_("%s: %s: error writing to stdout"), g_get_prgname (), g_file_get_uri (file));
          res = FALSE;
          goto out;
        }

      len -= written;
      p += written;
    }

  if (write (fd, "\n", 1) < 0)
    g_error ("%s: %s", g_get_prgname (), g_strerror (errno));

  g_free (data);

out:
  close_res = g_input_stream_close (in, NULL, &error);
  if (!close_res)
    {
      /* Translators: the first %s is the program name, the second one
       * is the URI of the file, the third is the error message.
       */
      g_printerr (_("%s: %s: error closing: %s\n"),
                  g_get_prgname (), g_file_get_uri (file), error->message);
      g_clear_error (&error);
      res = FALSE;
    }

  if (fd != STDOUT_FILENO)
    g_close (fd, NULL);

  return res;
}

int
main (int   argc,
      char *argv[])
{
  GOptionContext *context = NULL;
  GError *error = NULL;
  const char *description;
  const char *summary;
  JsonParser *parser;
  JsonGenerator *generator;
  gboolean res;
  int i;

  setlocale (LC_ALL, "");

  bindtextdomain (GETTEXT_PACKAGE, JSON_LOCALEDIR);
  bind_textdomain_codeset (GETTEXT_PACKAGE, "UTF-8");
  textdomain (GETTEXT_PACKAGE);

  /* Translators: this message will appear after the usage string */
  /* and before the list of options.                              */
  summary = _("Format JSON files.");
  description = _("json-glib-format formats JSON resources.");

  context = g_option_context_new (NULL);
  g_option_context_set_summary (context, summary);
  g_option_context_set_description (context, description);
  g_option_context_add_main_entries (context, entries, GETTEXT_PACKAGE);
  g_option_context_parse (context, &argc, &argv, &error);
  g_option_context_free (context);

  if (error != NULL)
    {
      /* Translators: the %s is the program name. This error message
       * means the user is calling json-glib-validate without any
       * argument.
       */
      g_printerr (_("Error parsing commandline options: %s\n"), error->message);
      g_printerr ("\n");
      g_printerr (_("Try “%s --help” for more information."), g_get_prgname ());
      g_printerr ("\n");
      g_error_free (error);
      return 1;
    }

  if (files == NULL)
    {
      /* Translators: the %s is the program name. This error message
       * means the user is calling json-glib-validate without any
       * argument.
       */
      g_printerr (_("%s: missing files"), g_get_prgname ());
      g_printerr ("\n");
      g_printerr (_("Try “%s --help” for more information."), g_get_prgname ());
      g_printerr ("\n");
      return 1;
    }

  generator = json_generator_new ();
  json_generator_set_pretty (generator, prettify);
  json_generator_set_indent (generator, indent_spaces);

  parser = json_parser_new ();
  res = TRUE;
  i = 0;

  do
    {
      GFile *file = g_file_new_for_commandline_arg (files[i]);

      res = format (parser, generator, file) && res;
      g_object_unref (file);
    }
  while (files[++i] != NULL);

  g_object_unref (parser);
  g_object_unref (generator);

  return res ? EXIT_SUCCESS : EXIT_FAILURE;
}
