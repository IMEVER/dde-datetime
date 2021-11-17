#ifndef WEEKITEM_H
#define WEEKITEM_H

#include "datetimeplugin.h"

#include <QWidget>
#include <QDate>

namespace Ui{
    class WeekItem;
}

class WeekItem : public QWidget
{
    Q_OBJECT

    public:
        WeekItem(QWidget *parent=nullptr);
        ~WeekItem();

        void updateInfo(QDate date, Holiday::DayType type, bool currentMonth);

    protected:
        void paintEvent(QPaintEvent *event);

    private:
        QString getHolidayName(const QDate date);

    private:
        Ui::WeekItem *ui;
        Holiday::DayType m_type;
};

#endif