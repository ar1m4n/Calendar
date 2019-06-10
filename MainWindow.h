#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QNetworkReply>
#include <QTimer>

class QNetworkReply;
class QNetworkAccessManager;
class MagicCalendarWidget;

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT
    
public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    void InitCalendar();
    
private:
    Ui::MainWindow *ui;
    QNetworkAccessManager *m_networkMgr;
    static const QString API_KEY;
    using Handlers = QMap<QNetworkReply*, std::function<void(QNetworkReply *)>>;
    Handlers m_handlers;
    MagicCalendarWidget *m_calendar;

    void GetFortune();
    void SetPassedFortune(const QDate &date, const QJsonObject &fortune);
    void SendEmail(const QDate &date, const QJsonObject &fortune);

private slots:
    void onManagerFinished(QNetworkReply *reply);
    void onDateClicked(const QDate &date);
};

#endif // MAINWINDOW_H
