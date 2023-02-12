#include "settingdialog.h"
#include "ui_settings.h"
#include "lunar.h"

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

    ui->tableDay->setHorizontalHeaderLabels({"开始日期", "结束日期", "调休日期"});
    ui->tableDay->setSelectionMode(QTableWidget::SingleSelection);
    ui->tableDay->setSelectionBehavior(QTableWidget::SelectRows);
    connect(ui->m_addYear, &QPushButton::clicked, this, [this](bool checked){
        QStringList years = {"2020", "2021", "2022", "2023", "2024", "2025", "2026", "2027", "2028", "2029", "2030", "2031", "2032"};
        for(auto year : m_conf->childGroups()) years.removeOne(year);
        bool ok;
        QString y = QInputDialog::getItem(this, "年份", "请选择年份", years, 0, false, &ok);
        if(ok and !y.isEmpty()) {
            ui->comboBox->addItem(y);
            ui->comboBox->setCurrentIndex(ui->comboBox->count()-1);
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
        const int row = ui->tableDay->rowCount();
        const int currentRow = ui->tableDay->currentRow();
        if(!year.isEmpty() && currentRow>=0 && QMessageBox::question(this, "警告", "确认要删除当前行？") == QMessageBox::Yes) {
            if(currentRow+1 < row) {
                QStringList moves;
                m_conf->beginReadArray(year);
                for(int i=currentRow+2;i<=row;i++) {
                    m_conf->setArrayIndex(i);
                    moves.append(m_conf->value("day").toString());
                }
                m_conf->endArray();
                m_conf->beginWriteArray(year, row);
                for(int i=currentRow+1;i<row;i++) {
                    m_conf->setArrayIndex(i);
                    m_conf->setValue("day", moves.takeFirst());
                }
                m_conf->endArray();
            }

            ui->tableDay->removeRow(currentRow);
            m_conf->beginWriteArray(year, row-1);
            m_conf->setArrayIndex(row);
            m_conf->remove("");
            m_conf->endArray();
            m_conf->sync();
            emit holidayChanged(year);
        }
    });
    connect(ui->m_addDay, &QPushButton::clicked, this, [this](bool checked){
        if(ui->comboBox->currentText().trimmed().isEmpty() == false) {
            ui->tableDay->blockSignals(true);
            const int row = ui->tableDay->rowCount();
            ui->tableDay->insertRow(row);
            ui->tableDay->setItem(row, 0, new QTableWidgetItem());
            ui->tableDay->setItem(row, 1, new QTableWidgetItem());
            ui->tableDay->setItem(row, 2, new QTableWidgetItem());
            ui->tableDay->blockSignals(false);
        } else
            QMessageBox::warning(this, "错误", "请先选择年份！");
    });
    connect(ui->tableDay, &QTableWidget::itemChanged, this, [this](QTableWidgetItem *item){
        const QString year = ui->comboBox->currentText().trimmed();
        const int row = item->row();
        QString start = ui->tableDay->item(row, 0)->text().trimmed();
        QString end = ui->tableDay->item(row, 1)->text().trimmed();
        const QString works = ui->tableDay->item(row, 2)->text().trimmed();
        if(!start.isEmpty() || !end.isEmpty()) {
            if(start.isEmpty()) start = end;
            else if(end.isEmpty()) end = start;

            if(start <= end) {
                m_conf->beginWriteArray(year, ui->tableDay->rowCount());
                m_conf->setArrayIndex(row);
                m_conf->setValue("day", QString("%1~%2|%3").arg(start).arg(end).arg(works.split(',', Qt::SkipEmptyParts).join('~')));
                m_conf->endArray();
                m_conf->sync();
                emit holidayChanged(year);
            } else
                QMessageBox::warning(this, "错误", "开始日期不能大于结束日期！");
        } else
            QMessageBox::warning(this, "错误", "开始、结束日期不能同时为空！");
    });
    connect(ui->comboBox,  QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this](const int &index){
        ui->tableDay->blockSignals(true);
        ui->tableDay->clearContents();
        const QString year = ui->comboBox->currentText();
        const int size = m_conf->beginReadArray(year);
        ui->tableDay->setRowCount(size);
        for(int i=0; i < size; i++)
        {
            m_conf->setArrayIndex(i);
            QStringList days = m_conf->value("day").toString().split('|', Qt::SkipEmptyParts);
            if(days.count() > 0) {
                QString start, end;
                QStringList works;

                QStringList rests = days.first().trimmed().split('~', Qt::SkipEmptyParts);
                start = rests.first().trimmed();
                end = rests.count() > 1 ? rests.last().trimmed() : start;

                if(days.count() > 1)
                for(auto day : days.last().trimmed().split('~', Qt::SkipEmptyParts))
                    works.append(day.trimmed());

                ui->tableDay->setItem(i, 0, new QTableWidgetItem(start));
                ui->tableDay->setItem(i, 1, new QTableWidgetItem(end));
                ui->tableDay->setItem(i, 2, new QTableWidgetItem(works.join(", ")));
            }
        }
        m_conf->endArray();
        ui->tableDay->blockSignals(false);
    });
    ui->comboBox->addItems(m_conf->childGroups());
    emit ui->comboBox->currentIndexChanged(0);
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