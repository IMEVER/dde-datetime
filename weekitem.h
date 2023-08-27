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

        void updateInfo(QDate date, Holiday holiday, bool currentMonth);

    protected:
        void paintEvent(QPaintEvent *event);

    private:
        Ui::WeekItem *ui;
};

#endif