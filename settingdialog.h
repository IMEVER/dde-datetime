#ifndef SETTINGDIALOG_H
#define SETTINGDIALOG_H

#include <QDialog>
#include <QSettings>

namespace Ui {
    class SettingDialog;
}

class SettingDialog : public QDialog {
    Q_OBJECT
public:
    SettingDialog();
    ~SettingDialog();

public slots:
    void updateCurrentChinaTime(QString format);

signals:
    void formattingChanged(const QString &format);
    void holidayChanged(const QString &year);
    void clockChanged(bool enable);
    void clockEditeableChanged(bool enable);
    void clockSizeChanged(int size);
    void clockShowGanzhiChanged(bool enable);
    void clockShowLunarChanged(bool enable);

private:
    Ui::SettingDialog *ui;
    QSettings *m_conf;
};

#endif