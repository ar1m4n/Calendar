#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QNetworkReply>

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
    QMap<QNetworkReply*, std::function<void(QNetworkReply *)>> m_handlers;
    MagicCalendarWidget *m_calendar;


private slots:
    void onManagerFinished(QNetworkReply *reply);
    void onDateClicked(const QDate &date);
};

#endif // MAINWINDOW_H
