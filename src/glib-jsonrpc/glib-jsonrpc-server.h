/* GlibJsonRpcServer
 * glib-jsonrpc-server.h: A Json RCP server in glib.
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
#ifndef GLIB_JSON_RPC_SERVER_H
#define GLIB_JSON_RPC_SERVER_H

#include "json-glib/json-glib.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
} GLibJsonRpcServer; 

typedef int (GLibJsonRpcCommandCallback)(GLibJsonRpcServer *server,
                                         const char *method,
                                         JsonNode *params,
                                         JsonNode **response,
                                         gpointer user_data);

typedef int (GLibJsonRpcCommandAsyncCallback)(GLibJsonRpcServer *server,
                                              const char *method,
                                              JsonNode *params,
                                              gpointer user_data);

GLibJsonRpcServer *glib_jsonrpc_server_new(int port);
void glib_jsonrpc_server_free(GLibJsonRpcServer *);

int glib_jsonrpc_server_register_command(GLibJsonRpcServer *jsonrpc_server,
                                         const gchar *command,
                                         GLibJsonRpcCommandCallback *callback,
                                         gpointer user_data);

int glib_jsonrpc_server_register_async_command(GLibJsonRpcServer *jsonrpc_server,
                                               const gchar *command,
                                               GLibJsonRpcCommandAsyncCallback *async_callback,
                                               gpointer user_data);
// The json node ownershap is transfered.
int  glib_jsonrpc_server_send_async_response(GLibJsonRpcServer *server,
                                             JsonNode *response);


void glib_jsonrpc_server_set_allow_non_loopback_connections(GLibJsonRpcServer *_server,
                                                            gboolean allow_non_loop_back_connections);


#ifdef __cplusplus
}
#endif

#endif /* GLIB-JSON-RPC-SERVER */
