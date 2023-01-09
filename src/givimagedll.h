//======================================================================
//  givimagedll.h - 
//
//  Dov Grobgeld <dov.grobgeld@gmail.com>
//  Mon Jan  9 12:09:18 2023
//----------------------------------------------------------------------
#ifndef GIVIMAGEDLL_H
#define GIVIMAGEDLL_H

#include <spdlog/spdlog.h>

void registerLogger(std::shared_ptr<spdlog::logger> logger);

#endif /* GIVIMAGEDLL */
