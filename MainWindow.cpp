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
    m_calendar = new MagicCalendarWidget();
    ui->centralWidget->layout()->addWidget(m_calendar);
    qDebug() << QSslSocket::supportsSsl();
    m_networkMgr = new QNetworkAccessManager(this);
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
    m_calendar->setEnabled(false);
    m_handlers.insert(reply, [this](QNetworkReply *reply) {
        QString data = reply->readAll();
        QJsonDocument doc = QJsonDocument::fromJson(data.toUtf8());
        m_calendar->setEnabled(true);
        if(!doc.isEmpty())
        {
            auto array = doc.array();
            for(auto val : array)
            {
                auto obj = val.toObject();
                QDate fortuneDate = QDate::fromString(obj.find("date")->toString(), Qt::DateFormat::ISODate);
                auto dateInfo = m_calendar->SetDateImage(fortuneDate, DateInfo(true, obj));
                auto fortuneArray = obj.find("fortune").value().toArray();
                for(auto pastFortuneRef : fortuneArray)
                {
                    auto link = pastFortuneRef.toObject().find("link")->toString();
                    if(!link.isEmpty())
                    {
                        auto picReply = m_networkMgr->get(QNetworkRequest(link));
                        m_handlers.insert(picReply, [dateInfo, this, fortuneDate](QNetworkReply *reply) {
                            QByteArray pngData = reply->readAll();
                            dateInfo->pixmap.reset(new QPixmap());
                            dateInfo->pixmap->loadFromData(pngData);
                            QDate dateToSelect = fortuneDate;
                            if(dateToSelect == QDate::currentDate())
                                dateToSelect = QDate::currentDate().addDays(-1);
                            m_calendar->setSelectedDate(dateToSelect);
                            m_calendar->setSelectedDate(QDate::currentDate());
                        });
                    }
                }
            }
        }
    });
}

void MainWindow::GetFortune()
{
    QUrl requestUrl("https://magiccalendar-5046.restdb.io/rest/fortune");
    QNetworkRequest request(requestUrl);
    request.setRawHeader("accept", "application/json");
    request.setRawHeader("x-apikey", API_KEY.toUtf8());
    auto reply = m_networkMgr->get(request);
    m_handlers.insert(reply, [this](QNetworkReply *reply) {
        QString data = reply->readAll();
        QJsonDocument doc = QJsonDocument::fromJson(data.toUtf8());
        if(!doc.isEmpty())
        {
            QVector<QJsonObject> objects;
            auto array = doc.array();
            for(auto val : array)
            {
                auto obj = val.toObject();
                int count = obj.find("count")->toInt();
                if(count)
                    objects.insert(objects.end(), count, obj);
            }

            if(!objects.empty())
            {
                int idx = QRandomGenerator::system()->bounded(objects.size());
                auto & obj = objects.at(idx);
                auto btn = QMessageBox::information(this, "За днес печалбата е: :) :*", obj.find("text")->toString());
                if(btn == QMessageBox::StandardButton::Ok)
                {
                    SetPassedFortune(QDate::currentDate(), obj);
                }
            }
        }
    });
}

void MainWindow::SetPassedFortune(const QDate &date, const QJsonObject &fortune)
{
    QUrl requestUrl("https://magiccalendar-5046.restdb.io/rest/pastfortune");
    auto data = QString("{\"date\":\"%1\",\"fortune\":[\"%2\"]}")
                .arg(date.toString(Qt::DateFormat::ISODate))
                .arg(fortune.find("_id")->toString());

    QNetworkRequest request(requestUrl);
    request.setRawHeader("Content-Type", "application/json");
    request.setRawHeader("x-apikey", API_KEY.toUtf8());
    auto reply = m_networkMgr->post(request, data.toUtf8());
    m_handlers.insert(reply, [this, date, fortune](QNetworkReply*) {
        SendEmail(date, fortune);
    });
}

void MainWindow::SendEmail(const QDate &date, const QJsonObject &fortune)
{
    QUrl requestUrl("https://magiccalendar-5046.restdb.io/mail");
    QNetworkRequest request(requestUrl);
    request.setRawHeader("Content-Type", "application/json");
    request.setRawHeader("x-apikey", API_KEY.toUtf8());

    QJsonObject obj;
    obj.insert("to", "ar1m4n@gmail.com");
    obj.insert("subject", QString("Подарък за %1").arg(date.toString("dd.MM.yyyy")));
    obj.insert("html", QString("<p><b>%1</b><img src=\"%2\" /></p>")
                .arg(fortune.find("text")->toString())
                .arg(fortune.find("link")->toString()));
    obj.insert("sendername", "Magic Calendar");

    QJsonDocument doc(obj);
    auto data = QString(doc.toJson(QJsonDocument::Compact)).toUtf8();
    auto reply = m_networkMgr->post(request, data);
    m_handlers.insert(reply, [this](QNetworkReply*) {
        InitCalendar();
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
        auto info = QString("Опитай пак след %1 дни :)").arg(QDate::currentDate().daysTo(date));
        QMessageBox::information(this, "Много бързаш :) :*", info);
    }
    else
    {
        auto dateInfo = m_calendar->GetDateInfo(date);
        if(date == QDate::currentDate() && !dateInfo)
            GetFortune();
        else if(dateInfo)
        {
            QMessageBox::information(this, QString("Наградата за %1 е:").arg(date.toString("dd.MM.yyyy")),
                                     dateInfo->GetFortuneText());
        }
    }
}
