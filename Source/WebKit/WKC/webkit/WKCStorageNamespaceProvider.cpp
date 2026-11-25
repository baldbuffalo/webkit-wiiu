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
#include <WebCore/dom/DocumentPage.h>
#include <WebCore/page/LocalFrameInlines.h>
#include "SecurityOrigin.h"
#include "SecurityOriginData.h"
#include "Storage.h"
#include "StorageArea.h"
#include "StorageEventDispatcher.h"
#include "StorageMap.h"
#include "StorageNamespace.h"
#include "StorageNamespaceProvider.h"
#include "StorageType.h"
#include <WebCore/SQLiteDatabase.h>
#include <WebCore/SQLiteStatement.h>
#include <wtf/FileSystem.h>
#include <wtf/HashMap.h>
#include <wtf/NeverDestroyed.h>
#include <wtf/Ref.h>
#include <wtf/RefCounted.h>
#include <wtf/text/WTFString.h>

#include <cstdlib>

namespace WKC {
using namespace WebCore;

namespace {

constexpr unsigned defaultStorageQuota = 5 * 1024 * 1024;

constexpr auto createLocalStorageTable = "CREATE TABLE IF NOT EXISTS WKCWebStorage (origin TEXT NOT NULL, key TEXT NOT NULL, value TEXT NOT NULL, PRIMARY KEY (origin, key))"_s;
constexpr auto selectLocalStorageItems = "SELECT key, value FROM WKCWebStorage WHERE origin=?"_s;
constexpr auto setLocalStorageItem = "INSERT OR REPLACE INTO WKCWebStorage (origin, key, value) VALUES (?, ?, ?)"_s;
constexpr auto removeLocalStorageItem = "DELETE FROM WKCWebStorage WHERE origin=? AND key=?"_s;
constexpr auto clearLocalStorageItems = "DELETE FROM WKCWebStorage WHERE origin=?"_s;

// One SQLite database stores all localStorage origins. The SQLite wrapper uses
// the WKC VFS selected by the platform build, so the same implementation works
// with the Wii U filesystem and with an in-memory fallback when no path is set.
class WKCStorageDatabase {
public:
    WKCStorageDatabase()
    {
        WTF::String databasePath = SQLiteDatabase::inMemoryPath();
        if (const char* configuredDirectory = std::getenv("WKC_LOCAL_STORAGE_PATH"); configuredDirectory && configuredDirectory[0]) {
            auto directory = WTF::String::fromUTF8(configuredDirectory);
            FileSystem::makeAllDirectories(directory);
            databasePath = FileSystem::pathByAppendingComponent(directory, "localstorage.db"_s);
        }

        if (!m_database.open(databasePath))
            return;

        m_isReady = m_database.executeCommand(createLocalStorageTable);
    }

    void load(const WTF::String& origin, StorageMap& map)
    {
        if (!m_isReady)
            return;

        auto statement = m_database.prepareStatement(selectLocalStorageItems);
        if (!statement || statement->bindText(1, origin) != SQLITE_OK)
            return;

        HashMap<WTF::String, WTF::String> items;
        int result = statement->step();
        while (result == SQLITE_ROW) {
            auto key = statement->columnText(0);
            auto value = statement->columnText(1);
            if (!key.isNull() && !value.isNull())
                items.set(WTF::move(key), WTF::move(value));
            result = statement->step();
        }

        if (result == SQLITE_DONE)
            map.importItems(WTF::move(items));
    }

    void set(const WTF::String& origin, const WTF::String& key, const WTF::String& value)
    {
        if (!m_isReady)
            return;

        auto statement = m_database.prepareStatement(setLocalStorageItem);
        if (!statement)
            return;
        if (statement->bindText(1, origin) || statement->bindText(2, key) || statement->bindText(3, value))
            return;
        statement->step();
    }

    void remove(const WTF::String& origin, const WTF::String& key)
    {
        if (!m_isReady)
            return;

        auto statement = m_database.prepareStatement(removeLocalStorageItem);
        if (!statement)
            return;
        if (statement->bindText(1, origin) || statement->bindText(2, key))
            return;
        statement->step();
    }

    void clear(const WTF::String& origin)
    {
        if (!m_isReady)
            return;

        auto statement = m_database.prepareStatement(clearLocalStorageItems);
        if (!statement || statement->bindText(1, origin))
            return;
        statement->step();
    }

private:
    SQLiteDatabase m_database;
    bool m_isReady { false };
};

static WKCStorageDatabase& localStorageDatabase()
{
    static NeverDestroyed<WKCStorageDatabase> database;
    return database.get();
}

// StorageMap supplies WebCore's quota checks and copy-on-write behavior. The
// small ref-counted wrapper lets each LocalDOMWindow have its own StorageArea
// identity while same-origin windows still share the underlying local data.
class WKCStorageMap final : public RefCounted<WKCStorageMap> {
public:
    WKCStorageMap(unsigned quota, WTF::String originIdentifier, bool persistent)
        : map(quota)
        , m_originIdentifier(WTF::move(originIdentifier))
        , m_persistent(persistent)
    {
        if (m_persistent)
            localStorageDatabase().load(m_originIdentifier, map);
    }

    WKCStorageMap(const StorageMap& other, const WTF::String& originIdentifier, bool persistent)
        : map(other)
        , m_originIdentifier(originIdentifier)
        , m_persistent(persistent)
    {
    }

    Ref<WKCStorageMap> copy() const
    {
        return adoptRef(*new WKCStorageMap(map, m_originIdentifier, m_persistent));
    }

    void persistSet(const WTF::String& key, const WTF::String& value)
    {
        if (m_persistent)
            localStorageDatabase().set(m_originIdentifier, key, value);
    }

    void persistRemove(const WTF::String& key)
    {
        if (m_persistent)
            localStorageDatabase().remove(m_originIdentifier, key);
    }

    void persistClear()
    {
        if (m_persistent)
            localStorageDatabase().clear(m_originIdentifier);
    }

    StorageMap map;

private:
    WTF::String m_originIdentifier;
    bool m_persistent { false };
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

    WTF::String LEGACY_NODELETE key(unsigned index) final
    {
        return m_storageMap->map.key(index);
    }

    WTF::String LEGACY_NODELETE item(const WTF::String& key) final
    {
        return m_storageMap->map.getItem(key);
    }

    void LEGACY_NODELETE setItem(LocalFrame& sourceFrame, const WTF::String& key, const WTF::String& value, bool& quotaException) final
    {
        WTF::String oldValue;
        m_storageMap->map.setItem(key, value, oldValue, quotaException);
        if (quotaException || (!oldValue.isNull() && oldValue == value))
            return;

        m_storageMap->persistSet(key, value);
        dispatchStorageChange(sourceFrame, key, oldValue, value);
    }

    void LEGACY_NODELETE removeItem(LocalFrame& sourceFrame, const WTF::String& key) final
    {
        WTF::String oldValue;
        m_storageMap->map.removeItem(key, oldValue);
        if (oldValue.isNull())
            return;

        m_storageMap->persistRemove(key);
        dispatchStorageChange(sourceFrame, key, oldValue, nullString());
    }

    void LEGACY_NODELETE clear(LocalFrame& sourceFrame) final
    {
        if (!m_storageMap->map.length())
            return;

        m_storageMap->map.clear();
        m_storageMap->persistClear();
        // A null key and null values represent the Storage.clear() event.
        dispatchStorageChange(sourceFrame, nullString(), nullString(), nullString());
    }

    bool LEGACY_NODELETE contains(const WTF::String& key) final
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
    void dispatchStorageChange(LocalFrame& sourceFrame, const WTF::String& key, const WTF::String& oldValue, const WTF::String& newValue)
    {
        auto* document = sourceFrame.document();
        auto* page = sourceFrame.page();
        if (!document || !page)
            return;

        WTF::Function<bool(Storage&)> isSourceStorage = [this](Storage& storage) {
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
            return adoptRef(*new WKCStorageMap(
                m_quota,
                origin.data().databaseIdentifier(),
                m_type == StorageType::Local && !origin.data().opaqueOriginIdentifier()
            ));
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
        static NeverDestroyed<WKCStorageNamespace> sharedNamespace(
            StorageType::Local, defaultStorageQuota, sessionID
        );
        UNUSED_PARAM(quota);
        return Ref<StorageNamespace> { sharedNamespace.get() };
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
