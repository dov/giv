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
#ifndef GLIB_JSONRPC_JSON_H
#define GLIB_JSONRPC_JSON_H

#include <json-glib/json-glib.h>

#ifdef __cplusplus
extern "C" {
#endif

// Allocate and convert a json node to a string
gchar *glib_jsonrpc_json_to_string(JsonNode *node);

// Convenience function for turning a string list into a newly allocated json array
JsonNode *glib_jsonrpc_json_csv_to_json_array(const char *string);

// A general json parser
JsonNode *glib_jsonrpc_json_string_to_json_node(const gchar *str);


#ifdef __cplusplus
}
#endif

#endif /* GLIB_JSONRPC_JSON */
