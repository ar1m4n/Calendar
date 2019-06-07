#include "MainWindow.h"
#include "ui_MainWindow.h"
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QJsonDocument>
#include <QUrlQuery>
#include <QModelIndex>
#include <QJsonArray>
#include <QJsonObject>
#include "MagicCalendarWidget.h"
#include <QMessageBox>
#include <QRandomGenerator>

const QString MainWindow::API_KEY = "5cf957096ae1583c9e1e5540";

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    MagicCalendarWidget *m_calendar = new MagicCalendarWidget(ui->centralWidget);
    ui->centralWidget->layout()->addWidget(m_calendar);
    qDebug() << QSslSocket::supportsSsl();
    m_networkMgr = new QNetworkAccessManager(this);
    m_calendar->setMinimumDate(QDate::currentDate());
    connect(m_calendar, &MagicCalendarWidget::clicked, this, &MainWindow::onDateClicked);
    connect(m_networkMgr, &QNetworkAccessManager::finished, this, &MainWindow::onManagerFinished);
    InitCalendar();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::InitCalendar()
{
    QUrl requestUrl("https://magiccalendar-5046.restdb.io/rest/pastfortune");
    QNetworkRequest request(requestUrl);
    request.setRawHeader("accept", "application/json");
    request.setRawHeader("x-apikey", API_KEY.toUtf8());

    auto reply = m_networkMgr->get(request);
    m_handlers.insert(reply, [this](QNetworkReply *reply) {
        QString data = reply->readAll();
        QJsonDocument doc = QJsonDocument::fromJson(data.toLatin1());
        if(!doc.isEmpty())
        {
            auto array = doc.array();
            for(auto val : array)
            {
                auto obj = val.toObject();
                QDate fortuneDate = QDate::fromString(obj.find("date")->toString(), Qt::DateFormat::ISODate);
                auto dateInfo = m_calendar->SetDateImage(fortuneDate, DateInfo(true));
                auto fortuneArray = obj.find("fortune").value().toArray();
                for(auto pastFortuneRef : fortuneArray)
                {
                    auto link = pastFortuneRef.toObject().find("link")->toString();
                    if(!link.isEmpty())
                    {
                        auto picReply = m_networkMgr->get(QNetworkRequest(link));
                        m_handlers.insert(picReply, [dateInfo](QNetworkReply *reply) {
                            QByteArray pngData = reply->readAll();
                            dateInfo->pixmap.reset(new QPixmap());
                            dateInfo->pixmap->loadFromData(pngData);
                        });
                    }
                }
            }
        }
    });
}

void MainWindow::onManagerFinished(QNetworkReply *reply)
{
    auto found = m_handlers.find(reply);
    if(reply->error() == QNetworkReply::NoError)
    {
        if(found != m_handlers.end())
            found.value()(reply);
    }
    else
    {
        qDebug() << reply->errorString() << reply->error();
    }

    if(found != m_handlers.end())
        m_handlers.erase(found);

    reply->deleteLater();
}

void MainWindow::onDateClicked(const QDate &date)
{
    if(date > QDate::currentDate())
    {
//        auto info = QString("Опитай пак след %1 дни :)").arg(QDate::currentDate().daysTo(date));
//        QMessageBox::information(this, "Много бързаш :) :*", info);
//    }
//    else if(!m_calendar->GetDateInfo(date))
//    {
        QUrl requestUrl("https://magiccalendar-5046.restdb.io/rest/fortune");
        QNetworkRequest request(requestUrl);
        request.setRawHeader("accept", "application/json");
        request.setRawHeader("x-apikey", API_KEY.toUtf8());
        auto reply = m_networkMgr->get(request);
        m_handlers.insert(reply, [this](QNetworkReply *reply) {
            QString data = reply->readAll();
            QJsonDocument doc = QJsonDocument::fromJson(data.toLatin1());
            if(!doc.isEmpty())
            {
                QVector<QJsonObject> objects;
                auto array = doc.array();
                for(auto val : array)
                {
                    auto obj = val.toObject();
                    int count = obj.find("count")->toInt();
                    objects.insert(objects.end(), count, obj);
                }
                int idx = QRandomGenerator::system()->bounded(objects.size());
                auto & obj = objects.at(idx);
                QMessageBox::information(this, "За днес печалбата е: :) :*", obj.find("text")->toString());
            }
        });
    }
}
