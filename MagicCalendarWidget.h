#ifndef MAGICCALENDARWIDGET_H
#define MAGICCALENDARWIDGET_H

#include <QCalendarWidget>
#include <QMap>
#include <QJsonObject>

struct DateInfo
{
    DateInfo(bool bPassed = false, const QJsonObject &obj = {});
    std::shared_ptr<QPixmap> pixmap = nullptr;
    bool passed;
    QJsonObject m_obj;
    QString GetFortuneText();
};

class MagicCalendarWidget : public QCalendarWidget
{
    Q_OBJECT
public:
    MagicCalendarWidget(QWidget *parent = nullptr);
    DateInfo *SetDateImage(const QDate &date, const DateInfo &dateInfo);
    DateInfo *GetDateInfo(const QDate &date);

protected:
    virtual void paintCell(QPainter *painter, const QRect &rect, const QDate &date) const;
    QMap<QDate, DateInfo> m_pictures;
};

#endif // MAGICCALENDARWIDGET_H
