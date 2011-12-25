/* test-glib-jsonrpc-json.c: Utilities for the GlibJsonRPC library.
 *
 * Copyright (C) 2011 Dov Grobgeld <dov.grobgeld@gmail.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 *
 */
#include <stdio.h>
#include "glib-jsonrpc-json.h"

/** 
 * Utility function for turning a json node into a string.
 */
gchar *glib_jsonrpc_json_to_string(JsonNode *node)
{
  // Serialize response into content_string
  JsonGenerator *gen = json_generator_new ();
  gsize len;
  json_generator_set_root (gen, node);
  gchar *json_string = json_generator_to_data(gen, &len);
  g_object_unref (gen);
  return json_string;
}

// Parse a json object, a json object stripped of {} or an json array
// stripped of [] into a JsonNode.
JsonNode *glib_jsonrpc_json_string_to_json_node(const gchar *str)
{
  JsonParser *parser = json_parser_new();
  GError *error;

  // First try to parse the string
  if (!json_parser_load_from_data(parser, str, -1, &error))
    {
      // Wrap it in a { } pair and try again.
      g_error_free(error); error = NULL;
      GString *j_str = g_string_new("");
      g_string_append_printf(j_str, "{%s}", str);

      // Try parsing it as an object
      if (!json_parser_load_from_data(parser, j_str->str, -1, &error))
        {
          // Still fail, try to parse it as an array
          g_string_free(j_str, TRUE);
          g_string_append_printf(j_str, "[%s]", str);
          if (!json_parser_load_from_data(parser, j_str->str, -1, &error))
            {
              // That's it, we give up.
              g_object_unref(parser);
              return NULL;
            }
        }
    }

  JsonNode *node = json_node_copy(json_parser_get_root(parser));
  g_object_unref(parser);
  return node;
}

JsonNode *glib_jsonrpc_json_csv_to_json_array(const gchar *str)
{
  gchar **str_list = g_strsplit(str, ",", -1);

  JsonArray *array = json_array_new();
  gchar **p = str_list;
  while(*p)
    {
      JsonNode *node = json_node_new(JSON_NODE_VALUE);
      json_node_set_string(node, *p);
      json_array_add_element(array,node);
      p++;
    }
  JsonNode *node = json_node_new(JSON_NODE_ARRAY);
  json_node_take_array(node, array);
  g_strfreev(str_list);
  return node;
}
