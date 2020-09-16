#include "couchdbquery.h"

#include <QtCore/qtimer.h>
#include <QtCore/qurl.h>
#include <QtNetwork/qnetworkrequest.h>

class CouchDBQueryPrivate
{
public:
    QUrl server;
    QScopedPointer<QNetworkRequest> request;
    CouchDBOperation operation = COUCHDB_CHECKINSTALLATION;
    QString database;
    QString documentID;
    QByteArray body;
    QTimer *timer = nullptr;
};

CouchDBQuery::CouchDBQuery(const QUrl &server, QObject *parent) :
    QObject(parent),
    d_ptr(new CouchDBQueryPrivate)
{
    Q_D(CouchDBQuery);
    d->server = server;
    d->request.reset(new QNetworkRequest);
    d->timer = new QTimer(this);
    d->timer->setInterval(20000); // ### TODO
    d->timer->setSingleShot(true);
    connect(d->timer, SIGNAL(timeout()), SIGNAL(timeout()));
}

CouchDBQuery::~CouchDBQuery()
{
}

QUrl CouchDBQuery::server() const
{
    Q_D(const CouchDBQuery);
    return d->server;
}

QNetworkRequest *CouchDBQuery::request() const
{
    Q_D(const CouchDBQuery);
    return d->request.data();
}

QUrl CouchDBQuery::url() const
{
    Q_D(const CouchDBQuery);
    return d->request->url();
}

void CouchDBQuery::setUrl(const QUrl &url)
{
    Q_D(CouchDBQuery);
    d->request->setUrl(url);
}

CouchDBOperation CouchDBQuery::operation() const
{
    Q_D(const CouchDBQuery);
    return d->operation;
}

void CouchDBQuery::setOperation(CouchDBOperation operation)
{
    Q_D(CouchDBQuery);
    d->operation = operation;
}

QString CouchDBQuery::database() const
{
    Q_D(const CouchDBQuery);
    return d->database;
}

void CouchDBQuery::setDatabase(const QString &database)
{
    Q_D(CouchDBQuery);
    d->database = database;
}

QString CouchDBQuery::documentId() const
{
    Q_D(const CouchDBQuery);
    return d->documentID;
}

void CouchDBQuery::setDocumentId(const QString &documentId)
{
    Q_D(CouchDBQuery);
    d->documentID = documentId;
}

QByteArray CouchDBQuery::body() const
{
    Q_D(const CouchDBQuery);
    return d->body;
}

void CouchDBQuery::setBody(const QByteArray &body)
{
    Q_D(CouchDBQuery);
    d->body = body;
}

void CouchDBQuery::startTimeoutTimer()
{
    Q_D(CouchDBQuery);
    d->timer->start();
}
