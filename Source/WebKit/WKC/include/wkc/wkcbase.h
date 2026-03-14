#pragma once

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

typedef unsigned int WKCColor;
typedef float        WKCFloat;

typedef struct WKCPoint_ { int fX, fY; } WKCPoint;
typedef struct WKCSize_  { int fWidth, fHeight; } WKCSize;
typedef struct WKCRect_  { WKCPoint fPoint; WKCSize fSize; } WKCRect;

typedef struct WKCFloatPoint_ { float fX, fY; } WKCFloatPoint;
typedef struct WKCFloatSize_  { float fWidth, fHeight; } WKCFloatSize;
typedef struct WKCFloatRect_  { WKCFloatPoint fPoint; WKCFloatSize fSize; } WKCFloatRect;

#define WKCPoint_Set(p, x, y)     do { (p)->fX = (x); (p)->fY = (y); } while(0)
#define WKCSize_Set(s, w, h)      do { (s)->fWidth = (w); (s)->fHeight = (h); } while(0)
#define WKCRect_Set(r, x, y, w, h) do { WKCPoint_Set(&(r)->fPoint,(x),(y)); WKCSize_Set(&(r)->fSize,(w),(h)); } while(0)
