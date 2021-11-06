#include "weekwidget.h"

#include "ui_weekwidget.h"
#include "lunar.h"

#include <QDateTime>
#include <QFont>
#include <QColor>
#include <QProcess>


WeekWidget::WeekWidget(QWidget *parent) : QWidget(parent),
    ui(new Ui::WeekWidget)
{
    ui->setupUi(this);

    QStringList weeks = {"周日", "周一", "周二", "周三", "周四", "周五", "周六"};
    ui->tableWidget->setHorizontalHeaderLabels(weeks);

    connect(ui->preButton, &QPushButton::clicked, [this] {
        m_showDate = m_showDate.addMonths(-1);
        showMonth();
    });
    connect(ui->nextButton, &QPushButton::clicked, [this] {
        m_showDate = m_showDate.addMonths(1);
        showMonth();
    });
    connect(ui->returnButton, &QPushButton::clicked, [this]{
        m_showDate = QDate(QDate::currentDate().year(), QDate::currentDate().month(), 1);
        showMonth();
    });
    connect(ui->calButton, &QPushButton::clicked, this, []{
        QProcess::startDetached("dbus-send", {"--print-reply", "--dest=com.deepin.Calendar", "/com/deepin/Calendar", "com.deepin.Calendar.RaiseWindow"});
    });
}

WeekWidget::~WeekWidget(){
    delete ui;
}

void WeekWidget::updateTime()
{
    QDateTime now = QDateTime::currentDateTime();
    Lunar lunar;
    QMap<QVariant, QVariant> dd = lunar.solar2lunar(now.date().year(), now.date().month(), now.date().day(), now.time().hour());

    ui->label->setText(QString("始皇：%1年 %2 %3%4 %5时 %6 %7\t公元：").arg(dd.value("lYear").toString(), dd.value("gzYear").toString(), dd.value("ImonthCn").toString(), dd.value("IdayCn").toString(), dd.value("gzHour").toString(), dd.value("animal").toString(), dd.value("Term").toString())
        + now.date().toString("yyyy年MM月dd日")
    );

    m_showDate = QDate(now.date().year(), now.date().month(), 1);
    showMonth();
}

void WeekWidget::showMonth()
{
    int currentDay = QDate::currentDate().day();
    int currentMonth = QDate::currentDate().month();
    int row = 0, column = 0, index = 0;

    ui->currentLabel->setText(QString("%1月").arg(m_showDate.month()));
    ui->returnButton->setText(m_showDate.month() == currentMonth ? "今天" : "返回今天");

    QDate start = m_showDate.addDays(-m_showDate.dayOfWeek());

    Lunar lunar;
    QMap<QVariant, QVariant> dd;
    while(index < 35)
    {
        dd = lunar.solar2lunarDay(start.year(), start.month(), start.day());

        QTableWidgetItem *item = ui->tableWidget->item(row, column);
        if(item == nullptr)
        {
            item = new QTableWidgetItem();
            ui->tableWidget->setItem(row, column, item);
        }

        item->setText(QString("%1\n%2 %3").arg(QString::number(start.day()), dd.value("Iday") == 1 ? dd.value("ImonthCn").toString() : dd.value("IdayCn").toString(), dd.value("Term").toString()));

        if(start.day() == currentDay && start.month() == currentMonth)
        {
            item->setForeground(Qt::blue);
            item->setBackground(Qt::white);
            item->setTextAlignment(Qt::AlignRight);
            QFont font = item->font();
            font.setPixelSize(18);
            font.setBold(true);
            item->setFont(font);
        }
        else
        {
            QFont font = item->font();
            font.setPixelSize(14);
            font.setBold(false);
            item->setFont(font);

            item->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);

            item->setBackground(QColor("#282828"));
            if(start.month() != m_showDate.month())
            {
                item->setForeground(Qt::darkGray);
            }
            else
            {
                item->setForeground(Qt::white);
            }
        }


        if(++column > 6)
        {
            column = 0;
            row++;
        }
        start = start.addDays(1);
        index++;
    }
}
