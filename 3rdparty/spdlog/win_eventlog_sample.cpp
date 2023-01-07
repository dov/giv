#include "spdlog/spdlog.h"
#if _WIN32
#include <windows.h>
#include "spdlog/sinks/win_eventlog_sink.h"
#endif

int main()
{
    // create logger object for HIPAA messages
    spdlog::logger hipaa_logger("hipaa");

#if  _WIN32     // When on Windows, let it sink to the Event Log
    hipaa_logger.sinks().push_back(
        std::make_shared<spdlog::sinks::win_eventlog_sink_mt>("DdeDti HIPAA"));
#else
    // wherever you want to direct the HIPAA message on Linux, or nothing
#endif

    // configure other logger stuff, like pattern, level etc.

    // ... logging a message somewhere down the code:
    int user_id=42, study_id=42;
    hipaa_logger.info("user {} has completed reviewing study {}", user_id, study_id);
}
