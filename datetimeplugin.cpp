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
#include "settingdialog.h"

#include <QLabel>
#include <QDebug>
#include <unistd.h>
#include <QDesktopServices>
#include <DDBusSender>

#define PLUGIN_STATE_KEY "enable"

static const QString confFile = QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation) + "/dde-datetime.ini";

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

    QSettings m_settings(confFile, QSettings::IniFormat);
    m_settings.setIniCodec(QTextCodec::codecForName("UTF-8"));

    m_pluginLoaded = true;
    m_dateTipsLabel = new TipsWidget;
    m_weekWidget = new WeekWidget(this);
    m_refershTimer = new QTimer(this);

    m_refershTimer->setInterval(1000);
    m_refershTimer->start();

    m_centralWidget = new DatetimeWidget();
    m_centralWidget->setFormat(m_settings.value("format", "yyyy/MM/dd HH:mm").toString());
    m_showSecond = m_settings.value("format", "yyyy/MM/dd HH:mm").toString().contains('s', Qt::CaseSensitive);

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
    items.reserve(2);

    QMap<QString, QVariant> refresh;
    refresh["itemId"] = "refresh";
    refresh["itemText"] = "刷新";
    refresh["isActive"] = true;
    items.append(refresh);

    QMap<QString, QVariant> edit;
    edit["itemId"] = "edit";
    edit["itemText"] = "编辑配置文件";
    edit["isActive"] = true;
    items.append(edit);

    QMap<QString, QVariant> openCalendar;
    openCalendar["itemId"] = "openCalendar";
    openCalendar["itemText"] = "打开日历";
    openCalendar["isActive"] = true;
    items.append(openCalendar);

    QMap<QString, QVariant> open;
    open["itemId"] = "open";
    open["itemText"] = "时间设置";
    open["isActive"] = true;
    items.append(open);

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
        // DDBusSender()
        // .service("com.deepin.dde.ControlCenter")
        // .interface("com.deepin.dde.ControlCenter")
        // .path("/com/deepin/dde/ControlCenter")
        // .method(QString("ShowModule"))
        // .arg(QString("datetime"))
        // .call();

        SettingDialog *dialog = new SettingDialog();
        dialog->setModal(false);
        dialog->setAttribute(Qt::WA_DeleteOnClose);

        connect(dialog, &SettingDialog::formattingChanged, this, [this](const QString &format){
            m_centralWidget->setFormat(format);
            m_showSecond = format.contains('s', Qt::CaseSensitive);
        });
        connect(dialog, &SettingDialog::holidayChanged, this, [this](const QString &year){
            m_weekWidget->refresh();
        });

        dialog->show();
    } else if(menuId == "openCalendar") {
        DDBusSender()
        .service("com.deepin.Calendar")
        .interface("com.deepin.Calendar")
        .path("/com/deepin/Calendar")
        .method(QString("RaiseWindow"))
        .call();
    }
    else if (menuId == "refresh") {
        QSettings m_settings(confFile, QSettings::IniFormat);
        m_settings.setIniCodec(QTextCodec::codecForName("UTF-8"));
        QString format = m_settings.value("format", "yyyy/MM/dd HH:mm").toString();
        m_centralWidget->setFormat(format);
        m_weekWidget->refresh();
        m_showSecond = format.contains('s', Qt::CaseSensitive);
    } else if(menuId == "edit") {
        QDesktopServices::openUrl(QUrl::fromLocalFile(confFile));
    }
}

void DatetimePlugin::pluginSettingsChanged()
{
    if (m_pluginLoaded)
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

    if (min == minute && !m_showSecond)
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
        m_refershTimer->deleteLater();
        m_dateTipsLabel->deleteLater();
        m_weekWidget->deleteLater();
        m_centralWidget->deleteLater();
    }
}

QList<Holiday> DatetimePlugin::getHolidays(int year)
{
    QSettings m_settings(confFile, QSettings::IniFormat);
    m_settings.setIniCodec(QTextCodec::codecForName("UTF-8"));

    const QString key = QString::number(year);
    QList<Holiday> list;

    const int size = m_settings.beginReadArray(key);
    for(int i=0; i < size; i++)
    {
        m_settings.setArrayIndex(i);
        QStringList days = m_settings.value("day").toString().split('|', Qt::SkipEmptyParts);
        if(days.count() > 0) {
            QDate start, end;
            QList<QDate> works;

            QStringList rests = days.first().trimmed().split('~', Qt::SkipEmptyParts);
            start = QDate::fromString(QString("%1-%2").arg(year).arg(rests.first().trimmed()), "yyyy-MM-dd");
            end = rests.count() > 1 ? QDate::fromString(QString("%1-%2").arg(year).arg(rests.last().trimmed()), "yyyy-MM-dd") : start;

            if(days.count() > 1)
            for(auto day : days.last().trimmed().split('~', Qt::SkipEmptyParts))
                works.append(QDate::fromString(QString("%1-%2").arg(year).arg(day.trimmed()), "yyyy-MM-dd"));

            list.append(Holiday(start, end, works));
        }
    }
    m_settings.endArray();

    return list;
}
