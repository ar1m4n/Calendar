#include "MagicCalendarWidget.h"
#include <QPainter>
#include <QJsonArray>

MagicCalendarWidget::MagicCalendarWidget(QWidget *parent)
    :QCalendarWidget (parent)
{

}

DateInfo *MagicCalendarWidget::SetDateImage(const QDate &date, const DateInfo &dateInfo)
{
    return &m_pictures.insert(date, dateInfo).value();
}

DateInfo *MagicCalendarWidget::GetDateInfo(const QDate &date)
{
    auto found = m_pictures.find(date);
    if(found == m_pictures.end())
        return nullptr;

    return &found.value();
}

void MagicCalendarWidget::paintCell(QPainter *painter, const QRect &rect, const QDate &date) const
{
    QCalendarWidget::paintCell(painter, rect, date);
    auto picInfo = m_pictures.find(date);
    if(picInfo != m_pictures.end() && picInfo.value().pixmap)
    {
        auto oldOpacity = painter->opacity();
        painter->setOpacity(100.0 / 255);
        painter->drawPixmap(rect, *picInfo.value().pixmap.get(), picInfo->pixmap->rect());
        painter->setOpacity(oldOpacity);
    }
}

DateInfo::DateInfo(bool bPassed, const QJsonObject &obj)
    :passed(bPassed), m_obj(obj)
{

}

QString DateInfo::GetFortuneText()
{
    return m_obj.find("fortune").value().toArray().first().toObject().find("text").value().toString();
}
