/* GlibJsonRpcServer
 * glib-jsonrpc-server.c: A Json RCP server in glib.
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
#include <stdlib.h>
#include <stdio.h>
#include "glib-jsonrpc-server.h"
#include "glib-jsonrpc-json.h"

typedef struct {
  GLibJsonRpcServer server; 

  GSocketService *service;
  GHashTable *command_hash;
  GString *req_string;
  gboolean allow_non_loopback_connections;

  // This is set whenever an asynchronous command is being run. Only
  // one asynchronous command can be run at a time.
  gboolean async_busy;
} GLibJsonRpcServerPrivate;

typedef struct {
  GLibJsonRpcCommandCallback *callback;
  GLibJsonRpcCommandAsyncCallback *async_callback;
  gpointer user_data;
  JsonNode *reply;
} command_hash_value_t;

typedef struct {
  GMutex *mutex;
  GCond *cond;
  GLibJsonRpcServer *server;
  JsonNode *reply;
  int error_num;
} GLibJsonRpcAsyncQueryPrivate;

static GLibJsonRpcAsyncQueryPrivate *glib_jsonrpc_server_query_new(GLibJsonRpcServer *server)
{
  GLibJsonRpcAsyncQueryPrivate *query = g_new0(GLibJsonRpcAsyncQueryPrivate, 1);
  query->cond = g_cond_new();
  query->mutex = g_mutex_new();
  query->server = server;
  query->reply = NULL;
  query->error_num = 0;
  //  g_mutex_lock(query->mutex);

  return query;
}

static void glib_jsonrpc_async_query_free(GLibJsonRpcAsyncQueryPrivate *query)
{
  g_mutex_unlock(query->mutex);
  g_cond_free(query->cond);
  g_mutex_free(query->mutex); // Why does this crash?
  json_node_free(query->reply);
  g_free(query);
}

GLibJsonRpcServer *glib_jsonrpc_async_query_get_server(GLibJsonRpcAsyncQuery *_query)
{
  GLibJsonRpcAsyncQueryPrivate *query = (GLibJsonRpcAsyncQueryPrivate*)_query;
  return query->server;
}

static JsonNode* create_fault_value_response(int error_num, JsonNode *msg_node, int id)
{
  JsonBuilder *builder = json_builder_new ();

  json_builder_begin_object (builder);
  json_builder_set_member_name (builder, "jsonrpc");
  json_builder_add_string_value (builder, "2.0");
  json_builder_set_member_name (builder, "error");
  json_builder_begin_object(builder);
  json_builder_set_member_name (builder, "code");
  json_builder_add_int_value(builder, error_num);
  json_builder_set_member_name (builder, "message");
  json_builder_add_value (builder, json_node_copy(msg_node));
  json_builder_end_object (builder);
  json_builder_set_member_name(builder, "id");
  if (id < 0)
    json_builder_add_null_value(builder);
  else
    json_builder_add_int_value(builder, id);
  json_builder_end_object (builder);
  JsonNode *node = json_node_copy(json_builder_get_root (builder));
  
  g_object_unref(builder);
  return node;
}
  
static JsonNode* create_fault_msg_response(int error_num, const char *message, int id)
{
  JsonNode *msg_node = json_node_new(JSON_NODE_VALUE);
  json_node_set_string(msg_node, message);
  JsonNode *node = create_fault_value_response(error_num, msg_node, id);
  json_node_free(msg_node);
  return node;
}
    
// Creates a response. Transfers ownership of reply.
static JsonNode* create_response(JsonNode *reply, int id)
{
  JsonBuilder *builder = json_builder_new ();

  // {"jsonrpc": "2.0", "result": 19, "id": 1}
  json_builder_begin_object (builder);
  json_builder_set_member_name (builder, "jsonrpc");
  json_builder_add_string_value (builder, "2.0");
  
  json_builder_set_member_name (builder, "id");
  json_builder_add_int_value (builder, id);

  json_builder_set_member_name (builder, "result");

  if (reply)
    json_builder_add_value (builder, json_node_copy(reply));
  else
    // default "ok" message
    json_builder_add_string_value (builder, "ok");

  json_builder_end_object (builder);
  
  JsonNode *node = json_node_copy(json_builder_get_root (builder));

  g_object_unref(builder);
  return node;
}
    

static gboolean
handler (GThreadedSocketService *service,
         GSocketConnection      *connection,
         GSocketListener        *listener,
         gpointer                user_data)
{
  GLibJsonRpcServerPrivate *jsonrpc_server = (GLibJsonRpcServerPrivate*)user_data;
  GError *error = NULL;
  
  // Check if it is a local connection. - This doesn't work!
  GSocketAddress *sockaddr
    = g_socket_connection_get_remote_address(connection,
                                             &error);
  GInetAddress *addr = g_inet_socket_address_get_address(G_INET_SOCKET_ADDRESS(sockaddr));

  if (!jsonrpc_server->allow_non_loopback_connections)
    {
#if 0
      // Why doesn't this work?
      if (!g_inet_address_get_is_loopback(addr))
        return TRUE; // just fail
#endif
      
      gchar *addr_string = g_inet_address_to_string(addr);
      gboolean is_local = g_strstr_len(addr_string, -1, "127.0.0.1") != NULL;
      g_free(addr_string);
      if (!is_local) 
        return TRUE; // silently fail
    }

  GOutputStream *out;
  GInputStream *in;
  char buffer[1024];
  gssize size;

  out = g_io_stream_get_output_stream (G_IO_STREAM (connection));
  in = g_io_stream_get_input_stream (G_IO_STREAM (connection));

  // Read the http header
  gboolean skip_header = TRUE;
  JsonParser *parser = json_parser_new();
  GString *json_string = g_string_new("");
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

  // TBD:   raise error if there was a syntax error
  g_string_free(json_string, TRUE);

  // Get params object without the reader 
  JsonNode *root = json_parser_get_root(parser);
  JsonObject *root_object = json_node_get_object(root);
  JsonNode* params = json_object_get_member(root_object, "params");

  // Use reader for method and id
  JsonReader *reader = json_reader_new(json_parser_get_root(parser));
  json_reader_read_member (reader, "method");
  const gchar *method = json_reader_get_string_value (reader);
  json_reader_end_member (reader);
  
  json_reader_read_member (reader, "id");
  gint64 id = json_reader_get_int_value(reader);
  json_reader_end_member (reader);

  // Build the response which is either a response object or an error object
  JsonNode *response = NULL;
  
  /* Call the callback */
  command_hash_value_t *command_val = NULL;
  if (method)
    command_val = g_hash_table_lookup(jsonrpc_server->command_hash,
                                      method);
  if (!command_val)
    response = create_fault_msg_response(-2, "No such method!",id);
  else if (command_val->async_callback)
    {
      if (jsonrpc_server->async_busy)
        response = create_fault_msg_response(-2, "Busy!",id);
      else
        {
          // With the embedding of the mutex in the query we should
          // be able to handle more than one connection, so there
          // is no need to protect against a busy state.
          // jsonrpc_server->async_busy = TRUE;
          GLibJsonRpcAsyncQueryPrivate *query = glib_jsonrpc_server_query_new((GLibJsonRpcServer*)jsonrpc_server);
          
          // Create a secondary main loop
          (*command_val->async_callback)((GLibJsonRpcServer*)jsonrpc_server,
                                         (GLibJsonRpcAsyncQuery*)query,
                                         method,
                                         params,
                                         command_val->user_data);
          
          // Lock on a mutex 
          g_mutex_lock(query->mutex);
          g_cond_wait(query->cond, query->mutex);
          g_mutex_unlock(query->mutex);

          if (query->error_num != 0)
            response = create_fault_value_response(query->error_num,query->reply,id);
          else
            response = create_response(query->reply, id);
          jsonrpc_server->async_busy = FALSE;

          // Erase the query
          glib_jsonrpc_async_query_free(query);
        }
    }
  else
    {
      JsonNode *reply;

      int ret = (*command_val->callback)((GLibJsonRpcServer*)jsonrpc_server,
                                         method,
                                         params,
                                         &reply,
                                         command_val->user_data);

      if (ret == 0)
        response = create_response(reply,id);
      else
        // For faults expect a string response containing the error
        response = create_fault_value_response(ret, reply,id);

      if (reply)
        json_node_free(reply);
    }

  if (response)
    {
      GString *response_string = g_string_new("");
  
      // Serialize response into content_string
      JsonGenerator *gen = json_generator_new ();
      gsize len;
      json_generator_set_root (gen, response);
      json_node_free(response);
      gchar *content_string = json_generator_to_data(gen, &len);
      g_object_unref (gen);

      g_string_append_printf(response_string,
                             "HTTP/1.1 200 OK\r\n"
                             "Connection: close\r\n"
                             "Content-Length: %d\r\n"
                             "Content-Type: text/xml\r\n"
                             "Date: Fri, 1 Jan 2000 00:00:00 GMT\r\n"
                             "Server: GlibJsonRPC server\r\n"
                             "\r\n"
                             "%s",
                             len,
                             content_string
                             );
      g_free(content_string);
  
      g_output_stream_write (out,
                             response_string->str,
                             response_string->len,
                             NULL,NULL);
      g_string_free(response_string, TRUE);
    }

  g_object_unref(parser);

  return TRUE;
}

GLibJsonRpcServer *glib_jsonrpc_server_new(int port)
{
  GLibJsonRpcServerPrivate *jsonrpc_server = g_new0(GLibJsonRpcServerPrivate, 1);
  GError *error = NULL;
  jsonrpc_server->service = g_threaded_socket_service_new (10);
  jsonrpc_server->async_busy = FALSE;
  jsonrpc_server->allow_non_loopback_connections = FALSE;

  if (!g_socket_listener_add_inet_port (G_SOCKET_LISTENER (jsonrpc_server->service),
					port,
					NULL,
					&error))
    {
      g_printerr ("%s\n", error->message);
      g_error_free(error);
      g_free(jsonrpc_server);
      return NULL;
    }

  g_signal_connect (jsonrpc_server->service, "run", G_CALLBACK (handler), jsonrpc_server);

  jsonrpc_server->command_hash = g_hash_table_new(g_str_hash,
                                                  g_str_equal);

  return (GLibJsonRpcServer*)jsonrpc_server;
}

void glib_jsonrpc_server_free(GLibJsonRpcServer *_server)
{
  GLibJsonRpcServerPrivate *server = (GLibJsonRpcServerPrivate *)_server;
  
  g_object_unref(G_OBJECT(server->service));
  g_free(server);
}

static int
glib_jsonrpc_server_register_command_full(GLibJsonRpcServer *_jsonrpc_server,
                                           const gchar *command,
                                           GLibJsonRpcCommandCallback *callback,
                                           GLibJsonRpcCommandAsyncCallback *async_callback,
                                           gpointer user_data)
{
  GLibJsonRpcServerPrivate *jsonrpc_server = (GLibJsonRpcServerPrivate*)_jsonrpc_server;
  /* TBD - Check if command exist and override it */
  command_hash_value_t *val = g_new0(command_hash_value_t, 1);
  val->callback = callback;
  val->async_callback = async_callback;
  val->user_data = user_data;
  g_hash_table_insert(jsonrpc_server->command_hash,
                      g_strdup(command),
                      val);

  return 0;
}

int glib_jsonrpc_server_register_command(GLibJsonRpcServer *jsonrpc_server,
                                          const gchar *command,
                                          GLibJsonRpcCommandCallback *callback,
                                          gpointer user_data)
{
  glib_jsonrpc_server_register_command_full(jsonrpc_server,
                                             command,
                                             callback,
                                             NULL, /* async */
                                             user_data);
  return 0;
}
    
int glib_jsonrpc_server_register_async_command(GLibJsonRpcServer *jsonrpc_server,
                                                const gchar *command,
                                                GLibJsonRpcCommandAsyncCallback *async_callback,
                                                gpointer user_data)
{
  glib_jsonrpc_server_register_command_full(jsonrpc_server,
                                            command,
                                            NULL,
                                            async_callback, /* async */
                                            user_data);
  return 0;
}
    
int  glib_jsonrpc_server_send_async_response(GLibJsonRpcAsyncQuery *_query,
                                             int error_num,
                                             JsonNode *reply)
{
  GLibJsonRpcAsyncQueryPrivate *query = (GLibJsonRpcAsyncQueryPrivate *)_query;
  query->reply = reply;
  query->error_num = error_num;
  g_mutex_lock(query->mutex);
  g_cond_broadcast(query->cond);
  g_mutex_unlock(query->mutex);

  return TRUE;
}

void glib_jsonrpc_server_set_allow_non_loopback_connections(GLibJsonRpcServer *_server,
                                                            gboolean allow_non_loopback_connections)
{
  GLibJsonRpcServerPrivate *server = (GLibJsonRpcServerPrivate *)_server;
  server->allow_non_loopback_connections = allow_non_loopback_connections;
}
