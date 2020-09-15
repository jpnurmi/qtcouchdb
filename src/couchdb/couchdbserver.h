#ifndef COUCHDBSERVER_H
#define COUCHDBSERVER_H

#include <QtCouchDB/couchdbglobal.h>
#include <QtCore/qobject.h>
#include <QtCore/qscopedpointer.h>

class CouchDBServerPrivate;

class COUCHDB_EXPORT CouchDBServer : public QObject
{
    Q_OBJECT

public:
    CouchDBServer(QObject *parent = 0);
    ~CouchDBServer();

    QString url() const;
    void setUrl(const QString& url);

    int  port() const;
    void setPort(const int& port);

    bool secureConnection() const;
    void setSecureConnection(const bool& secureConnection);

    QString baseURL(const bool& withCredential = true) const;

    QByteArray credential() const;
    void setCredential(const QString& username, const QString& password);

    bool hasCredential() const;

private:
    Q_DECLARE_PRIVATE(CouchDBServer)
    QScopedPointer<CouchDBServerPrivate> d_ptr;
};

#endif // COUCHDBSERVER_H
