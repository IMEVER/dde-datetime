#ifndef WEEKWIDGET_H
#define WEEKWIDGET_H

#include <QWidget>
#include <QDate>

namespace Ui{
    class WeekWidget;
}

class WeekWidget : public QWidget
{
    Q_OBJECT
    public:
        explicit WeekWidget(QWidget *parent=nullptr);
        ~WeekWidget();
        void updateTime();

    private slots:
        void showMonth();

    private:
        Ui::WeekWidget *ui;
        QDate m_showDate;
};

#endif