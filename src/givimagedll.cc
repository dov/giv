// A spdlog registration for the givimage dll

#include "givimagedll.h"

void registerLogger(std::shared_ptr<spdlog::logger> logger)
{
  spdlog::register_logger(logger);
  spdlog::set_default_logger(logger);
  spdlog::info("Registering the logger in the givimage dll");
}
