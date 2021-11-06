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

#include "datetimewidget.h"

#include "lunar.h"
#include "constants.h"

#include <QPainter>
#include <QDebug>
#include <QSvgRenderer>
#include <DFontSizeManager>

#define PLUGIN_STATE_KEY    "enable"
#define TIME_FONT DFontSizeManager::instance()->t5()

DWIDGET_USE_NAMESPACE

DatetimeWidget::DatetimeWidget(QWidget *parent)
    : QWidget(parent)
{
    setMinimumSize(PLUGIN_BACKGROUND_MIN_SIZE, PLUGIN_BACKGROUND_MIN_SIZE);
    m_timeFont = TIME_FONT;
    m_timeFont.setPixelSize(15);
}

void DatetimeWidget::setShowDate(const bool value)
{
    if (m_showDate == value) {
        return;
    }

    m_showDate = value;
    updateGeometry();
    update();
}

void DatetimeWidget::setShowLunar(const bool value)
{
    if (m_showLunar == value) {
        return;
    }

    m_showLunar = value;
    updateGeometry();
    update();
}

void DatetimeWidget::set24HourFormat(const bool value)
{
    if (m_24HourFormat == value) {
        return;
    }

    m_24HourFormat = value;
    updateGeometry();
    update();
}

void DatetimeWidget::setShowSecond(const bool value)
{
    if(m_showSecond != value)
    {
        m_showSecond = value;
        updateGeometry();
        update();
    }
}

void DatetimeWidget::setShowWeek(const bool value)
{
	if(m_showWeek != value)
	{
		m_showWeek = value;
		updateGeometry();
		update();
	}
}

QSize DatetimeWidget::sizeHint() const
{
    // QString str = currentChinaTime();
    // while (QFontMetrics(m_timeFont).boundingRect(str).size().height() > PLUGIN_BACKGROUND_MIN_SIZE - 2) {
        // m_timeFont.setPixelSize(m_timeFont.pixelSize() - 1);
    // }

    return QSize(QFontMetrics(m_timeFont).boundingRect(QRect(0,0,0,0), Qt::AlignLeft, currentChinaTime()).size().width() + 2, height());
}

void DatetimeWidget::paintEvent(QPaintEvent *e)
{
    Q_UNUSED(e);

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    painter.setFont(m_timeFont);
    painter.setPen(QPen(palette().brightText(), 1));

    painter.drawText(rect(), Qt::AlignVCenter | Qt::AlignLeft, currentChinaTime());
}

QString DatetimeWidget::currentChinaTime() const
{
    QStringList date;
    const QDateTime current = QDateTime::currentDateTime();
    Lunar lunar;

    if (m_showDate)
    {
        // date.append(QLocale::system().toString(current.date(), QLocale::ShortFormat));
        date.append(current.date().toString("yyyy-MM-dd"));
    }

    // date.append(QLocale::system().toString(current.time(), m_showSecond ? QLocale::LongFormat : QLocale::ShortFormat));
    date.append(current.time().toString(QString().append(m_24HourFormat ? "HH" : "AP hh").append(":mm").append(m_showSecond ? ":ss" : "")));

    if(m_showWeek)
    {
    	date.append(current.toString("ddd"));
    }

    if (m_showLunar)
    {
        date.append(QString("%1年  %2时").arg(lunar.toGanZhiBySolarYear(current.date().year())).arg(lunar.toDizhiHour(current.time().hour())));
    }

    return date.join(" ");
}

QStringList DatetimeWidget::dateString()
{
    QDateTime current = QDateTime::currentDateTime();
    Lunar lunar;
    QMap<QVariant, QVariant> dd = lunar.solar2lunar(current.date().year(), current.date().month(), current.date().day(), current.time().hour());

    QStringList tips;
    tips.append(QString("干支：%1年 %2月 %3日 %4时").arg(dd.value("gzYear").toString(), dd.value("gzMonth").toString(), dd.value("gzDay").toString(), dd.value("gzHour").toString()));
    tips.append(QString("始皇：%1年%2%3 %4 %5").arg(dd.value("lYear").toString(), dd.value("ImonthCn").toString(), dd.value("IdayCn").toString(), dd.value("animal").toString(), dd.value("Term").toString()));

    tips.append("阳历：" + current.date().toString(Qt::SystemLocaleLongDate));

    return tips;
}
