﻿#include "couchview.h"
#include "couchclient.h"
#include "couchdatabase.h"
#include "couchdesigndocument.h"
#include "couchrequest.h"
#include "couchresponse.h"

class CouchViewPrivate
{
    Q_DECLARE_PUBLIC(CouchView)

public:
    CouchResponse *response(CouchResponse *response)
    {
        Q_Q(CouchView);
        QObject::connect(response, &CouchResponse::errorOccurred, [=](const CouchError &error) {
            emit q->errorOccurred(error);
        });
        return response;
    }

    CouchView *q_ptr = nullptr;
    QString name;
    CouchDesignDocument *designDocument = nullptr;
};

CouchView::CouchView(QObject *parent)
    : CouchView(QString(), nullptr, parent)
{
}

CouchView::CouchView(const QString &name, CouchDesignDocument *designDocument, QObject *parent)
    : QObject(parent),
    d_ptr(new CouchViewPrivate)
{
    Q_D(CouchView);
    d->q_ptr = this;
    d->name = name;
    d->designDocument = designDocument;
}

CouchView::~CouchView()
{
}

QUrl CouchView::url() const
{
    Q_D(const CouchView);
    if (!d->designDocument || d->name.isEmpty())
        return QUrl();

    return Couch::viewUrl(d->designDocument->url(), d->name);
}

QString CouchView::name() const
{
    Q_D(const CouchView);
    return d->name;
}

void CouchView::setName(const QString &name)
{
    Q_D(CouchView);
    if (d->name == name)
        return;

    d->name = name;
    emit urlChanged(url());
    emit nameChanged(name);
}

CouchClient *CouchView::client() const
{
    Q_D(const CouchView);
    if (!d->designDocument)
        return nullptr;

    return d->designDocument->client();
}

CouchDatabase *CouchView::database() const
{
    Q_D(const CouchView);
    if (!d->designDocument)
        return nullptr;

    return d->designDocument->database();
}

CouchDesignDocument *CouchView::designDocument() const
{
    Q_D(const CouchView);
    return d->designDocument;
}

void CouchView::setDesignDocument(CouchDesignDocument *designDocument)
{
    Q_D(CouchView);
    if (d->designDocument == designDocument)
        return;

    d->designDocument = designDocument;
    emit urlChanged(url());
    emit designDocumentChanged(designDocument);
}

CouchResponse *CouchView::listRows(Couch::Query query)
{
    Q_D(CouchView);
    CouchClient *client = d->designDocument ? d->designDocument->client() : nullptr;
    if (!client)
        return nullptr;

    CouchRequest request = Couch::listRows(url(), query);
    CouchResponse *response = client->sendRequest(request);
    connect(response, &CouchResponse::received, [=](const QByteArray &data) {
        QJsonDocument json = QJsonDocument::fromJson(data);
        QJsonArray rows = json.object().value(QStringLiteral("rows")).toArray();
        emit rowsListed(rows);
    });
    return d->response(response);
}
