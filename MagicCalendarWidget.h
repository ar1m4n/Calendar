#ifndef MAGICCALENDARWIDGET_H
#define MAGICCALENDARWIDGET_H

#include <QCalendarWidget>
#include <QMap>

struct DateInfo
{
    DateInfo(bool bPassed = false, const QString &text = "");
    std::shared_ptr<QPixmap> pixmap = nullptr;
    bool passed;
    QString m_text;
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
