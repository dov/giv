//======================================================================
//  givimagedll.h - 
//
//  Dov Grobgeld <dov.grobgeld@gmail.com>
//  Mon Jan  9 12:09:18 2023
//----------------------------------------------------------------------
#ifndef GIVIMAGEDLL_H
#define GIVIMAGEDLL_H

#include <spdlog/spdlog.h>

// In giv-image.h
#ifdef _WIN32
  #ifdef GIV_EXPORTS
    #define GIV_API __declspec(dllexport)
  #else
    #define GIV_API __declspec(dllimport)
  #endif
#else
  #define GIV_API
#endif

GIV_API void registerLogger(std::shared_ptr<spdlog::logger> logger);

#endif /* GIVIMAGEDLL */
