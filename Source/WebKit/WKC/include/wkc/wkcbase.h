/*
 * wkcbase.h — WKC base definitions stub for Wii U / Aroma port
 */

#ifndef _WKC_BASE_H_
#define _WKC_BASE_H_

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#ifdef __cplusplus
#  define WKC_BEGIN_C_LINKAGE extern "C" {
#  define WKC_END_C_LINKAGE   }
#else
#  define WKC_BEGIN_C_LINKAGE
#  define WKC_END_C_LINKAGE
#endif

#define WKC_PEER_API
#define WKC_API
#define WKC_EXPORT

typedef unsigned int  WKCColor;  /* 0xAARRGGBB */
typedef float         WKCFloat;

typedef struct WKCPoint_ { int x, y; }          WKCPoint;
typedef struct WKCSize_  { int width, height; }  WKCSize;
typedef struct WKCRect_  { WKCPoint origin; WKCSize size; } WKCRect;

typedef struct WKCFloatPoint_ { float x, y; }         WKCFloatPoint;
typedef struct WKCFloatSize_  { float width, height; } WKCFloatSize;
typedef struct WKCFloatRect_  { WKCFloatPoint origin; WKCFloatSize size; } WKCFloatRect;

#endif /* _WKC_BASE_H_ */
