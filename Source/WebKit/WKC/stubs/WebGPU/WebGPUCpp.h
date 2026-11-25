/*
 * WKC compatibility forwarding header.
 *
 * WebKit requires the declarations-only WebGPU C++ API header in shared
 * WebCore and inspector headers on every port, even when ENABLE(WEBGPU)=0.
 * Keep the canonical API in Source/WebGPU and expose it through the existing
 * WKC stubs include path without changing cached compiler command lines.
 */
#pragma once

#include "../../../../WebGPU/WebGPUCpp.h"
