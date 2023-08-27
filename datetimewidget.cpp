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
#include <QFontDatabase>
#include <QMouseEvent>

#define PLUGIN_STATE_KEY    "enable"

DWIDGET_USE_NAMESPACE

DatetimeWidget::DatetimeWidget(QWidget *parent) : QWidget(parent)
{
    // m_timeFont = TIME_FONT;
    QFont timeFont = DFontSizeManager::instance()->t5(); // QFont(QFontDatabase::applicationFontFamilies(QFontDatabase::addApplicationFont(":/resources/font/DigitalNumbers.ttf")).at(0));
    timeFont.setPixelSize(14);
    setFont(timeFont);
}

void DatetimeWidget::setFormat(QString format) {
    if(m_format != format) {
        m_format = format;
        setFixedSize(fontMetrics().boundingRect(QRect(0,0,0,0), Qt::AlignLeft, currentChinaTime()).size().width() + 10, 24);
        updateGeometry();
        update();
    }
}

void DatetimeWidget::paintEvent(QPaintEvent *e)
{
    Q_UNUSED(e);

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setPen(QPen(palette().brush(QPalette::BrightText), 1));

    painter.drawText(rect(), Qt::AlignCenter, currentChinaTime());
}

void DatetimeWidget::mouseReleaseEvent(QMouseEvent *e) {
    if(e->button() == Qt::LeftButton) {
        if(e->pos().x() > rect().center().x()) {
            QProcess::startDetached("dbus-send", {"--print-reply", "--dest=org.deepin.dde.Widgets1", "/org/deepin/dde/Widgets1", "org.deepin.dde.Widgets1.Toggle"});
            return;
        }
    }

    QWidget::mouseReleaseEvent(e);
}

QString DatetimeWidget::currentChinaTime() const
{
    const QDateTime current = QDateTime::currentDateTime();
    Lunar lunar;

    QString ret = QLocale::system().toString(current, m_format);
    if(ret.contains("GG"))
        ret.replace("GG", lunar.solar2Ganzhi(current.date()));
    if(ret.contains("SS")) ret.replace("SS", lunar.toDizhiHour(current.time().hour()) + "时");
    return ret;
}

QStringList DatetimeWidget::dateString()
{
    QDateTime current = QDateTime::currentDateTime();
    Lunar lunar;
    const QMap<QVariant, QVariant> &dd = lunar.solar2lunar(current.date().year(), current.date().month(), current.date().day(), current.time().hour());

    QStringList tips;
    tips.append(QString("干支：%1年 %2月 %3日 %4时").arg(dd.value("gzYear").toString(), dd.value("gzMonth").toString(), dd.value("gzDay").toString(), dd.value("gzHour").toString()));
    tips.append(QString("始皇：%1年 %2%3 %4 %5").arg(dd.value("lYear").toString(), dd.value("ImonthCn").toString(), dd.value("IdayCn").toString(), dd.value("animal").toString(), dd.value("Term").toString()));

    tips.append("阳历：" + current.date().toString(Qt::SystemLocaleLongDate));

    return tips;
}
