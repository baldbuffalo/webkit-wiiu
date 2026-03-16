#pragma once
// Stub fontconfig header for WKC/Wii U bare metal

typedef unsigned int FcChar32;
typedef unsigned char FcChar8;
typedef int FcBool;

#define FcFalse 0
#define FcTrue  1

typedef struct _FcPattern   FcPattern;
typedef struct _FcFontSet   FcFontSet;
typedef struct _FcConfig    FcConfig;
typedef struct _FcCharSet   FcCharSet;
typedef struct _FcLangSet   FcLangSet;
typedef struct _FcObjectSet FcObjectSet;

typedef enum _FcResult {
    FcResultMatch,
    FcResultNoMatch,
    FcResultTypeMismatch,
    FcResultNoId,
    FcResultOutOfMemory
} FcResult;

typedef enum _FcMatchKind {
    FcMatchPattern,
    FcMatchFont,
    FcMatchScan
} FcMatchKind;

typedef enum _FcSetName {
    FcSetSystem = 0,
    FcSetApplication = 1
} FcSetName;

static inline void FcCharSetDestroy(FcCharSet*) {}
static inline void FcFontSetDestroy(FcFontSet*) {}
static inline void FcLangSetDestroy(FcLangSet*) {}
static inline void FcObjectSetDestroy(FcObjectSet*) {}
static inline void FcPatternDestroy(FcPattern*) {}
static inline void FcConfigDestroy(FcConfig*) {}
