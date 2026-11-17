/*
 * Copyright (C) 2026 baldbuffalo. All rights reserved.
 *
 * This file is part of the WebKit Wii U port.
 */

#pragma once

#include <wtf/Ref.h>

namespace WebCore {
class DatabaseProvider;
}

namespace WKC {

WTF::Ref<WebCore::DatabaseProvider> createWKCDatabaseProvider();

} // namespace WKC
