#ifndef DATETIMEPLUGIN_H
#define DATETIMEPLUGIN_H

#include <dde-dock/pluginsiteminterface.h>
// #include "../../interfaces/pluginsiteminterface.h"
#include "datetimewidget.h"
#include "../../widgets/tipswidget.h"

#include <QTimer>
#include <QLabel>

using namespace Dock;

class WeekWidget;
class Clock;
class Holiday {
    public:
        enum DayType {
            Normal, Work, Rest
        };

        static QList<Holiday> getHolidays(int year);

        Holiday(){}

        DayType getDayType(const QDate &date) const
        {
            if(m_start <= date && date <= m_end)
                return Rest;
            else if(m_workdays.contains(date))
                return Work;
            else
                return Normal;
        }

        bool isValid() const {
            return m_start.isValid();
        }

        QString m_title;
        QDate m_day;
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
    ~DatetimePlugin();

    const QString pluginName() const override;
    const QString pluginDisplayName() const override;
    void init(PluginProxyInterface *proxyInter) override;

    void pluginStateSwitched() override;
    bool pluginIsAllowDisable() override { return true; }
    bool pluginIsDisable() override;
    PluginType type() override { return Normal; }
    PluginSizePolicy pluginSizePolicy() const override { return Custom; };

    int itemSortKey(const QString &itemKey) Q_DECL_OVERRIDE;
    void setSortKey(const QString &itemKey, const int order) Q_DECL_OVERRIDE;

    QWidget *itemWidget(const QString &itemKey) override;
    QWidget *itemTipsWidget(const QString &itemKey) override;
    QWidget *itemPopupApplet(const QString &itemKey) override;

    const QString itemCommand(const QString &itemKey) override;
    const QString itemContextMenu(const QString &itemKey) override;

    void invokedMenuItem(const QString &itemKey, const QString &menuId, const bool checked) override;

    PluginFlags flags() const override { return PluginFlag::Type_System | PluginFlag::Attribute_CanInsert | PluginFlag::Attribute_CanSetting; }

private slots:
    void updateCurrentTimeString();

private:
    void loadPlugin();

private:
    QPointer<DatetimeWidget> m_centralWidget;
    QPointer<TipsWidget> m_dateTipsLabel;
    Clock *m_clock;
    QTimer *m_refershTimer;
    QStringList tips;
    WeekWidget *m_weekWidget;
    int hour;
    int minute;
    bool m_showSecond;
};

#endif // DATETIMEPLUGIN_H
