#pragma once
// Compatibility helpers for WKC peer structs

static inline void WKCFloatRect_SetRect(WKCFloatRect* dst, const WKCFloatRect* src) {
    *dst = *src;
}
static inline void WKCFloatSize_Set(WKCFloatSize* s, float w, float h) {
    s->fWidth = w; s->fHeight = h;
}
static inline void WKCFloatPoint_Set(WKCFloatPoint* p, float x, float y) {
    p->fX = x; p->fY = y;
}
