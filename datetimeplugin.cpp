/*
 * Copyright (C) 2011 ~ 2018 Deepin Technology Co., Ltd.
 *
 * Author:     sbw <sbw@sbw.so>
 *
 * Maintainer: sbw <sbw@sbw.so>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "datetimeplugin.h"
#include "weekwidget.h"

#include <DDBusSender>
#include <QLabel>
#include <QDebug>
#include <QDBusConnectionInterface>

#include <unistd.h>

#define PLUGIN_STATE_KEY "enable"
#define TIME_FORMAT_KEY "Use24HourFormat"
#define SECOND_SHOW_KEY "showSecond"
#define DATE_SHOW_KEY "showDate"
#define WEEK_SHOW_KEY "showWeek"
#define LUNAR_SHOW_KEY "showLunar"

DatetimePlugin::DatetimePlugin(QObject *parent)
    : QObject(parent)
    , m_pluginLoaded(false)
{

}

const QString DatetimePlugin::pluginName() const
{
    return "custom_datetime";
}

const QString DatetimePlugin::pluginDisplayName() const
{
    return "定制时间";
}

void DatetimePlugin::init(PluginProxyInterface *proxyInter)
{
    m_proxyInter = proxyInter;

    if (pluginIsDisable()) {
        return;
    }

    loadPlugin();
}

void DatetimePlugin::loadPlugin()
{
    if (m_pluginLoaded)
        return;

    QDBusConnection::sessionBus().connect("com.deepin.daemon.Timedate", "/com/deepin/daemon/Timedate", "org.freedesktop.DBus.Properties",  "PropertiesChanged", this, SLOT(propertiesChanged()));
    m_settings = new QSettings(QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation) + "/dde-datetime.ini", QSettings::IniFormat);
    m_settings->setIniCodec(QTextCodec::codecForName("UTF-8"));

    m_pluginLoaded = true;
    m_dateTipsLabel = new TipsWidget;
    m_weekWidget = new WeekWidget(this);
    m_refershTimer = new QTimer(this);

    m_refershTimer->setInterval(1000);
    m_refershTimer->start();

    m_centralWidget = new DatetimeWidget();
    m_centralWidget->setShowDate(m_settings->value(DATE_SHOW_KEY, true).toBool());
    m_centralWidget->setShowLunar(m_settings->value(LUNAR_SHOW_KEY, true).toBool());
    m_centralWidget->setShowSecond(m_settings->value(SECOND_SHOW_KEY, false).toBool());
    m_centralWidget->setShowWeek(m_settings->value(WEEK_SHOW_KEY, false).toBool());

    connect(m_centralWidget, &DatetimeWidget::requestUpdateGeometry, [this] { m_proxyInter->itemUpdate(this, pluginName()); });
    connect(m_refershTimer, &QTimer::timeout, this, &DatetimePlugin::updateCurrentTimeString);

    m_proxyInter->itemAdded(this, pluginName());

    pluginSettingsChanged();
}

void DatetimePlugin::pluginStateSwitched()
{
    m_proxyInter->saveValue(this, PLUGIN_STATE_KEY, pluginIsDisable());

    refreshPluginItemsVisible();
}

bool DatetimePlugin::pluginIsDisable()
{
    return !(m_proxyInter->getValue(this, PLUGIN_STATE_KEY, true).toBool());
}

int DatetimePlugin::itemSortKey(const QString &itemKey)
{
    Q_UNUSED(itemKey);
    return m_proxyInter->getValue(this, "pos", 5).toInt();
}

void DatetimePlugin::setSortKey(const QString &itemKey, const int order)
{
    Q_UNUSED(itemKey);
    m_proxyInter->saveValue(this, "pos", order);
}

QWidget *DatetimePlugin::itemWidget(const QString &itemKey)
{
    Q_UNUSED(itemKey);

    return m_centralWidget;
}

QWidget *DatetimePlugin::itemTipsWidget(const QString &itemKey)
{
    Q_UNUSED(itemKey);

    return m_dateTipsLabel;
}

QWidget *DatetimePlugin::itemPopupApplet(const QString &itemKey)
{
    Q_UNUSED(itemKey);
    if(!m_weekWidget->isVisible())
        m_weekWidget->updateTime();
    return m_weekWidget;
}

const QString DatetimePlugin::itemCommand(const QString &itemKey)
{
    Q_UNUSED(itemKey);

    return nullptr; //"dbus-send --print-reply --dest=com.deepin.Calendar /com/deepin/Calendar com.deepin.Calendar.RaiseWindow";
}

const QString DatetimePlugin::itemContextMenu(const QString &itemKey)
{
    Q_UNUSED(itemKey);

    QList<QVariant> items;
    items.reserve(1);

    QMap<QString, QVariant> settings;
    settings["itemId"] = "settings";
    if (m_centralWidget->is24HourFormat())
        settings["itemText"] = "12小时制";
    else
        settings["itemText"] = "24小时制";
    settings["isActive"] = true;
    items.push_back(settings);

    QMap<QString, QVariant> showSecond;
    showSecond["itemId"] = "showSecond";
    showSecond["itemText"] = m_centralWidget->isShowSecond() ? "隐藏秒" : "显示秒";
    showSecond["isActive"] = true;
    items.push_front(showSecond);

    QMap<QString, QVariant> showDate;
    showDate["itemId"] = "showDate";
    showDate["itemText"] = m_centralWidget->isShowDate() ? "隐藏日期" : "显示日期";
    showDate["isActive"] = true;
    items.push_back(showDate);

    QMap<QString, QVariant> showWeek;
    showWeek["itemId"] = "showWeek";
    showWeek["itemText"] = m_centralWidget->isShowWeek() ? "隐藏星期" : "显示星期";
    showWeek["isActive"] = true;
    items.push_back(showWeek);

    QMap<QString, QVariant> showLunar;
    showLunar["itemId"] = "showLunar";
    showLunar["itemText"] = m_centralWidget->isShowLunar() ? "隐藏干支" : "显示干支";
    showLunar["isActive"] = true;
    items.push_back(showLunar);

    QMap<QString, QVariant> open;
    open["itemId"] = "open";
    open["itemText"] = "时间设置";
    open["isActive"] = true;
    items.push_back(open);

    QMap<QString, QVariant> menu;
    menu["items"] = items;
    menu["checkableMenu"] = false;
    menu["singleCheck"] = false;

    return QJsonDocument::fromVariant(menu).toJson();
}

void DatetimePlugin::invokedMenuItem(const QString &itemKey, const QString &menuId, const bool checked)
{
    Q_UNUSED(itemKey)
    Q_UNUSED(checked)

    if (menuId == "open")
    {
        DDBusSender()
        .service("com.deepin.dde.ControlCenter")
        .interface("com.deepin.dde.ControlCenter")
        .path("/com/deepin/dde/ControlCenter")
        .method(QString("ShowModule"))
        .arg(QString("datetime"))
        .call();
    }
    else if (menuId == "showDate")
    {
        m_centralWidget->setShowDate(!m_centralWidget->isShowDate());
        m_settings->setValue(DATE_SHOW_KEY, m_centralWidget->isShowDate());
    }
    else if (menuId == "showWeek")
    {
	    m_centralWidget->setShowWeek(!m_centralWidget->isShowWeek());
	    m_settings->setValue(WEEK_SHOW_KEY, m_centralWidget->isShowWeek());
    }
    else if (menuId == "showLunar")
    {
        m_centralWidget->setShowLunar(!m_centralWidget->isShowLunar());
        m_settings->setValue(LUNAR_SHOW_KEY, m_centralWidget->isShowLunar());
    }
    else if(menuId == "showSecond")
    {
        m_centralWidget->setShowSecond(!m_centralWidget->isShowSecond());
        m_settings->setValue(SECOND_SHOW_KEY, m_centralWidget->isShowSecond());
    }
    else
    {
        const bool value = is24HourFormat();
        save24HourFormat(!value);
        m_centralWidget->set24HourFormat(!value);
    }
}

void DatetimePlugin::pluginSettingsChanged()
{
    if (!m_pluginLoaded)
        return;

    const bool value = is24HourFormat();

    m_centralWidget->set24HourFormat(value);

    refreshPluginItemsVisible();
}

void DatetimePlugin::updateCurrentTimeString()
{
    const QDateTime currentDateTime = QDateTime::currentDateTime();

    if(tips.count() == 0 || m_dateTipsLabel->isVisible())
    {
	    int h = currentDateTime.time().hour();

	    if (tips.count() == 0 || h != hour)
	    {
		    hour = h;
		    tips = m_centralWidget->dateString();
	    }

	    QStringList t;
	    foreach(QString s, tips)
		    t.append(s);

        // t.append("时间：" + QLocale::system().toString(currentDateTime.time(), QLocale::LongFormat));
        t.append("时间：" + currentDateTime.time().toString("HH:mm:ss"));

	    m_dateTipsLabel->setTextList(t);
    }
    const int min = currentDateTime.time().minute();

    if (min == minute && !m_centralWidget->isShowSecond())
        return;

    minute = min;
    m_centralWidget->update();
}

void DatetimePlugin::refreshPluginItemsVisible()
{
    if (!pluginIsDisable()) {

        if (!m_pluginLoaded) {
            loadPlugin();
            return;
        }
        m_proxyInter->itemAdded(this, pluginName());
    } else {
        m_proxyInter->itemRemoved(this, pluginName());
        m_pluginLoaded = false;
        QDBusConnection::sessionBus().disconnect("com.deepin.daemon.Timedate", "/com/deepin/daemon/Timedate", "org.freedesktop.DBus.Properties",  "PropertiesChanged", this, SLOT(propertiesChanged()));
        m_settings->deleteLater();
        m_refershTimer->deleteLater();
        m_dateTipsLabel->deleteLater();
        m_weekWidget->deleteLater();
        m_centralWidget->deleteLater();
    }
}

void DatetimePlugin::propertiesChanged()
{
    pluginSettingsChanged();
}

bool DatetimePlugin::is24HourFormat()
{
    QDBusInterface *m_interface;
        if (QDBusConnection::sessionBus().interface()->isServiceRegistered("com.deepin.daemon.Timedate")) {
            m_interface = new QDBusInterface("com.deepin.daemon.Timedate", "/com/deepin/daemon/Timedate", "com.deepin.daemon.Timedate");
        } else {
            QString path = QString("/com/deepin/daemon/Accounts/User%1").arg(QString::number(getuid()));
            m_interface = new QDBusInterface("com.deepin.daemon.Accounts", path, "com.deepin.daemon.Accounts.User",
                                      QDBusConnection::systemBus(), this);
        }

    bool format = m_interface->property(TIME_FORMAT_KEY).toBool();

    m_interface->deleteLater();
    return format;
}

void DatetimePlugin::save24HourFormat(bool format)
{
    if (QDBusConnection::sessionBus().interface()->isServiceRegistered("com.deepin.daemon.Timedate")) {
        QDBusInterface *m_interface = new QDBusInterface("com.deepin.daemon.Timedate", "/com/deepin/daemon/Timedate", "com.deepin.daemon.Timedate");
        m_interface->setProperty(TIME_FORMAT_KEY, format);
        m_interface->deleteLater();
    }
}

QList<Holiday> DatetimePlugin::getHolidays(int year)
{
    const QString key = QString::number(year);
    QList<Holiday> list;
    // if(allKeys.contains(key))
    {
        const int size = m_settings->beginReadArray(key);
        for(int i=0; i < size; i++)
        {
            m_settings->setArrayIndex(i);

            QDate start, end;
            QList<QDate> works;

            QStringList workDays = m_settings->value("work").toStringList();
            for(auto day : workDays)
                works.append(QDate::fromString(QString("%1-%2").arg(year).arg(day), "yyyy-MM-dd"));

            start = QDate::fromString(QString("%1-%2").arg(year).arg(m_settings->value("start").toString()), "yyyy-MM-dd");

            end = QDate::fromString(QString("%1-%2").arg(year).arg(m_settings->value("end").toString()), "yyyy-MM-dd");

            list.append(Holiday(start, end, works));
        }
        m_settings->endArray();
    }
    return list;
}
