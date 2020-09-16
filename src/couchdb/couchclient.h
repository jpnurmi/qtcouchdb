#ifndef COUCHCLIENT_H
#define COUCHCLIENT_H

#include <QtCouchDB/couchdbglobal.h>
#include <QtCore/qobject.h>
#include <QtCore/qscopedpointer.h>

class CouchQuery;
class CouchResponse;
class CouchClientPrivate;

class COUCHDB_EXPORT CouchClient : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QUrl url READ url WRITE setUrl)

public:
    explicit CouchClient(QObject *parent = nullptr);
    explicit CouchClient(const QUrl &url, QObject *parent = nullptr);
    ~CouchClient();

    QUrl url() const;
    void setUrl(const QUrl &url);

    Q_INVOKABLE QUrl databaseUrl(const QString &databaseName) const;
    Q_INVOKABLE QUrl documentUrl(const QString &databaseName, const QString &documentId, const QString &revision = QString()) const;
    Q_INVOKABLE QUrl attachmentUrl(const QString &databaseName, const QString &documentId, const QString &attachmentName, const QString &revision = QString()) const;

signals:
    void installationChecked(const CouchResponse &response);
    void sessionStarted(const CouchResponse &response);
    void sessionEnded(const CouchResponse &response);
    void databasesListed(const CouchResponse &response);
    void databaseCreated(const CouchResponse &response);
    void databaseDeleted(const CouchResponse &response);
    void documentsListed(const CouchResponse &response);
    void revisionRetrieved(const CouchResponse &response);
    void documentRetrieved(const CouchResponse &response);
    void documentUpdated(const CouchResponse &response);
    void documentDeleted(const CouchResponse &response);
    void attachmentUploaded(const CouchResponse &response);
    void attachmentDeleted(const CouchResponse &response);
    void databaseReplicated(const CouchResponse &response);

public slots:
    Q_INVOKABLE void checkInstallation();

    Q_INVOKABLE void startSession(const QString &username, const QString &password);
    Q_INVOKABLE void endSession();

    Q_INVOKABLE void listDatabases();
    Q_INVOKABLE void createDatabase(const QString &database);
    Q_INVOKABLE void deleteDatabase(const QString &database);

    Q_INVOKABLE void listDocuments(const QString &database);
    Q_INVOKABLE void retrieveRevision(const QString &database, const QString &documentId);
    Q_INVOKABLE void retrieveDocument(const QString &database, const QString &documentId);
    Q_INVOKABLE void updateDocument(const QString &database, const QString &documentId, const QByteArray &document);
    Q_INVOKABLE void deleteDocument(const QString &database, const QString &documentId, const QString &revision);

    Q_INVOKABLE void uploadAttachment(const QString &database, const QString &documentId, const QString &attachmentName, const QByteArray &attachment,
                                      const QString &mimeType, const QString &revision);
    Q_INVOKABLE void deleteAttachment(const QString &database, const QString &documentId, const QString &attachmentName, const QString &revision);

    Q_INVOKABLE void replicateDatabaseFrom(const QUrl &sourceServer, const QString &sourceDatabase, const QString &targetDatabase,
                                           bool createTarget, bool continuous, bool cancel = false);
    Q_INVOKABLE void replicateDatabaseTo(const QUrl &targetServer, const QString &sourceDatabase, const QString &targetDatabase,
                                         bool createTarget, bool continuous, bool cancel = false);

protected:
    void executeQuery(const CouchQuery &query);

private:
    Q_DECLARE_PRIVATE(CouchClient)
    QScopedPointer<CouchClientPrivate> d_ptr;
};

#endif // COUCHCLIENT_H