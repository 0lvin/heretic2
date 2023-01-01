//
// Copyright 1998 Raven Software
//
// Heretic II
//
#ifndef H2COMMON_H
#define H2COMMON_H

#ifdef H2COMMON_STATIC
#  define H2COMMON_API
#else
#  ifdef _WIN32
#    ifdef H2COMMON
#      define H2COMMON_API __declspec(dllexport)
#    else
#      define H2COMMON_API __declspec(dllimport)
#    endif
#  else
#    define H2COMMON_API
#  endif
#endif

#endif
