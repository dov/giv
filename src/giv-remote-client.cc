#include "glib-jsonrpc/glib-jsonrpc-client.h"
#include "glib-jsonrpc/glib-jsonrpc-json.h"
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

const int GIV_DEFAULT_PORT = 8222;

static void die(const char *fmt, ...)
{
    va_list ap;
    va_start(ap,fmt); 
    
    vfprintf(stderr, fmt, ap);
    exit(-1);
}

#define CASE(s) if (!strcmp(s, S_))

int main(int argc, char **argv)
{
    int argp = 1;
    int giv_port = GIV_DEFAULT_PORT;
    const gchar *giv_host = "localhost"; // Currently fixed

    while(argp < argc && argv[argp][0] == '-') {
        char *S_ = argv[argp++];

        CASE("-help") {
            printf("giv-remote-client - A simple giv protocol client\n\n"
                   "Syntax:\n"
                   "    giv-remote-client [-port] command arguments\n"
                   "\n"
                   "Description:\n"
                   "    Run giv remote command cmd. Use \"help\n"
                   "   to get a list of supported commands.\n"
                   "\n"
                   "Options:\n"
                   "    -port p    Set port for giv remote jsonrpc server. Default is 8222\n"
                   );
            exit(0);
        }
        CASE("-port") {
            giv_port = atoi(argv[argp++]);
            continue;
        }
#if 0
        CASE("-json_method") {
            json_method = g_strdup(argv[argp++]);
            continue;
        }
        CASE("-json_params") {
            json_params_string = argv[argp++];
            continue;
        }
#endif
        die("Unknown option %s!\n", S_);
    }

    gchar *json_method= argv[argp++];

    GLibJsonRpcClient *client = glib_jsonrpc_client_new(giv_host,
                                                        giv_port);
    if (!client)
        die("Failed opening client to %s:%d\n", giv_host, giv_port);

    if (client) {
        JsonNode *params = NULL;

        char **json_params = &argv[argp];
        params = glib_jsonrpc_json_strv_to_json_array(json_params);

        JsonNode *response = NULL;
        glib_jsonrpc_client_call(client,
                                 json_method,
                                 params,
                                 // output
                                 &response);
        json_node_free(params);
        if (response) {
            gchar *str = glib_jsonrpc_json_to_string(response);
            printf("%s\n",str);
            g_free(str);
            json_node_free(response);
        }
        exit(0);
    }

    // Fail!
    fprintf(stderr, "No giv remote client found!\n");
    exit(-1);

    exit(0);
    return(0);
}
