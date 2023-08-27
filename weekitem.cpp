#include "weekitem.h"
#include "ui_weekitem.h"
#include "lunar.h"

#include <QStyleOption>
#include <QPainter>
#include <QGuiApplication>

WeekItem::WeekItem(QWidget *parent) : QWidget(parent)
    , ui(new Ui::WeekItem)
{
    ui->setupUi(this);
}

WeekItem::~WeekItem()
{
    delete ui;
}

void WeekItem::updateInfo(QDate date, Holiday holiday, bool currentMonth)
{
    const Holiday::DayType m_type = holiday.getDayType(date);
    const bool isToday = date == QDate::currentDate();

    Lunar lunar;
    const QMap<QVariant, QVariant> &dd = lunar.solar2lunarDay(date.year(), date.month(), date.day());

    ui->label->setText(QString::number(date.day()));
    ui->label_2->setText(QString("%1 %2 %3").arg(dd.value("Holiday", dd.value("Iday") == 1 ? dd.value("ImonthCn").toString() : dd.value("IdayCn").toString()).toString(), dd.value("Term").toString(), date == holiday.m_day ? holiday.m_title : ""));

    ui->verticalLayoutWidget->setToolTip(m_type == Holiday::Rest ? holiday.m_title : "");

    QColor base = QGuiApplication::palette().color(QPalette::Window);
    QColor disableColor = base.value() < 128 ? QColor(base.red() + 48, base.green() + 48, base.blue() + 48) : QColor(base.red() - 48, base.green() - 48, base.blue() - 48);
    QString color = "#282828";
    if(!currentMonth && m_type == Holiday::Normal)
        color = disableColor.name(); //"#464646";
    else if(date.dayOfWeek() == 7 || date.dayOfWeek() == 6 || !holiday.m_title.isEmpty())
        color = "red";
    else if(isToday)
        color = "blue";
    else if(m_type != Holiday::Normal)
        color = "black";
    else
        color = palette().color(QPalette::BrightText).name();

    ui->label->setStyleSheet(QString("color: %1").arg(color));

    if(!dd.value("Term").toString().isEmpty() || !dd.value("Holiday").toString().isEmpty())
        color = "red";
    else if(dd.value("Iday") == 1)
        color = "#9370db";
    else if(isToday)
        color = "blue";
    else if(m_type != Holiday::Normal)
        color = "black";
    else if(!currentMonth)
        color = disableColor.name(); //"#464646";
    else
        color = palette().color(QPalette::BrightText).name();
    ui->label_2->setStyleSheet(QString("color: %1").arg(color));

    QString bgImg = "none";
    color = "transparent";
    if(m_type == Holiday::Work)
    {
        color = "#e8f1ff";
        bgImg = "url(:/icons/work.png)";
    } else if(m_type == Holiday::Rest)
    {
        color = "#ffeaef";
        bgImg = "url(:/icons/rest.png)";
    } else if(isToday)
        color = "white";

    setStyleSheet(QString("WeekItem { margin: 1px; background-color: %1; border-width: 0; border-image: %2 0 0 0 0 stretch stretch; }").arg(color, bgImg));
}

void WeekItem::paintEvent(QPaintEvent *event)
{
    QStyleOption opt;
    opt.init(this);
    QPainter p(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
}
