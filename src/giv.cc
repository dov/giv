//======================================================================
//  test-giv-win.cc - This will be the new giv
//
//  Dov Grobgeld <dov.grobgeld@gmail.com>
//  Wed Jul  8 19:32:57 2009
//----------------------------------------------------------------------
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include "gtk/gtk.h"
#include "giv-win.h"
#include "givimagedll.h"
#include "glib-jsonrpc/glib-jsonrpc-json.h"
#include "glib-jsonrpc/glib-jsonrpc-client.h"
#include "spdlog/sinks/basic_file_sink.h"
#include "spdlog/sinks/rotating_file_sink.h"
#include "spdlog/spdlog.h"
#include "spdlog/async_logger.h"
#include <glib/gstdio.h>
#include <plis/plis.h>


const int GIV_DEFAULT_PORT = 8222;

using namespace std;
using namespace plis;
using namespace fmt;

#define CASE(s) if (!strcmp(s, S_))

string join(const vector<string>& v, const string& glue)
{
  string ret;
  for (int i=0; i<(int)v.size(); i++)
  {
    ret += v[i];
    if (i < (int)v.size()-1)
      ret += glue;
  }
  return ret;
}

// Remove an argument from argv. Note that there is no need to free.
void rm_from_args(int& argc, char **argv, int argp)
{
  if (argp <= 0 || argp >= argc)
    throw runtime_error("Can't remove outside of bounds!");

  for (int i=argp; i<argc-1; i++)
    argv[i]=argv[i+1];

  argc--;
}

#if 0
static void rec_mkdir(std::string path)
{
  size_t pos = 0;
  int mode = 0;
#ifndef _WIN32
  mode = S_IRWXU | S_IRWXG | S_IRWXO;
#endif

  if (path[path.size()-1]!= '/')
      path += '/';

  while((pos = path.find_first_of('/', pos+1)) != std::string::npos)
    g_mkdir(path.substr(0, pos).c_str(), mode);
}
#endif

int main(int argc, char **argv)
{
  vector<string> args;
  for (int i=0; i<argc; i++)
    args.push_back(argv[i]);


  // defaults
  bool do_auto_fit_marks = true;
  bool do_join = false;
  double scale = -1;
  double shift_x = 0;
  double shift_y = 0;
  char *export_filename = NULL;
  int giv_port = GIV_DEFAULT_PORT;
  gboolean do_remote = FALSE;
  int remote_port = 8822;
  string remote_host = "localhost"; 
  gchar *remote_string=NULL;
  gchar *json_method=NULL;
  gchar *json_params_string=NULL;
  int width = 500;
  int height = 500;
  int argp=1;
  string log_filename;
    
  // Parse command line
  while(argp < argc && argv[argp][0] == '-') {
    char *S_ = argv[argp++];

    CASE("--help") {
      printf("giv - An image and vector viewer\n"
             "\n"
             "Syntax:\n"
             "    giv [-n] [--join] image [image...]\n"
             "\n"
             "Options:\n"
             "    --log_file log_file  Log giv debug to the given log file\n"
             "    --join               Join the display of all files on command line\n"
             "    -n                   Initially use 1:1 zoom for images.\n"
             "    --geometry           Set size of image viewer\n"
             "    --zoom scale shift_x shift_y   Zoom the image\n"
             "    --port p             Set port for giv remote jsonrpc server. Default is 8222\n"
             "    --remote cmd         Run giv remote command cmd. Use \"-remote\n"
             "                         help\" to get a list of supported commands.\n"
             "    --remote-port port   Portfor remote client. Default is 8822.\n"
             "    --remote-host host   Host for remote client.\n"

             );
      exit(0);
    }
    CASE("--log_file")
    {
      log_filename = argv[argp++];
      continue;
    }
    CASE("-n") {
      do_auto_fit_marks = false;
      continue;
    }
    CASE("--join") {
      do_join = true;
      continue;
    }
    CASE("--geometry") {
      llip matches;
      if (slip(argv[argp++]).m("(\\d+)x(\\d+)", matches)) {
        width = atoi(matches[1]);
        height = atoi(matches[2]);
      }
      continue;
    }
    CASE("--zoom") {
      scale = atof(argv[argp++]);
      shift_x = atof(argv[argp++]);
      shift_y = atof(argv[argp++]);
      continue;
    }
    CASE("--export")
    {
      export_filename = argv[argp++];
      continue;
    }
    CASE("--port")
    {
      giv_port = atoi(argv[argp++]);
      continue;
    }
    CASE("--remote")
    {
      do_remote = TRUE;
      remote_string= argv[argp++];
      continue;
    }
    CASE("--remote-port")
    {
      remote_port = atoi(argv[argp++]);
      continue;
    }
    CASE("--remote-host")
    {
      remote_host = argv[argp++];
      continue;
    }
    CASE("--json_method")
    {
      do_remote = TRUE;
      json_method = g_strdup(argv[argp++]);
      continue;
    }
    CASE("--json_params")
    {
      json_params_string = argv[argp++];
      continue;
    }

    printf("Unknown option %s!\n", S_);
    exit(-1);
  }

  vector<spdlog::sink_ptr> log_sinks;
  if (log_filename.size())
  {
    auto rotating_sink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(log_filename, 1024*1024*10, 3);
    log_sinks.push_back(rotating_sink);

  }
  auto logger = std::make_shared<spdlog::logger>("giv logger", log_sinks.begin(), log_sinks.end());
  spdlog::set_default_logger(logger);
  logger->set_pattern("[%H:%M:%S] [%l] %v");

  spdlog::info("======================================================");
  spdlog::info("Starting giv");
  spdlog::info("CommitID: {}", GIT_COMMIT_ID);
  spdlog::info("CommitTime: {}", GIT_COMMIT_TIME);
  spdlog::info("Command line: {}", join(args," "));

  // This is needed only under windows because of the difference
  // in dll conventions
#ifdef _WIN32
  spdlog::info("Calling to register logger in givimagedll");
  registerLogger(logger);
#endif

  // Load an image into a remote instance of giv. If this fails,
  // then normal loading should take place.
  if (do_remote)
  {
    GLibJsonRpcClient *client = glib_jsonrpc_client_new(remote_host.c_str(),
                                                        remote_port);

    if (client)
    {
      JsonNode *params = NULL;
      if (!json_method)
      {
        // Either split remote string as space separated by params
        char **json_args = g_strsplit(remote_string," ",-1);
        json_method = g_strdup(json_args[0]);
        char **json_params = &json_args[1];
        params = glib_jsonrpc_json_strv_to_json_array(json_params);
        g_strfreev(json_args);
      }
      else if (json_params_string)
        params = glib_jsonrpc_json_string_to_json_node(json_params_string);

      JsonNode *response = NULL;
      glib_jsonrpc_client_call(client,
                               json_method,
                               params,
                               // output
                               &response);
      g_free(json_method);
      json_node_free(params);
      if (response)
      {
        gchar *str = glib_jsonrpc_json_to_string(response);
        printf("%s\n",str);
        g_free(str);
        json_node_free(response);
      }
      exit(0);
    }

    // Fail!
    print(stderr, "No giv remote server found at port {}:{}!\n",
          remote_host, remote_port);
    exit(-1);
  }
            


#if 0
  string appdir; // Only used on linux
  std::vector<spdlog::sink_ptr> general_sinks;
  const char *s;
  if ((s = getenv("LOCALAPPDATA"))) // Really only for windows
    appdir = string(s) + "/Giv";
  else
    appdir = string(getenv("HOME")) + "/.local/giv";
 
  string logdir = appdir + "/logs";
  rec_mkdir(logdir);
#endif

  gtk_init(&argc, &argv);

  GtkWidget *giv = giv_win_new(
    do_auto_fit_marks,
    do_join,
    width,
    height,
    scale,
    shift_x,
    shift_y,
    export_filename,
    giv_port,
    argc,
    argv,
    argp);

  gtk_widget_show(giv);

  gtk_main();

  exit(0);
  return(0);
}
