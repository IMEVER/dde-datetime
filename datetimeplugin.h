#ifndef DATETIMEPLUGIN_H
#define DATETIMEPLUGIN_H

#include <dde-dock/pluginsiteminterface.h>
// #include "../../interfaces/pluginsiteminterface.h"
#include "datetimewidget.h"
#include "../../widgets/tipswidget.h"

#include <QTimer>
#include <QLabel>

class WeekWidget;

class Holiday {
    public:
        enum DayType {
            Normal, Work, Rest
        };

        Holiday(QDate start, QDate end, QList<QDate> workdays = QList<QDate>())
        {
            this->m_start = start;
            this->m_end = end;
            this->m_workdays = workdays;
        }

        Holiday(QDate start) { Holiday(start, start); }

        DayType getDayType(QDate date)
        {
            if(m_start <= date && date <= m_end)
                return Rest;
            else if(m_workdays.contains(date))
                return Work;
            else
                return Normal;
        }

        QString toString() const
        {
            return QString("%1,%2").arg(m_start.toString("yyyy-MM-dd")).arg(m_end.toString("yyyy-MM-dd"));
        }

    private:
        QDate m_start;
        QDate m_end;
        QList<QDate> m_workdays;
};
class DatetimePlugin : public QObject, PluginsItemInterface
{
    Q_OBJECT
    Q_INTERFACES(PluginsItemInterface)
    Q_PLUGIN_METADATA(IID "com.deepin.dock.PluginsItemInterface" FILE "datetime.json")

public:
    explicit DatetimePlugin(QObject *parent = 0);

    const QString pluginName() const override;
    const QString pluginDisplayName() const override;
    void init(PluginProxyInterface *proxyInter) override;

    void pluginStateSwitched() override;
    bool pluginIsAllowDisable() override { return true; }
    bool pluginIsDisable() override;
    PluginType type() override { return Fixed; }
    PluginSizePolicy pluginSizePolicy() const override { return Custom; };

    int itemSortKey(const QString &itemKey) Q_DECL_OVERRIDE;
    void setSortKey(const QString &itemKey, const int order) Q_DECL_OVERRIDE;

    QWidget *itemWidget(const QString &itemKey) override;
    QWidget *itemTipsWidget(const QString &itemKey) override;
    QWidget *itemPopupApplet(const QString &itemKey) override;

    const QString itemCommand(const QString &itemKey) override;
    const QString itemContextMenu(const QString &itemKey) override;

    void invokedMenuItem(const QString &itemKey, const QString &menuId, const bool checked) override;

    void pluginSettingsChanged() override;
    QList<Holiday> getHolidays(int year);

private slots:
    void updateCurrentTimeString();
    void refreshPluginItemsVisible();

private:
    void loadPlugin();

private:
    QPointer<DatetimeWidget> m_centralWidget;
    QPointer<TipsWidget> m_dateTipsLabel;
    QTimer *m_refershTimer;
    QStringList tips;
    WeekWidget *m_weekWidget;
    int hour;
    int minute;

    bool m_pluginLoaded;
    bool m_showSecond;
};

#endif // DATETIMEPLUGIN_H
