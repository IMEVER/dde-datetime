#include "weekwidget.h"

#include "ui_weekwidget.h"
#include "weekitem.h"
#include "lunar.h"

#include <QDateTime>
#include <QFont>
#include <QColor>
#include <QProcess>
#include <QItemDelegate>

class CMarginDelegate : public QItemDelegate
{
public:
    CMarginDelegate(QObject* parent)
        :   QItemDelegate(parent)
    {}

    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override
    {
        QStyleOptionViewItem itemOption(option);

        // Make the 'drawing rectangle' smaller.
        itemOption.rect.adjust(-1, 0, 0, 0);
        // itemOption.rect.setLeft(0);

        QItemDelegate::paint(painter, itemOption, index);
    }
};

WeekWidget::WeekWidget(DatetimePlugin *plugin, QWidget *parent) : QWidget(parent),
    m_plugin(plugin),
    ui(new Ui::WeekWidget)
{
    ui->setupUi(this);ui->tableWidget->setItemDelegate(new CMarginDelegate(this));

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

    ui->label->setText(QString("始皇：%1年 %2 %3%4 %5时 %6 %7\t公元：").arg(dd.value("lYear").toString(), dd.value("gzYear").toString(), dd.value("ImonthCn").toString(), dd.value("IdayCn").toString(), dd.value("gzHour").toString().at(1), dd.value("animal").toString(), dd.value("Term").toString())
        + now.date().toString("yyyy年MM月dd日")
    );

    m_showDate = QDate(now.date().year(), now.date().month(), 1);
    showMonth();
    ui->tableWidget->update();
}

void WeekWidget::showMonth()
{
    ui->currentLabel->setText(QString("%1月").arg(m_showDate.month()));
    ui->returnButton->setText(m_showDate.month() == QDate::currentDate().month() ? "今天" : "返回今天");

    auto getDayType = [this](QDate day) {
        if(m_holidays.contains(day.year()) && !m_holidays.value(day.year()).isEmpty())
        {
            for(Holiday holiday : m_holidays.value(day.year()))
            {
                Holiday::DayType type = holiday.getDayType(day);
                if(type != Holiday::Normal)
                    return type;
            }
        }

        return Holiday::Normal;
    };

    QDate start = m_showDate.dayOfWeek() == 7 ? m_showDate : m_showDate.addDays(-m_showDate.dayOfWeek());
    int row = 0, column = 0;
    while(row < 5 && column < 7)
    {
        if(!m_holidays.contains(start.year()))
        {
            QList<Holiday> holiday = m_plugin->getHolidays(start.year());
            m_holidays.insert(start.year(), holiday);
        }

        WeekItem *item = static_cast<WeekItem *>(ui->tableWidget->cellWidget(row, column));
        if(item == nullptr)
        {
            item = new WeekItem(this);
            ui->tableWidget->setCellWidget(row, column, item);
        }

        item->updateInfo(start, getDayType(start), start.month() == m_showDate.month());

        QTableWidgetItem *tmp = ui->tableWidget->item(row, column);
        if(!tmp)
        {
            tmp = new QTableWidgetItem();
            ui->tableWidget->setItem(row, column, tmp);
        }

        tmp->setFlags(start.month() == m_showDate.month() ? tmp->flags() | Qt::ItemIsSelectable : tmp->flags() & ~Qt::ItemIsSelectable);

        if(++column > 6)
        {
            column = 0;
            row++;
        }
        start = start.addDays(1);
    }

    ui->tableWidget->clearSelection();
}
