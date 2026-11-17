/*
 * Copyright (C) 2026 baldbuffalo. All rights reserved.
 *
 * This file is part of the WebKit Wii U port.
 */

#include "config.h"
#include "WKCStorageNamespaceProvider.h"

#include "Document.h"
#include "LocalFrame.h"
#include "Page.h"
#include "SecurityOrigin.h"
#include "SecurityOriginData.h"
#include "Storage.h"
#include "StorageArea.h"
#include "StorageEventDispatcher.h"
#include "StorageMap.h"
#include "StorageNamespace.h"
#include "StorageNamespaceProvider.h"
#include "StorageType.h"
#include <wtf/HashMap.h>
#include <wtf/NeverDestroyed.h>
#include <wtf/Ref.h>
#include <wtf/RefCounted.h>

namespace WKC {
using namespace WebCore;

namespace {

constexpr unsigned defaultStorageQuota = 5 * 1024 * 1024;

// StorageMap supplies WebCore's quota checks and copy-on-write behavior. The
// small ref-counted wrapper lets each LocalDOMWindow have its own StorageArea
// identity while same-origin windows still share the underlying local data.
class WKCStorageMap final : public RefCounted<WKCStorageMap> {
public:
    explicit WKCStorageMap(unsigned quota)
        : map(quota)
    {
    }

    explicit WKCStorageMap(const StorageMap& other)
        : map(other)
    {
    }

    Ref<WKCStorageMap> copy() const
    {
        return adoptRef(*new WKCStorageMap(map));
    }

    StorageMap map;
};

class WKCStorageArea final : public StorageArea {
public:
    WKCStorageArea(StorageType type, Ref<WKCStorageMap> storageMap)
        : m_type(type)
        , m_storageMap(WTF::move(storageMap))
    {
    }

    unsigned LEGACY_NODELETE length() final
    {
        return m_storageMap->map.length();
    }

    String LEGACY_NODELETE key(unsigned index) final
    {
        return m_storageMap->map.key(index);
    }

    String LEGACY_NODELETE item(const String& key) final
    {
        return m_storageMap->map.getItem(key);
    }

    void LEGACY_NODELETE setItem(LocalFrame& sourceFrame, const String& key, const String& value, bool& quotaException) final
    {
        String oldValue;
        m_storageMap->map.setItem(key, value, oldValue, quotaException);
        if (quotaException || (!oldValue.isNull() && oldValue == value))
            return;

        dispatchStorageChange(sourceFrame, key, oldValue, value);
    }

    void LEGACY_NODELETE removeItem(LocalFrame& sourceFrame, const String& key) final
    {
        String oldValue;
        m_storageMap->map.removeItem(key, oldValue);
        if (oldValue.isNull())
            return;

        dispatchStorageChange(sourceFrame, key, oldValue, nullString());
    }

    void LEGACY_NODELETE clear(LocalFrame& sourceFrame) final
    {
        if (!m_storageMap->map.length())
            return;

        m_storageMap->map.clear();
        // A null key and null values represent the Storage.clear() event.
        dispatchStorageChange(sourceFrame, nullString(), nullString(), nullString());
    }

    bool LEGACY_NODELETE contains(const String& key) final
    {
        return m_storageMap->map.contains(key);
    }

    StorageType LEGACY_NODELETE storageType() const final
    {
        return m_type;
    }

    size_t LEGACY_NODELETE memoryBytesUsedByCache() final
    {
        size_t bytes = 0;
        for (auto& entry : m_storageMap->map.items())
            bytes += entry.key.sizeInBytes() + entry.value.sizeInBytes();
        return bytes;
    }

private:
    void dispatchStorageChange(LocalFrame& sourceFrame, const String& key, const String& oldValue, const String& newValue)
    {
        auto* document = sourceFrame.document();
        auto* page = sourceFrame.page();
        if (!document || !page)
            return;

        auto isSourceStorage = [this](Storage& storage) {
            return &storage.area() == this;
        };

        if (m_type == StorageType::Session) {
            StorageEventDispatcher::dispatchSessionStorageEvents(
                key, oldValue, newValue, *page, document->securityOrigin(),
                document->url().string(), isSourceStorage
            );
            return;
        }

        // Local storage is shared across pages and page groups in this
        // single-process port, so dispatch to every same-origin page.
        StorageEventDispatcher::dispatchLocalStorageEvents(
            key, oldValue, newValue, nullptr, document->securityOrigin(),
            document->url().string(), isSourceStorage
        );
    }

    StorageType m_type;
    Ref<WKCStorageMap> m_storageMap;
};

class WKCStorageNamespace final : public StorageNamespace {
public:
    WKCStorageNamespace(StorageType type, unsigned quota, PAL::SessionID sessionID,
                        RefPtr<SecurityOrigin> topLevelOrigin = nullptr)
        : m_type(type)
        , m_quota(quota)
        , m_sessionID(sessionID)
        , m_topLevelOrigin(WTF::move(topLevelOrigin))
    {
    }

    Ref<StorageArea> storageArea(const SecurityOrigin& origin) final
    {
        auto& storageMap = m_storageMaps.ensure(origin.data(), [&] {
            return adoptRef(*new WKCStorageMap(m_quota));
        }).iterator->value;

        // Each document gets a distinct StorageArea object so storage-event
        // delivery can suppress the source window without suppressing peers.
        return adoptRef(*new WKCStorageArea(m_type, storageMap));
    }

    const SecurityOrigin* LEGACY_NODELETE topLevelOrigin() const final
    {
        return m_topLevelOrigin.get();
    }

    Ref<StorageNamespace> copy(Page& page) final
    {
        auto result = adoptRef(*new WKCStorageNamespace(m_type, m_quota, page.sessionID(), m_topLevelOrigin));
        for (auto& entry : m_storageMaps)
            result->m_storageMaps.set(entry.key, entry.value->copy());
        return result;
    }

    PAL::SessionID LEGACY_NODELETE sessionID() const final
    {
        return m_sessionID;
    }

    void LEGACY_NODELETE setSessionIDForTesting(PAL::SessionID sessionID) final
    {
        m_sessionID = sessionID;
    }

private:
    StorageType m_type;
    unsigned m_quota;
    PAL::SessionID m_sessionID;
    RefPtr<SecurityOrigin> m_topLevelOrigin;
    HashMap<SecurityOriginData, Ref<WKCStorageMap>> m_storageMaps;
};

class WKCStorageNamespaceProvider final : public StorageNamespaceProvider {
public:
    WKCStorageNamespaceProvider()
    {
        setSessionStorageQuota(defaultStorageQuota);
    }

private:
    Ref<StorageNamespace> createLocalStorageNamespace(unsigned quota, PAL::SessionID sessionID) final
    {
        // Share localStorage data between every WKC page in this process.
        // The namespace keeps distinct maps for each SecurityOrigin.
        static NeverDestroyed<Ref<WKCStorageNamespace>> sharedNamespace(
            adoptRef(*new WKCStorageNamespace(StorageType::Local, defaultStorageQuota, sessionID))
        );
        UNUSED_PARAM(quota);
        return Ref<StorageNamespace> { *sharedNamespace.get() };
    }

    Ref<StorageNamespace> createTransientLocalStorageNamespace(SecurityOrigin& topLevelOrigin,
                                                                 unsigned quota, PAL::SessionID sessionID) final
    {
        auto originCopy = SecurityOrigin::create(topLevelOrigin.data().isolatedCopy());
        return adoptRef(*new WKCStorageNamespace(StorageType::TransientLocal, quota, sessionID, WTF::move(originCopy)));
    }

    RefPtr<StorageNamespace> sessionStorageNamespace(const SecurityOrigin& topLevelOrigin,
                                                      Page& page, ShouldCreateNamespace shouldCreate) final
    {
        auto iterator = m_sessionStorageNamespaces.find(topLevelOrigin.data());
        if (iterator != m_sessionStorageNamespaces.end())
            return iterator->value.ptr();

        if (shouldCreate == ShouldCreateNamespace::No)
            return nullptr;

        auto originCopy = SecurityOrigin::create(topLevelOrigin.data().isolatedCopy());
        Ref<StorageNamespace> storageNamespace = adoptRef(*new WKCStorageNamespace(
            StorageType::Session, sessionStorageQuota(), page.sessionID(), WTF::move(originCopy)
        ));
        auto result = m_sessionStorageNamespaces.add(topLevelOrigin.data(), WTF::move(storageNamespace));
        return result.iterator->value.ptr();
    }

    HashMap<SecurityOriginData, Ref<StorageNamespace>> m_sessionStorageNamespaces;
};

} // anonymous namespace

Ref<WebCore::StorageNamespaceProvider> createWKCStorageNamespaceProvider()
{
    return adoptRef(*new WKCStorageNamespaceProvider());
}

} // namespace WKC
