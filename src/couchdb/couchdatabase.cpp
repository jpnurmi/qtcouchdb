﻿#include "couchdatabase.h"
#include "couchclient.h"
#include "couchrequest.h"
#include "couchresponse.h"

class CouchDatabasePrivate
{
    Q_DECLARE_PUBLIC(CouchDatabase)

public:
    CouchResponse *response(CouchResponse *response)
    {
        Q_Q(CouchDatabase);
        QObject::connect(response, &CouchResponse::errorOccurred, [=](const CouchError &error) {
            emit q->errorOccurred(error);
        });
        return response;
    }

    CouchDatabase *q_ptr = nullptr;
    QString name;
    CouchClient *client = nullptr;
};

CouchDatabase::CouchDatabase(QObject *parent)
    : CouchDatabase(QString(), nullptr, parent)
{
}

CouchDatabase::CouchDatabase(const QString &name, CouchClient *client, QObject *parent)
    : QObject(parent),
    d_ptr(new CouchDatabasePrivate)
{
    Q_D(CouchDatabase);
    d->q_ptr = this;
    d->name = name;
    setClient(client);
}

CouchDatabase::~CouchDatabase()
{
}

QUrl CouchDatabase::url() const
{
    Q_D(const CouchDatabase);
    if (!d->client || d->name.isEmpty())
        return QUrl();

    return Couch::databaseUrl(d->client->baseUrl(), d->name);
}

QString CouchDatabase::name() const
{
    Q_D(const CouchDatabase);
    return d->name;
}

void CouchDatabase::setName(const QString &name)
{
    Q_D(CouchDatabase);
    if (d->name == name)
        return;

    d->name = name;
    emit urlChanged(url());
    emit nameChanged(name);
}

CouchClient *CouchDatabase::client() const
{
    Q_D(const CouchDatabase);
    return d->client;
}

void CouchDatabase::setClient(CouchClient *client)
{
    Q_D(CouchDatabase);
    if (d->client == client)
        return;

    if (d->client)
        d->client->disconnect(this);
    if (client)
        connect(client, &CouchClient::baseUrlChanged, this, &CouchDatabase::urlChanged);

    d->client = client;
    emit urlChanged(url());
    emit clientChanged(client);
}

static QString trimPrefix(QString str, const QString &prefix)
{
    if (!str.startsWith(prefix))
        return str;

    return str.mid(prefix.length());
}

static QString toDesignDocumentName(const QJsonObject &json)
{
    QString key = json.value(QStringLiteral("key")).toString();
    return trimPrefix(key, QStringLiteral("_design/"));
}

static QStringList toDesignDocumentList(const QJsonArray &json)
{
    QStringList designs;
    for (const QJsonValue &value : json)
        designs += toDesignDocumentName(value.toObject());
    return designs;
}

bool CouchDatabase::listDesignDocuments()
{
    Q_D(CouchDatabase);
    if (!d->client)
        return false;

    CouchRequest request = Couch::listDesignDocuments(url());
    CouchResponse *response = d->client->sendRequest(request);
    if (!response)
        return false;

    connect(response, &CouchResponse::received, [=](const QByteArray &data) {
        QJsonDocument json = QJsonDocument::fromJson(data);
        QJsonArray rows = json.object().value(QStringLiteral("rows")).toArray();
        emit designDocumentsListed(toDesignDocumentList(rows));
    });
    return d->response(response);
}

bool CouchDatabase::createDesignDocument(const QString &designDocument)
{
    Q_D(CouchDatabase);
    if (!d->client)
        return false;

    CouchRequest request = Couch::createDesignDocument(Couch::designDocumentUrl(url(), designDocument));
    CouchResponse *response = d->client->sendRequest(request);
    if (!response)
        return false;

    connect(response, &CouchResponse::received, [=](const QByteArray &) {
        emit designDocumentCreated(designDocument);
    });
    return d->response(response);
}

bool CouchDatabase::deleteDesignDocument(const QString &designDocument)
{
    Q_D(CouchDatabase);
    if (!d->client)
        return false;

    CouchRequest request = Couch::deleteDesignDocument(Couch::designDocumentUrl(url(), designDocument));
    CouchResponse *response = d->client->sendRequest(request);
    if (!response)
        return false;

    connect(response, &CouchResponse::received, [=](const QByteArray &) {
        emit designDocumentDeleted(designDocument);
    });
    return d->response(response);
}

static QList<CouchDocument> toDocumentList(const QJsonArray &json)
{
    QList<CouchDocument> docs;
    for (const QJsonValue &value : json)
        docs += CouchDocument::fromJson(value.toObject());
    return docs;
}

bool CouchDatabase::listDocumentIds()
{
    return queryDocuments(CouchQuery());
}

bool CouchDatabase::listFullDocuments()
{
    return queryDocuments(CouchQuery::full());
}

bool CouchDatabase::queryDocuments(const CouchQuery &query)
{
    Q_D(CouchDatabase);
    if (!d->client)
        return false;

    CouchRequest request = Couch::queryDocuments(url(), query);
    CouchResponse *response = d->client->sendRequest(request);
    if (!response)
        return false;

    connect(response, &CouchResponse::received, [=](const QByteArray &data) {
        QJsonDocument json = QJsonDocument::fromJson(data);
        QJsonArray rows = json.object().value(QStringLiteral("rows")).toArray();
        emit documentsListed(toDocumentList(rows));
    });
    return d->response(response);
}

bool CouchDatabase::createDocument(const CouchDocument &document)
{
    Q_D(CouchDatabase);
    if (!d->client)
        return false;

    CouchRequest request = Couch::createDocument(url(), QJsonDocument(document.toJson()).toJson(QJsonDocument::Compact));
    CouchResponse *response = d->client->sendRequest(request);
    if (!response)
        return false;

    connect(response, &CouchResponse::received, [=](const QByteArray &data) {
        QJsonDocument json = QJsonDocument::fromJson(data);
        emit documentCreated(CouchDocument::fromJson(json.object()));
    });
    return d->response(response);
}

bool CouchDatabase::getDocument(const CouchDocument &document)
{
    Q_D(CouchDatabase);
    if (!d->client)
        return false;

    CouchRequest request = Couch::getDocument(url(), document.id(), document.revision());
    CouchResponse *response = d->client->sendRequest(request);
    if (!response)
        return false;

    connect(response, &CouchResponse::received, [=](const QByteArray &data) {
        QJsonDocument json = QJsonDocument::fromJson(data);
        emit documentReceived(CouchDocument::fromJson(json.object()));
    });
    return d->response(response);
}

bool CouchDatabase::updateDocument(const CouchDocument &document)
{
    Q_D(CouchDatabase);
    if (!d->client)
        return false;

    CouchRequest request = Couch::updateDocument(url(), document.id(), document.revision(), document.content());
    CouchResponse *response = d->client->sendRequest(request);
    if (!response)
        return false;

    connect(response, &CouchResponse::received, [=](const QByteArray &data) {
        QJsonDocument json = QJsonDocument::fromJson(data);
        emit documentUpdated(CouchDocument::fromJson(json.object()));
    });
    return d->response(response);
}

bool CouchDatabase::deleteDocument(const CouchDocument &document)
{
    Q_D(CouchDatabase);
    if (!d->client)
        return false;

    CouchRequest request = Couch::deleteDocument(url(), document.id(), document.revision());
    CouchResponse *response = d->client->sendRequest(request);
    if (!response)
        return false;

    connect(response, &CouchResponse::received, [=](const QByteArray &data) {
        QJsonDocument json = QJsonDocument::fromJson(data);
        emit documentDeleted(CouchDocument::fromJson(json.object()));
    });
    return d->response(response);
}
