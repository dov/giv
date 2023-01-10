// A spdlog registration for the givimage dll

#include "givimagedll.h"

void registerLogger(std::shared_ptr<spdlog::logger> logger)
{
#ifdef _WIN32
  // This is needed only under windows because the different namespaces of the DLL
  spdlog::register_logger(logger);
  spdlog::set_default_logger(logger);
  spdlog::info("Registering the logger in the givimage dll");
#endif
}
