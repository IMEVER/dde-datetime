#include "settingdialog.h"
#include "ui_settings.h"
#include "lunar.h"
#include "datetimeplugin.h"

#include <QDateTime>
#include <QComboBox>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QStandardPaths>
#include <QTextCodec>
#include <QMessageBox>
#include <QModelIndex>
#include <QInputDialog>

static const QString confFile = QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation) + "/dde-datetime.ini";

SettingDialog::SettingDialog() : QDialog(), ui(new Ui::SettingDialog)
    , m_conf(new QSettings(confFile, QSettings::IniFormat, this))
{
    ui->setupUi(this);

    m_conf->setIniCodec(QTextCodec::codecForName("UTF-8"));

    ui->m_desktopClock->setChecked(m_conf->value("showDesktopClock", true).toBool());
    ui->m_editClock->setChecked(false);
    // ui->m_editClock->setEnabled(ui->m_desktopClock->isChecked());
    ui->m_clockSize->setValue(m_conf->value("clockSize", 300).toInt());
    // ui->m_clockSize->setEnabled(false);
    ui->m_showGanzhi->setChecked(m_conf->value("showGanzhi", true).toBool());
    // ui->m_showGanzhi->setEnabled(false);
    ui->m_showLunar->setChecked(m_conf->value("showLunar", true).toBool());
    // ui->m_showLunar->setEnabled(false);
    connect(ui->m_editClock, &QCheckBox::clicked, this, &SettingDialog::clockEditeableChanged);
    connect(ui->m_showGanzhi, &QCheckBox::clicked, this, [this](bool enabled){
        m_conf->setValue("showGanzhi", enabled);
        m_conf->sync();
        emit clockShowGanzhiChanged(enabled);
    });
    connect(ui->m_showLunar, &QCheckBox::clicked, this, [this](bool enabled){
        m_conf->setValue("showLunar", enabled);
        m_conf->sync();
        emit clockShowLunarChanged(enabled);
    });
    connect(ui->m_desktopClock, &QGroupBox::clicked, this, [this](bool checked) {
        // ui->m_editClock->setEnabled(checked);
        if(!checked) {
            ui->m_editClock->setChecked(false);
            // ui->m_clockSize->setEnabled(false);
        }
        m_conf->setValue("showDesktopClock", checked);
        m_conf->sync();
        emit clockChanged(checked);
    });
    connect(ui->m_clockSize, &QSlider::valueChanged, this, [this](int value) {
        m_conf->setValue("clockSize", value);
        m_conf->sync();
        emit clockSizeChanged(value);
    });

    ui->m_formatEdit->setText(m_conf->value("format", "yyyy/MM/dd HH:mm:ss").toString());
    connect(ui->m_formatEdit, &QLineEdit::textEdited, this, &SettingDialog::updateCurrentChinaTime);
    connect(ui->m_applyButton, &QPushButton::clicked, this, [this]{
        QString format = ui->m_formatEdit->text().trimmed();
        if(format.isEmpty() == false && format != m_conf->value("format", "yyyy/MM/dd HH:mm").toString().trimmed()) {
            m_conf->setValue("format", format);
            m_conf->sync();
            emit formattingChanged(format);
        }
    });
    updateCurrentChinaTime(ui->m_formatEdit->text());

    ui->tableDay->setHorizontalHeaderLabels({"节日名称", "节日日期", "开始日期", "结束日期", "调休日期"});
    ui->tableDay->setSelectionMode(QTableWidget::SingleSelection);
    ui->tableDay->setSelectionBehavior(QTableWidget::SelectRows);
    connect(ui->m_addYear, &QPushButton::clicked, this, [this](bool checked){
        static QStringList years;
        if(years.isEmpty()) {
            const int currentYear = QDate::currentDate().year();
            for (int i=-3; i < 10; i++) years.append(QString::number(currentYear + i));
        }

        bool ok;
        QString y = QInputDialog::getItem(this, "年份", "请选择年份", years, 0, false, &ok);
        if(ok and !y.isEmpty()) {
            int index = ui->comboBox->findText(y);
            if(index == -1) {
                ui->comboBox->addItem(y);
                index = ui->comboBox->count()-1;
            }
            ui->comboBox->setCurrentIndex(index);
        }
    });
    connect(ui->m_delYear, &QPushButton::clicked, this, [this](bool checked){
        const QString year = ui->comboBox->currentText().trimmed();
        if(year.isEmpty() == false && QMessageBox::question(this, "警告", "确认要删除: " + year) == QMessageBox::Yes) {
            m_conf->remove(year);
            m_conf->sync();
            ui->comboBox->removeItem(ui->comboBox->currentIndex());
            emit holidayChanged(year);
        }
    });
    connect(ui->m_delDay, &QPushButton::clicked, this, [this](bool checked){
        const QString year = ui->comboBox->currentText().trimmed();
        const int currentRow = ui->tableDay->currentRow();
        if(!year.isEmpty() && currentRow>=0 && QMessageBox::question(this, "警告", "确认要删除当前行？") == QMessageBox::Yes) {
            ui->tableDay->removeRow(currentRow);
            const int row = ui->tableDay->rowCount();
            m_conf->beginWriteArray(year, row);
            for (int i = 0; i < row; i++) {
                m_conf->setArrayIndex(i);
                m_conf->setValue("title", ui->tableDay->item(i, 0)->text());
                m_conf->setValue("day", ui->tableDay->item(i, 1)->text());
                m_conf->setValue("start", ui->tableDay->item(i, 2)->text());
                m_conf->setValue("end", ui->tableDay->item(i, 3)->text());
                m_conf->setValue("work", ui->tableDay->item(i, 4)->text());
            }
            m_conf->endArray();
            m_conf->sync();
            emit holidayChanged(year);
        }
    });
    connect(ui->m_addDay, &QPushButton::clicked, this, [this](bool checked){
        if(ui->comboBox->currentText().trimmed().isEmpty() == false) {
            ui->tableDay->blockSignals(true);
            int row = ui->tableDay->rowCount();
            ui->tableDay->insertRow(row);
            ui->tableDay->setItem(row, 0, new QTableWidgetItem());
            ui->tableDay->setItem(row, 1, new QTableWidgetItem());
            ui->tableDay->setItem(row, 2, new QTableWidgetItem());
            ui->tableDay->setItem(row, 3, new QTableWidgetItem());
            ui->tableDay->setItem(row, 4, new QTableWidgetItem());
            ui->tableDay->blockSignals(false);
        } else
            QMessageBox::warning(this, "错误", "请先选择年份！");
    });
    connect(ui->tableDay, &QTableWidget::itemChanged, this, [this](QTableWidgetItem *item){
        const QString year = ui->comboBox->currentText().trimmed();
        const int row = item->row();
        QString title = ui->tableDay->item(row, 0)->text().trimmed();
        QString day = ui->tableDay->item(row, 1)->text().trimmed();
        QString start = ui->tableDay->item(row, 2)->text().trimmed();
        QString end = ui->tableDay->item(row, 3)->text().trimmed();
        QString works = ui->tableDay->item(row, 4)->text().trimmed();
        if(!day.isEmpty()) {
            if(start.isEmpty()) start = day;
            if(end.isEmpty()) end = day;

            if(start <= day && day <= end) {
                m_conf->beginWriteArray(year, ui->tableDay->rowCount());
                m_conf->setArrayIndex(row);
                m_conf->setValue("title", title);
                m_conf->setValue("day", day);
                m_conf->setValue("start", start);
                m_conf->setValue("end", end);
                m_conf->setValue("work", works);
                m_conf->endArray();
                m_conf->sync();
                emit holidayChanged(year);
            } else
                QMessageBox::warning(this, "错误", "节日日期必须在开始结束日期之间！");
        }
    });
    connect(ui->comboBox,  QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this](const int &index){
        ui->tableDay->blockSignals(true);
        ui->tableDay->clearContents();
        const QString year = ui->comboBox->currentText();
        const QList<Holiday> holidays = Holiday::getHolidays(year.toInt());
        ui->tableDay->setRowCount(holidays.size());
        for(int i=0; i < holidays.size(); i++)
        {
            Holiday holiday = holidays.at(i);
            if(holiday.m_start.isValid()) {
                QStringList works;
                for(auto day : holiday.m_workdays) works.append(day.toString("MM-dd"));

                ui->tableDay->setItem(i, 0, new QTableWidgetItem(holiday.m_title));
                ui->tableDay->setItem(i, 1, new QTableWidgetItem(holiday.m_day.toString("MM-dd")));
                ui->tableDay->setItem(i, 2, new QTableWidgetItem(holiday.m_start.toString("MM-dd")));
                ui->tableDay->setItem(i, 3, new QTableWidgetItem(holiday.m_end.toString("MM-dd")));
                ui->tableDay->setItem(i, 4, new QTableWidgetItem(works.join(", ")));
            }
        }
        ui->tableDay->blockSignals(false);
    });
    ui->comboBox->addItems(m_conf->childGroups());
    if(ui->comboBox->count() > 0)
        ui->comboBox->setCurrentIndex(ui->comboBox->count()-1);
}

SettingDialog::~SettingDialog() {
    delete ui;
}

void SettingDialog::updateCurrentChinaTime(QString format)
{
    if (format.isEmpty()) return;

    static const QDateTime current = QDateTime::fromString("2023-01-08 09:05:01", "yyyy-MM-dd HH:mm:ss");
    Lunar lunar;

    QString ret = QLocale::system().toString(current, format);
    if (ret.contains("GG"))
    {
        static QString ganzhi = lunar.solar2Ganzhi(current.date());
        ret.replace("GG", ganzhi);
    }
    if (ret.contains("SS"))
    {
        static QString hour = lunar.toDizhiHour(current.time().hour()) + "时";
        ret.replace("SS", hour);
    }

    ui->m_preview->setText(ret);
}