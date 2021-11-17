#include "weekitem.h"
#include "ui_weekitem.h"
#include "lunar.h"

#include <QStyleOption>
#include <QPainter>

WeekItem::WeekItem(QWidget *parent) : QWidget(parent)
    , ui(new Ui::WeekItem)
{
    ui->setupUi(this);
    m_type = Holiday::Normal;
}

WeekItem::~WeekItem()
{
    delete ui;
}

void WeekItem::updateInfo(QDate date, Holiday::DayType type, bool currentMonth)
{
    m_type = type;
    bool isToday = date == QDate::currentDate();

    Lunar lunar;
    QMap<QVariant, QVariant> dd = lunar.solar2lunarDay(date.year(), date.month(), date.day());
    QString holiday = getHolidayName(date);

    ui->label->setText(QString::number(date.day()));
    ui->label_2->setText(QString("%1 %2 %3").arg(!dd.value("Holiday").toString().isEmpty() ? dd.value("Holiday").toString() : (dd.value("Iday") == 1 ? dd.value("ImonthCn").toString() : dd.value("IdayCn").toString()), dd.value("Term").toString(), holiday));

    QString color = "#282828";
    if(!currentMonth)
        color = "#464646";
    else if(date.dayOfWeek() == 7 || date.dayOfWeek() == 6 || !holiday.isEmpty())
        color = "red";
    else if(isToday)
        color = "blue";
    else if(type == Holiday::Rest)
        color = "black";
    else
        color = palette().color(QPalette::BrightText).name();

    ui->label->setStyleSheet(QString("color: %1").arg(color));

    if(!dd.value("Term").toString().isEmpty() || !dd.value("Holiday").toString().isEmpty())
        color = "red";
    else if(!currentMonth)
        color = "#464646";
    else if(isToday)
        color = "blue";
    else if(type != Holiday::Normal)
        color = "black";
    else
        color = palette().color(QPalette::BrightText).name();
    ui->label_2->setStyleSheet(QString("color: %1").arg(color));

    QString bgImg = "none";
    color = "transparent";
    if(type == Holiday::Work)
    {
        color = "#e8f1ff";
        bgImg = "url(:/icons/work.png)";
    } else if(type == Holiday::Rest)
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

QString WeekItem::getHolidayName(const QDate date)
{
    static QMap<QString, QString> holidays = {{"10-01", "国庆节"}, {"01-01", "元旦"}, {"05-01", "劳动节"}};
    return holidays.value(date.toString("MM-dd"));
}