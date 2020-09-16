#include "couchdblistener.h"
#include "couchdb.h"
#include "couchdb_p.h"

#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QUrl>
#include <QUrlQuery>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QTimer>
#include <QDebug>

class CouchDBListenerPrivate
{
public:
    virtual ~CouchDBListenerPrivate()
    {
        if(retryTimer) retryTimer->stop();
        if(reply) reply->abort();

        if(reply) delete reply;
        if(retryTimer) delete retryTimer;

        if(networkManager) delete networkManager;
    }

    QUrl server;
    QNetworkAccessManager *networkManager = nullptr;
    QString database;
    QString documentID;
    QNetworkReply *reply = nullptr;
    QTimer* retryTimer = nullptr;
    QMap<QString,QString> parameters;
    QMap<QString,QString> revisionsMap;
};

CouchDBListener::CouchDBListener(const QUrl &server, QObject *parent) :
    QObject(parent),
    d_ptr(new CouchDBListenerPrivate)
{
    Q_D(CouchDBListener);
    d->server = server;
    d->networkManager = new QNetworkAccessManager(this);
    connect(d->networkManager, SIGNAL(finished(QNetworkReply*)),  this, SLOT(listenFinished(QNetworkReply*)));

    d->retryTimer = new QTimer(this);
    d->retryTimer->setInterval(500);
    d->retryTimer->setSingleShot(true);
    connect(d->retryTimer, SIGNAL(timeout()), this, SLOT(start()));

    d->parameters.insert("filter", "app/docFilter");
    d->parameters.insert("feed", "continuous");
    d->parameters.insert("heartbeat", "10000");
    d->parameters.insert("timeout", "60000");
}

CouchDBListener::~CouchDBListener()
{
}

QUrl CouchDBListener::server() const
{
    Q_D(const CouchDBListener);
    return d->server;
}

QString CouchDBListener::database() const
{
    Q_D(const CouchDBListener);
    return d->database;
}

void CouchDBListener::setDatabase(const QString &database)
{
    Q_D(CouchDBListener);
    d->database = database;
}

QString CouchDBListener::documentID() const
{
    Q_D(const CouchDBListener);
    return d->documentID;
}

void CouchDBListener::setDocumentID(const QString &documentID)
{
    Q_D(CouchDBListener);
    d->documentID = documentID;
    d->parameters.insert("name", d->documentID);
}

QString CouchDBListener::revision(const QString &documentID) const
{
    Q_D(const CouchDBListener);

    QString docID = d->documentID;
    if(!documentID.isEmpty()) docID = documentID;

    return d->revisionsMap.value(docID);
}

void CouchDBListener::setCookieJar(QNetworkCookieJar *cookieJar)
{
    Q_D(CouchDBListener);
    d->networkManager->setCookieJar(cookieJar);
}

void CouchDBListener::setParam(const QString& name, const QString& value)
{
    Q_D(CouchDBListener);
    d->parameters.insert(name, value);
}

void CouchDBListener::launch()
{
    Q_D(CouchDBListener);
    d->retryTimer->start();
}

void CouchDBListener::start()
{
    Q_D(CouchDBListener);

    QUrlQuery urlQuery;
    QMapIterator<QString, QString> i(d->parameters);
    while (i.hasNext())
    {
        i.next();
        urlQuery.addQueryItem(i.key(), i.value());
    }

    QUrl url = d->server;
    url.setPath(QStringLiteral("%1/_changes").arg(d->database));
    url.setQuery(urlQuery);

    qDebug() << url;

    QNetworkRequest request(url);
    QString username = d->server.userName();
    QString password = d->server.password();
    if (!username.isEmpty() && !password.isEmpty())
        request.setRawHeader("Authorization", CouchDBPrivate::basicAuth(username, password));

    d->reply = d->networkManager->get(request);
    connect(d->reply, SIGNAL(readyRead()), this, SLOT(readChanges()));
}

void CouchDBListener::readChanges()
{
    Q_D(CouchDBListener);

    const QByteArray replyBA = d->reply->readAll();
    QJsonDocument document = QJsonDocument::fromJson(replyBA);

    if(!document.object().contains("changes")) return;

    QString revision = document.object().value("changes").toArray().first().toObject().value("rev").toString();
    QString docID = d->documentID.isEmpty() ? document.object().value("id").toString() : d->documentID;

    //If the revision is the same as previous changes return
    if(d->revisionsMap.value(docID) == revision) return;

    d->revisionsMap.insert(docID, revision);
    emit changesMade(revision);
}

void CouchDBListener::listenFinished(QNetworkReply *reply)
{
    Q_D(CouchDBListener);

    // Check the network reply for errors.
    QNetworkReply::NetworkError netError = reply->error();
    if(netError != QNetworkReply::NoError)
    {
        qWarning() << "ERROR";
        switch(netError)
        {
        case QNetworkReply::ContentNotFoundError:
            qWarning() << "The content was not found on the server";
            break;

        case QNetworkReply::HostNotFoundError:
            qWarning() << "The server was not found";
            break;

        default:
            qWarning() << reply->errorString();
            break;
        }

    }
    reply->deleteLater();
    d->retryTimer->start();
}
