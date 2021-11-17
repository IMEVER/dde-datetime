#ifndef WEEKWIDGET_H
#define WEEKWIDGET_H

#include "datetimeplugin.h"

#include <QWidget>
#include <QDate>

namespace Ui{
    class WeekWidget;
}

class WeekWidget : public QWidget
{
    Q_OBJECT
    public:
        explicit WeekWidget(DatetimePlugin *plugin, QWidget *parent=nullptr);
        ~WeekWidget();
        void updateTime();

    private slots:
        void showMonth();

    private:
        DatetimePlugin *m_plugin;
        Ui::WeekWidget *ui;
        QDate m_showDate;
        QMap<int, QList<Holiday>> m_holidays;
};

#endif