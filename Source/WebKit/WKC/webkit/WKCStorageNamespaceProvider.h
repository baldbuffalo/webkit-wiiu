/*
 * Copyright (C) 2026 baldbuffalo. All rights reserved.
 *
 * This file is part of the WebKit Wii U port.
 */

#pragma once

#include <wtf/Ref.h>

namespace WebCore {
class StorageNamespaceProvider;
}

namespace WKC {

WTF::Ref<WebCore::StorageNamespaceProvider> createWKCStorageNamespaceProvider();

} // namespace WKC
