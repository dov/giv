/* GlibJsonRpcClient
 * glib-jsonrpc-server.h: A Json RCP client in glib.
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
#include <string.h>
#include "glib-jsonrpc-client.h"
#include "glib-jsonrpc-json.h"

typedef struct {
  GLibJsonRpcClient parent;

  GSocketClient *client;
  GSocketConnection *connection;
} GLibJsonRpcClientPrivate; 

GLibJsonRpcClient *glib_jsonrpc_client_new(const char *hostname, int port)
{
  GLibJsonRpcClientPrivate *selfp = g_new0(GLibJsonRpcClientPrivate, 1);

  GResolver *resolver = g_resolver_get_default();
  GList *addresses = g_resolver_lookup_by_name(resolver,hostname,NULL,NULL);
  if (!addresses)
    {
      glib_jsonrpc_client_free((GLibJsonRpcClient*)selfp);
      return NULL;
    }
  GInetAddress *address = (GInetAddress*)g_list_nth_data(addresses, 0);

  selfp->client = g_socket_client_new();
  GSocketAddress *sockaddr = g_inet_socket_address_new(address, port);
  selfp->connection = g_socket_client_connect (selfp->client, G_SOCKET_CONNECTABLE(sockaddr), NULL, NULL);
  g_resolver_free_addresses(addresses);

  if (!selfp->connection)
    {
      glib_jsonrpc_client_free((GLibJsonRpcClient*)selfp);
      return NULL;
    }
    
  return (GLibJsonRpcClient*)selfp;
}

void glib_jsonrpc_client_free(GLibJsonRpcClient *client)
{
  GLibJsonRpcClientPrivate *selfp = (GLibJsonRpcClientPrivate*)client;
  if (selfp->connection)
    g_object_unref(G_OBJECT(selfp->connection));
  if (selfp->client)
    g_object_unref(G_OBJECT(selfp->client));
  g_free(client);
}

int glib_jsonrpc_client_call(GLibJsonRpcClient *client,
                             const char *method,
                             JsonNode *params,
                             // output
                             JsonNode **response)
{
  GLibJsonRpcClientPrivate *selfp = (GLibJsonRpcClientPrivate*)client;
  GOutputStream *out = g_io_stream_get_output_stream (G_IO_STREAM (selfp->connection));
  GInputStream *in = g_io_stream_get_input_stream (G_IO_STREAM (selfp->connection));

  GString *http_string = g_string_new("");
  
  JsonBuilder *builder = json_builder_new ();
  json_builder_begin_object (builder);
  json_builder_set_member_name (builder, "jsonrpc");
  json_builder_add_string_value (builder, "2.0");
  json_builder_set_member_name (builder, "method");
  json_builder_add_string_value (builder, method);
  json_builder_set_member_name (builder, "params");
  json_builder_add_value(builder,params);
  json_builder_set_member_name (builder, "id");
  json_builder_add_int_value(builder,42);
  json_builder_end_object (builder);
  JsonNode *node = json_builder_get_root(builder);
  char *content_string = glib_jsonrpc_json_to_string(node);
  if (!content_string)
    return -1;
  g_object_unref(builder);

  g_string_append_printf(http_string,
                         "POST / HTTP/1.1\r\n"
                         "User-Agent: GlibJsonRpcClient\r\n"
                         "Host: foo\r\n"
                         "Content-Type: application/x-www-form-urlencoded\r\n"
                         "Content-Length: %d\r\n"
                         "\r\n"
                         ,
                         (int)strlen(content_string));

  g_string_append(http_string, content_string);
  g_free(content_string);

  g_output_stream_write (out,
                         http_string->str,
                         http_string->len,
                         NULL,NULL);
  g_string_free(http_string, TRUE);

  // Read the response
  char buffer[1024];
  gssize size;
  gboolean skip_header = TRUE;
  GString *json_string = g_string_new("");
  JsonParser *parser = json_parser_new();
  GError *error = NULL;
  while (0 < (size = g_input_stream_read (in, buffer,
                                          sizeof buffer, NULL, NULL)))
    {
      int header_size = 0;
      
      if (skip_header)
        {
          gchar *head_end = g_strstr_len(buffer, size,
                                         "\r\n\r\n");
          if (head_end > 0)
              header_size = head_end - buffer;
          else
              continue;
        }
      
      g_string_append_len(json_string, buffer+header_size, size-header_size);
      if (json_parser_load_from_data(parser, json_string->str, -1, &error))
          break;
      else
          g_error_free(error);
    }
  JsonNode *root_object = json_parser_get_root(parser);
  JsonNode *result = json_object_get_member(json_node_get_object(root_object), "result");
  if (!result)
    printf("Oops. did not get json result as expected!\n");
  *response = json_node_copy(result);
  g_object_unref(parser);

  return 0;
}
