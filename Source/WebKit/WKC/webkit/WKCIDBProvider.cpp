/*
 * Copyright (C) 2026 baldbuffalo. All rights reserved.
 *
 * This file is part of the WebKit Wii U port.
 */

#include "config.h"
#include "WKCIDBProvider.h"

#include <WebCore/ClientOrigin.h>
#include <WebCore/DatabaseProvider.h>
#include <WebCore/IDBConnectionToClient.h>
#include <WebCore/IDBConnectionToClientDelegate.h>
#include <WebCore/IDBConnectionToServer.h>
#include <WebCore/IDBConnectionToServerDelegate.h>
#include <WebCore/IDBCursorInfo.h>
#include <WebCore/IDBDatabaseNameAndVersion.h>
#include <WebCore/IDBError.h>
#include <WebCore/IDBGetAllRecordsData.h>
#include <WebCore/IDBGetRecordData.h>
#include <WebCore/IDBIndexInfo.h>
#include <WebCore/IDBIterateCursorData.h>
#include <WebCore/IDBKeyData.h>
#include <WebCore/IDBKeyPath.h>
#include <WebCore/IDBKeyRangeData.h>
#include <WebCore/IDBObjectStoreInfo.h>
#include <WebCore/IDBOpenRequestData.h>
#include <WebCore/IDBRequestData.h>
#include <WebCore/IDBResultData.h>
#include <WebCore/IDBServer.h>
#include <WebCore/IDBTransactionInfo.h>
#include <WebCore/IDBValue.h>
#include <WebCore/IndexKey.h>
#include <WebCore/ProcessIdentifier.h>
#include <wtf/FileSystem.h>
#include <wtf/Function.h>
#include <wtf/HashMap.h>
#include <wtf/NeverDestroyed.h>
#include <wtf/RunLoop.h>
#include <wtf/StdLibExtras.h>
#include <wtf/ThreadSafeRefCounted.h>
#include <wtf/WorkQueue.h>
#include <wtf/text/WTFString.h>

#include <cstdlib>

namespace WKC {
using namespace WebCore;

namespace {

using IDBServerType = WebCore::IDBServer::IDBServer;

class WKCIDBServerContext {
public:
    explicit WKCIDBServerContext(PAL::SessionID sessionID)
        : m_queue(WorkQueue::create("WKC IndexedDB"_s))
    {
        String databaseDirectory;
        if (const char* configuredPath = std::getenv("WKC_INDEXEDDB_PATH"); configuredPath && configuredPath[0]) {
            auto basePath = String::fromUTF8(configuredPath);
            databaseDirectory = FileSystem::pathByAppendingComponent(basePath, String::number(sessionID.toUInt64()));
        }

        // IDBServer requires a non-main thread for construction and all storage
        // operations. The serial WorkQueue also preserves IndexedDB transaction
        // ordering without starting WebKit's separate NetworkProcess.
        m_queue->dispatchSync([this, databaseDirectory = WTF::move(databaseDirectory)]() mutable {
            m_server = makeUnique<IDBServerType>(WTF::move(databaseDirectory),
                [](const ClientOrigin&, uint64_t) {
                    return true;
                });
        });
    }

    void dispatch(Function<void(IDBServerType&)>&& callback)
    {
        m_queue->dispatch([this, callback = WTF::move(callback)]() mutable {
            callback(*m_server);
        });
    }

    void dispatchSync(Function<void(IDBServerType&)>&& callback)
    {
        m_queue->dispatchSync([this, callback = WTF::move(callback)]() mutable {
            callback(*m_server);
        });
    }

private:
    Ref<WorkQueue> m_queue;
    std::unique_ptr<IDBServerType> m_server;
};

static std::optional<IDBKeyPath> isolatedKeyPath(const std::optional<IDBKeyPath>& keyPath)
{
    if (!keyPath)
        return std::nullopt;

    if (auto* stringPath = std::get_if<String>(&*keyPath))
        return IDBKeyPath { stringPath->isolatedCopy() };

    Vector<String> components;
    const auto& sourceComponents = std::get<Vector<String>>(*keyPath);
    components.reserveInitialCapacity(sourceComponents.size());
    for (const auto& component : sourceComponents)
        components.uncheckedAppend(component.isolatedCopy());
    return IDBKeyPath { WTF::move(components) };
}

static IndexIDToIndexKeyMap isolatedIndexKeys(const IndexIDToIndexKeyMap& source)
{
    IndexIDToIndexKeyMap result;
    for (const auto& entry : source)
        result.set(entry.key, entry.value.isolatedCopy());
    return result;
}

// One bridge is shared by every page in a browser session, just as WebKit's
// WebProcess shares one WebIDBConnectionToServer for its NetworkProcess
// connection. Requests travel to the worker-owned IDBServer; results and
// version-change events return to WebCore on the main RunLoop.
class WKCIDBConnection final
    : public IDBClient::IDBConnectionToServerDelegate
    , public IDBServer::IDBConnectionToClientDelegate
    , public ThreadSafeRefCounted<WKCIDBConnection> {
public:
    explicit WKCIDBConnection(PAL::SessionID sessionID)
        : m_sessionID(sessionID)
        , m_identifier(Process::identifier())
        , m_serverContext(sessionID)
    {
        ASSERT(RunLoop::isMain());

        m_clientConnection = IDBClient::IDBConnectionToServer::create(*this, sessionID);
        m_serverContext.dispatchSync([this](IDBServerType& server) {
            m_serverConnection = IDBServer::IDBConnectionToClient::create(*this);
            server.registerConnection(*m_serverConnection);
        });
    }

    void ref() const final
    {
        ThreadSafeRefCounted<WKCIDBConnection>::ref();
    }

    void deref() const final
    {
        ThreadSafeRefCounted<WKCIDBConnection>::deref();
    }

    IDBClient::IDBConnectionToServer& coreConnectionToServer() const
    {
        return m_clientConnection.get();
    }

    std::optional<IDBConnectionIdentifier> identifier() const final
    {
        return m_identifier;
    }

private:
    template<typename Callback>
    void dispatchToMain(Callback&& callback)
    {
        RunLoop::mainSingleton().dispatch([this, callback = WTF::forward<Callback>(callback)]() mutable {
            callback(*m_clientConnection);
        });
    }

    void dispatchToServer(Function<void(IDBServerType&)>&& callback)
    {
        m_serverContext.dispatch(WTF::move(callback));
    }

    // IDBClient::IDBConnectionToServerDelegate: requests from WebCore to IDBServer.

    void deleteDatabase(const IDBOpenRequestData& request) final
    {
        auto copy = request.isolatedCopy();
        dispatchToServer([request = WTF::move(copy)](IDBServerType& server) mutable {
            server.deleteDatabase(request);
        });
    }

    void openDatabase(const IDBOpenRequestData& request) final
    {
        auto copy = request.isolatedCopy();
        dispatchToServer([request = WTF::move(copy)](IDBServerType& server) mutable {
            server.openDatabase(request);
        });
    }

    void abortTransaction(const IDBResourceIdentifier& transactionIdentifier) final
    {
        auto copy = transactionIdentifier.isolatedCopy();
        dispatchToServer([transactionIdentifier = WTF::move(copy)](IDBServerType& server) mutable {
            server.abortTransaction(transactionIdentifier);
        });
    }

    void commitTransaction(const IDBResourceIdentifier& transactionIdentifier, uint64_t handledRequestResultsCount) final
    {
        auto copy = transactionIdentifier.isolatedCopy();
        dispatchToServer([transactionIdentifier = WTF::move(copy), handledRequestResultsCount](IDBServerType& server) mutable {
            server.commitTransaction(transactionIdentifier, handledRequestResultsCount);
        });
    }

    void didFinishHandlingVersionChangeTransaction(IDBDatabaseConnectionIdentifier databaseConnectionIdentifier, const IDBResourceIdentifier& transactionIdentifier) final
    {
        auto copy = transactionIdentifier.isolatedCopy();
        dispatchToServer([databaseConnectionIdentifier, transactionIdentifier = WTF::move(copy)](IDBServerType& server) mutable {
            server.didFinishHandlingVersionChangeTransaction(databaseConnectionIdentifier, transactionIdentifier);
        });
    }

    void createObjectStore(const IDBRequestData& request, const IDBObjectStoreInfo& info) final
    {
        auto requestCopy = request.isolatedCopy();
        auto infoCopy = info.isolatedCopy();
        dispatchToServer([request = WTF::move(requestCopy), info = WTF::move(infoCopy)](IDBServerType& server) mutable {
            server.createObjectStore(request, info);
        });
    }

    void deleteObjectStore(const IDBRequestData& request, const String& objectStoreName) final
    {
        auto requestCopy = request.isolatedCopy();
        auto nameCopy = objectStoreName.isolatedCopy();
        dispatchToServer([request = WTF::move(requestCopy), objectStoreName = WTF::move(nameCopy)](IDBServerType& server) mutable {
            server.deleteObjectStore(request, objectStoreName);
        });
    }

    void renameObjectStore(const IDBRequestData& request, IDBObjectStoreIdentifier objectStoreIdentifier, const String& newName) final
    {
        auto requestCopy = request.isolatedCopy();
        auto nameCopy = newName.isolatedCopy();
        dispatchToServer([request = WTF::move(requestCopy), objectStoreIdentifier, newName = WTF::move(nameCopy)](IDBServerType& server) mutable {
            server.renameObjectStore(request, objectStoreIdentifier, newName);
        });
    }

    void clearObjectStore(const IDBRequestData& request, IDBObjectStoreIdentifier objectStoreIdentifier) final
    {
        auto copy = request.isolatedCopy();
        dispatchToServer([request = WTF::move(copy), objectStoreIdentifier](IDBServerType& server) mutable {
            server.clearObjectStore(request, objectStoreIdentifier);
        });
    }

    void createIndex(const IDBRequestData& request, const IDBIndexInfo& info) final
    {
        auto requestCopy = request.isolatedCopy();
        auto infoCopy = info.isolatedCopy();
        dispatchToServer([request = WTF::move(requestCopy), info = WTF::move(infoCopy)](IDBServerType& server) mutable {
            server.createIndex(request, info);
        });
    }

    void deleteIndex(const IDBRequestData& request, IDBObjectStoreIdentifier objectStoreIdentifier, const String& indexName) final
    {
        auto requestCopy = request.isolatedCopy();
        auto nameCopy = indexName.isolatedCopy();
        dispatchToServer([request = WTF::move(requestCopy), objectStoreIdentifier, indexName = WTF::move(nameCopy)](IDBServerType& server) mutable {
            server.deleteIndex(request, objectStoreIdentifier, indexName);
        });
    }

    void renameIndex(const IDBRequestData& request, IDBObjectStoreIdentifier objectStoreIdentifier, IDBIndexIdentifier indexIdentifier, const String& newName) final
    {
        auto requestCopy = request.isolatedCopy();
        auto nameCopy = newName.isolatedCopy();
        dispatchToServer([request = WTF::move(requestCopy), objectStoreIdentifier, indexIdentifier, newName = WTF::move(nameCopy)](IDBServerType& server) mutable {
            server.renameIndex(request, objectStoreIdentifier, indexIdentifier, newName);
        });
    }

    void putOrAdd(const IDBRequestData& request, const IDBKeyData& key, const IDBValue& value, const IndexIDToIndexKeyMap& indexKeys, const IndexedDB::ObjectStoreOverwriteMode mode) final
    {
        auto requestCopy = request.isolatedCopy();
        auto keyCopy = key.isolatedCopy();
        auto valueCopy = value.isolatedCopy();
        auto indexKeysCopy = isolatedIndexKeys(indexKeys);
        dispatchToServer([request = WTF::move(requestCopy), key = WTF::move(keyCopy), value = WTF::move(valueCopy), indexKeys = WTF::move(indexKeysCopy), mode](IDBServerType& server) mutable {
            server.putOrAdd(request, key, value, indexKeys, mode);
        });
    }

    void getRecord(const IDBRequestData& request, const IDBGetRecordData& data) final
    {
        auto requestCopy = request.isolatedCopy();
        auto dataCopy = data.isolatedCopy();
        dispatchToServer([request = WTF::move(requestCopy), data = WTF::move(dataCopy)](IDBServerType& server) mutable {
            server.getRecord(request, data);
        });
    }

    void getAllRecords(const IDBRequestData& request, const IDBGetAllRecordsData& data) final
    {
        auto requestCopy = request.isolatedCopy();
        auto dataCopy = data.isolatedCopy();
        dispatchToServer([request = WTF::move(requestCopy), data = WTF::move(dataCopy)](IDBServerType& server) mutable {
            server.getAllRecords(request, data);
        });
    }

    void getCount(const IDBRequestData& request, const IDBKeyRangeData& range) final
    {
        auto requestCopy = request.isolatedCopy();
        auto rangeCopy = range.isolatedCopy();
        dispatchToServer([request = WTF::move(requestCopy), range = WTF::move(rangeCopy)](IDBServerType& server) mutable {
            server.getCount(request, range);
        });
    }

    void deleteRecord(const IDBRequestData& request, const IDBKeyRangeData& range) final
    {
        auto requestCopy = request.isolatedCopy();
        auto rangeCopy = range.isolatedCopy();
        dispatchToServer([request = WTF::move(requestCopy), range = WTF::move(rangeCopy)](IDBServerType& server) mutable {
            server.deleteRecord(request, range);
        });
    }

    void openCursor(const IDBRequestData& request, const IDBCursorInfo& info) final
    {
        auto requestCopy = request.isolatedCopy();
        auto infoCopy = info.isolatedCopy();
        dispatchToServer([request = WTF::move(requestCopy), info = WTF::move(infoCopy)](IDBServerType& server) mutable {
            server.openCursor(request, info);
        });
    }

    void iterateCursor(const IDBRequestData& request, const IDBIterateCursorData& data) final
    {
        auto requestCopy = request.isolatedCopy();
        auto dataCopy = data.isolatedCopy();
        dispatchToServer([request = WTF::move(requestCopy), data = WTF::move(dataCopy)](IDBServerType& server) mutable {
            server.iterateCursor(request, data);
        });
    }

    void establishTransaction(IDBDatabaseConnectionIdentifier databaseConnectionIdentifier, const IDBTransactionInfo& info) final
    {
        auto infoCopy = info.isolatedCopy();
        dispatchToServer([databaseConnectionIdentifier, info = WTF::move(infoCopy)](IDBServerType& server) mutable {
            server.establishTransaction(databaseConnectionIdentifier, info);
        });
    }

    void databaseConnectionPendingClose(IDBDatabaseConnectionIdentifier databaseConnectionIdentifier) final
    {
        dispatchToServer([databaseConnectionIdentifier](IDBServerType& server) {
            server.databaseConnectionPendingClose(databaseConnectionIdentifier);
        });
    }

    void databaseConnectionClosed(IDBDatabaseConnectionIdentifier databaseConnectionIdentifier) final
    {
        dispatchToServer([databaseConnectionIdentifier](IDBServerType& server) {
            server.databaseConnectionClosed(databaseConnectionIdentifier);
        });
    }

    void abortOpenAndUpgradeNeeded(IDBDatabaseConnectionIdentifier databaseConnectionIdentifier, const std::optional<IDBResourceIdentifier>& transactionIdentifier) final
    {
        auto transactionCopy = transactionIdentifier ? std::optional<IDBResourceIdentifier> { transactionIdentifier->isolatedCopy() } : std::nullopt;
        dispatchToServer([databaseConnectionIdentifier, transactionIdentifier = WTF::move(transactionCopy)](IDBServerType& server) mutable {
            server.abortOpenAndUpgradeNeeded(databaseConnectionIdentifier, transactionIdentifier);
        });
    }

    void didFireVersionChangeEvent(IDBDatabaseConnectionIdentifier databaseConnectionIdentifier, const IDBResourceIdentifier& requestIdentifier, const IndexedDB::ConnectionClosedOnBehalfOfServer connectionClosed) final
    {
        auto requestCopy = requestIdentifier.isolatedCopy();
        dispatchToServer([databaseConnectionIdentifier, requestIdentifier = WTF::move(requestCopy), connectionClosed](IDBServerType& server) mutable {
            server.didFireVersionChangeEvent(databaseConnectionIdentifier, requestIdentifier, connectionClosed);
        });
    }

    void didGenerateIndexKeyForRecord(const IDBResourceIdentifier& transactionIdentifier, const IDBResourceIdentifier& requestIdentifier, const IDBIndexInfo& info, const IDBKeyData& key, const IndexKey& indexKey, std::optional<int64_t> recordID) final
    {
        auto transactionCopy = transactionIdentifier.isolatedCopy();
        auto requestCopy = requestIdentifier.isolatedCopy();
        auto infoCopy = info.isolatedCopy();
        auto keyCopy = key.isolatedCopy();
        auto indexKeyCopy = indexKey.isolatedCopy();
        dispatchToServer([transaction = WTF::move(transactionCopy), request = WTF::move(requestCopy), info = WTF::move(infoCopy), key = WTF::move(keyCopy), indexKey = WTF::move(indexKeyCopy), recordID](IDBServerType& server) mutable {
            server.didGenerateIndexKeyForRecord(transaction, request, info, key, indexKey, recordID);
        });
    }

    void openDBRequestCancelled(const IDBOpenRequestData& request) final
    {
        auto copy = request.isolatedCopy();
        dispatchToServer([request = WTF::move(copy)](IDBServerType& server) mutable {
            server.openDBRequestCancelled(request);
        });
    }

    void getAllDatabaseNamesAndVersions(const IDBResourceIdentifier& requestIdentifier, const ClientOrigin& origin) final
    {
        auto requestCopy = requestIdentifier.isolatedCopy();
        auto originCopy = origin.isolatedCopy();
        dispatchToServer([request = WTF::move(requestCopy), origin = WTF::move(originCopy)](IDBServerType& server) mutable {
            server.getAllDatabaseNamesAndVersions(Process::identifier(), request, origin);
        });
    }

    // IDBServer::IDBConnectionToClientDelegate: results and events from the
    // worker-owned IDBServer back to the WebCore client on the main RunLoop.

    void didDeleteDatabase(const IDBResultData& result) final
    {
        auto copy = result.isolatedCopy();
        dispatchToMain([result = WTF::move(copy)](auto& client) mutable { client.didDeleteDatabase(result); });
    }

    void didOpenDatabase(const IDBResultData& result) final
    {
        auto copy = result.isolatedCopy();
        dispatchToMain([result = WTF::move(copy)](auto& client) mutable { client.didOpenDatabase(result); });
    }

    void didAbortTransaction(const IDBResourceIdentifier& transactionIdentifier, const IDBError& error) final
    {
        auto transactionCopy = transactionIdentifier.isolatedCopy();
        auto errorCopy = error.isolatedCopy();
        dispatchToMain([transaction = WTF::move(transactionCopy), error = WTF::move(errorCopy)](auto& client) mutable {
            client.didAbortTransaction(transaction, error);
        });
    }

    void didCommitTransaction(const IDBResourceIdentifier& transactionIdentifier, const IDBError& error) final
    {
        auto transactionCopy = transactionIdentifier.isolatedCopy();
        auto errorCopy = error.isolatedCopy();
        dispatchToMain([transaction = WTF::move(transactionCopy), error = WTF::move(errorCopy)](auto& client) mutable {
            client.didCommitTransaction(transaction, error);
        });
    }

    void didCreateObjectStore(const IDBResultData& result) final
    {
        auto copy = result.isolatedCopy();
        dispatchToMain([result = WTF::move(copy)](auto& client) mutable { client.didCreateObjectStore(result); });
    }

    void didDeleteObjectStore(const IDBResultData& result) final
    {
        auto copy = result.isolatedCopy();
        dispatchToMain([result = WTF::move(copy)](auto& client) mutable { client.didDeleteObjectStore(result); });
    }

    void didRenameObjectStore(const IDBResultData& result) final
    {
        auto copy = result.isolatedCopy();
        dispatchToMain([result = WTF::move(copy)](auto& client) mutable { client.didRenameObjectStore(result); });
    }

    void didClearObjectStore(const IDBResultData& result) final
    {
        auto copy = result.isolatedCopy();
        dispatchToMain([result = WTF::move(copy)](auto& client) mutable { client.didClearObjectStore(result); });
    }

    void didCreateIndex(const IDBResultData& result) final
    {
        auto copy = result.isolatedCopy();
        dispatchToMain([result = WTF::move(copy)](auto& client) mutable { client.didCreateIndex(result); });
    }

    void didDeleteIndex(const IDBResultData& result) final
    {
        auto copy = result.isolatedCopy();
        dispatchToMain([result = WTF::move(copy)](auto& client) mutable { client.didDeleteIndex(result); });
    }

    void didRenameIndex(const IDBResultData& result) final
    {
        auto copy = result.isolatedCopy();
        dispatchToMain([result = WTF::move(copy)](auto& client) mutable { client.didRenameIndex(result); });
    }

    void didPutOrAdd(const IDBResultData& result) final
    {
        auto copy = result.isolatedCopy();
        dispatchToMain([result = WTF::move(copy)](auto& client) mutable { client.didPutOrAdd(result); });
    }

    void didGetRecord(const IDBResultData& result) final
    {
        auto copy = result.isolatedCopy();
        dispatchToMain([result = WTF::move(copy)](auto& client) mutable { client.didGetRecord(result); });
    }

    void didGetAllRecords(const IDBResultData& result) final
    {
        auto copy = result.isolatedCopy();
        dispatchToMain([result = WTF::move(copy)](auto& client) mutable { client.didGetAllRecords(result); });
    }

    void didGetCount(const IDBResultData& result) final
    {
        auto copy = result.isolatedCopy();
        dispatchToMain([result = WTF::move(copy)](auto& client) mutable { client.didGetCount(result); });
    }

    void didDeleteRecord(const IDBResultData& result) final
    {
        auto copy = result.isolatedCopy();
        dispatchToMain([result = WTF::move(copy)](auto& client) mutable { client.didDeleteRecord(result); });
    }

    void didOpenCursor(const IDBResultData& result) final
    {
        auto copy = result.isolatedCopy();
        dispatchToMain([result = WTF::move(copy)](auto& client) mutable { client.didOpenCursor(result); });
    }

    void didIterateCursor(const IDBResultData& result) final
    {
        auto copy = result.isolatedCopy();
        dispatchToMain([result = WTF::move(copy)](auto& client) mutable { client.didIterateCursor(result); });
    }

    void fireVersionChangeEvent(UniqueIDBDatabaseConnection& connection, const IDBResourceIdentifier& requestIdentifier, uint64_t requestedVersion) final
    {
        auto databaseConnectionIdentifier = connection.identifier();
        auto requestCopy = requestIdentifier.isolatedCopy();
        dispatchToMain([databaseConnectionIdentifier, request = WTF::move(requestCopy), requestedVersion](auto& client) mutable {
            client.fireVersionChangeEvent(databaseConnectionIdentifier, request, requestedVersion);
        });
    }

    void generateIndexKeyForRecord(const IDBResourceIdentifier& requestIdentifier, const IDBIndexInfo& info, const std::optional<IDBKeyPath>& keyPath, const IDBKeyData& key, const IDBValue& value, std::optional<int64_t> recordID) final
    {
        auto requestCopy = requestIdentifier.isolatedCopy();
        auto infoCopy = info.isolatedCopy();
        auto keyPathCopy = isolatedKeyPath(keyPath);
        auto keyCopy = key.isolatedCopy();
        auto valueCopy = value.isolatedCopy();
        dispatchToMain([request = WTF::move(requestCopy), info = WTF::move(infoCopy), keyPath = WTF::move(keyPathCopy), key = WTF::move(keyCopy), value = WTF::move(valueCopy), recordID](auto& client) mutable {
            client.generateIndexKeyForRecord(request, info, keyPath, key, value, recordID);
        });
    }

    void didStartTransaction(const IDBResourceIdentifier& transactionIdentifier, const IDBError& error) final
    {
        auto transactionCopy = transactionIdentifier.isolatedCopy();
        auto errorCopy = error.isolatedCopy();
        dispatchToMain([transaction = WTF::move(transactionCopy), error = WTF::move(errorCopy)](auto& client) mutable {
            client.didStartTransaction(transaction, error);
        });
    }

    void didCloseFromServer(UniqueIDBDatabaseConnection& connection, const IDBError& error) final
    {
        auto databaseConnectionIdentifier = connection.identifier();
        auto errorCopy = error.isolatedCopy();
        dispatchToMain([databaseConnectionIdentifier, error = WTF::move(errorCopy)](auto& client) mutable {
            client.didCloseFromServer(databaseConnectionIdentifier, error);
        });
    }

    void notifyOpenDBRequestBlocked(const IDBResourceIdentifier& requestIdentifier, uint64_t oldVersion, uint64_t newVersion) final
    {
        auto requestCopy = requestIdentifier.isolatedCopy();
        dispatchToMain([request = WTF::move(requestCopy), oldVersion, newVersion](auto& client) mutable {
            client.notifyOpenDBRequestBlocked(request, oldVersion, newVersion);
        });
    }

    void didGetAllDatabaseNamesAndVersions(const IDBResourceIdentifier& requestIdentifier, Vector<IDBDatabaseNameAndVersion>&& databases) final
    {
        auto requestCopy = requestIdentifier.isolatedCopy();
        Vector<IDBDatabaseNameAndVersion> databasesCopy;
        databasesCopy.reserveInitialCapacity(databases.size());
        for (auto& database : databases)
            databasesCopy.uncheckedAppend(database.isolatedCopy());
        dispatchToMain([request = WTF::move(requestCopy), databases = WTF::move(databasesCopy)](auto& client) mutable {
            client.didGetAllDatabaseNamesAndVersions(request, WTF::move(databases));
        });
    }

    PAL::SessionID m_sessionID;
    IDBConnectionIdentifier m_identifier;
    WKCIDBServerContext m_serverContext;
    RefPtr<IDBClient::IDBConnectionToServer> m_clientConnection;
    RefPtr<IDBServer::IDBConnectionToClient> m_serverConnection;
};

class WKCDatabaseProvider final : public DatabaseProvider {
public:
    IDBClient::IDBConnectionToServer& idbConnectionToServerForSession(PAL::SessionID sessionID) final
    {
        static NeverDestroyed<HashMap<uint64_t, Ref<WKCIDBConnection>>> connections;
        auto key = sessionID.toUInt64();
        auto& connection = connections.get().ensure(key, [&] {
            return adoptRef(*new WKCIDBConnection(sessionID));
        }).iterator->value;
        return connection->coreConnectionToServer();
    }
};

} // anonymous namespace

Ref<DatabaseProvider> createWKCDatabaseProvider()
{
    return adoptRef(*new WKCDatabaseProvider());
}

} // namespace WKC
