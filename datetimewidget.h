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

#ifndef DATETIMEWIDGET_H
#define DATETIMEWIDGET_H

#include <QWidget>
#include <QColor>

class DatetimeWidget : public QWidget
{
    Q_OBJECT

public:
    explicit DatetimeWidget(QWidget *parent = 0);

    bool is24HourFormat() const { return m_24HourFormat; }
    bool isShowSecond() const { return m_showSecond; }
    bool isShowDate() const { return m_showDate; }
    bool isShowLunar() const { return m_showLunar; }
    bool isShowWeek() const { return m_showWeek; }
    QSize sizeHint() const;
    QStringList dateString();

signals:
    void requestUpdateGeometry() const;

public slots:
    void set24HourFormat(const bool value);
    void setShowSecond(const bool value);
    void setShowDate(const bool value);
    void setShowLunar(const bool value);
    void setShowWeek(const bool value);

private:
    void paintEvent(QPaintEvent *e);
    QString currentChinaTime() const;

private:
    bool m_24HourFormat;
    bool m_showSecond;
    bool m_showDate;
    bool m_showLunar;
    bool m_showWeek;
    mutable QFont m_timeFont;
    mutable QFont m_dateFont;
    mutable int m_timeOffset;
};

#endif // DATETIMEWIDGET_H
