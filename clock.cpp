#include "clock.h"
#include "lunar.h"

#include <QResizeEvent>
#include <QPaintEvent>
#include <QPainter>
#include <QGuiApplication>
#include <QScreen>
#include <QTimer>
#include <QTime>
#include <QCursor>
#include <QWindow>
#include <QtMath>

class FaceWidget : public QWidget{
public:
    FaceWidget(QWidget *parent) : QWidget(parent) {
        // setAttribute(Qt::WA_TranslucentBackground);
    }

protected:
    void paintEvent(QPaintEvent *event) override {
        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing);

        int radius = rect().width() / 2 - 2;
        painter.save();
        painter.translate(rect().center());

        int i = 0;
        do {
            painter.fillRect(-1, -radius, 2, i % 5 == 0 ? 8 : 4, palette().windowText());
            painter.rotate(6);
        } while(++i < 60);

        i=0;
        radius -= 20;
        do {
            qreal rangle = (i-3) / 6. * M_PI;
            QPointF p = QPointF(radius * qCos(rangle), radius * qSin(rangle));
            painter.drawText(QRect(p.toPoint() - QPoint(7, 5), QSize(14, 10)), Qt::AlignCenter, QString::number(i==0 ? 12 : i));
        } while(++i < 12);
        painter.restore();

        painter.setPen(QPen(palette().brightText(), 2));
        painter.setBrush(Qt::NoBrush);
        painter.drawRoundedRect(rect().adjusted(1, 1, -1, -1), 100, 100, Qt::RelativeSize);
    }
};

class HandWidget  : public QWidget {
    Q_OBJECT
public:
    HandWidget(QWidget *parent) : QWidget(parent) {
        m_timer = new QTimer(this);
        m_timer->setSingleShot(false);
        m_timer->setInterval(1000);
        connect(m_timer, &QTimer::timeout, this, [this] { repaint(); });
    }

protected:
    void paintEvent(QPaintEvent *event) override {
        static const int hourMargin = 20;
        static const int hourWidth = 6;
        static const int minuteMargin = 10;
        static const int minuteWidth = 4;
        static const int secondMargin = 0;
        static const int secondWidth = 2;

        const int radius = rect().width() / 2 - 2 - 8;
        const QTime time = QTime::currentTime();
        const int hour = time.hour() % 12;
        const int minute = time.minute();
        const int second = time.second();

        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing);
        painter.translate(rect().center());

        qreal rangle = hour * 30 + minute/2.;
        painter.rotate(rangle);
        painter.fillRect(-hourWidth/2, -radius + hourMargin, hourWidth, radius - hourMargin + 6, QBrush(Qt::darkYellow));
        painter.rotate(-rangle);

        rangle = minute * 6 + second/10.;
        painter.rotate(rangle);
        painter.fillRect(-minuteWidth/2, -radius + minuteMargin, minuteWidth, radius - minuteMargin + 10, QBrush(Qt::darkCyan));
        painter.rotate(-rangle);

        rangle = second * 6;
        painter.rotate(rangle);
        painter.fillRect(-secondWidth/2, -radius + secondMargin, secondWidth, radius - secondMargin + 15, QBrush(Qt::darkRed));
        painter.rotate(-rangle);

        painter.setPen(Qt::NoPen);
        painter.setBrush(QBrush(Qt::yellow));
        painter.drawEllipse(QPoint(0, 0), 4, 4);

        if(m_enabledGanzhi || m_enabledLunar) {
            static Lunar lunar;
            painter.setPen(QPen(Qt::yellow, 2));
            painter.setBrush(Qt::NoBrush);

            if(m_enabledGanzhi)
                painter.drawText(-28, -radius + 55, QString("%1 %2时").arg(lunar.solar2Ganzhi(QDate::currentDate())).arg(lunar.toDizhiHour(time.hour())));

            if(m_enabledLunar) {
                auto d = lunar.solar2lunarDate(QDate::currentDate());
                painter.drawText(QRect(-radius+40-15, -10, 30, 20), Qt::AlignCenter, d.first);
                painter.drawText(QRect(radius-40-15, -10, 30, 20), Qt::AlignCenter, d.second);
            }
        }
    }

private:
    QTimer *m_timer;
    bool m_enabledGanzhi;
    bool m_enabledLunar;

    friend class Clock;
};

Clock::Clock(int s, bool enableGanzhi, bool enableLunar) : QWidget() {
    setWindowFlags(Qt::FramelessWindowHint | Qt::WindowTransparentForInput | Qt::WindowStaysOnBottomHint);
    setAttribute(Qt::WA_TranslucentBackground);
    setAttribute(Qt::WA_X11NetWmWindowTypeUtility);

    m_face = new FaceWidget(this);
    m_hand = new HandWidget(this);
    m_hand->m_enabledGanzhi = enableGanzhi;
    m_hand->m_enabledLunar = enableLunar;

    setFixedSize(s, s);
    move(QGuiApplication::primaryScreen()->availableGeometry().topRight() + QPoint(-s-20, 20));
    m_hand->m_timer->start();
}

void Clock::edit(bool editable) {
    if((windowFlags() & Qt::WindowTransparentForInput) == !editable) return;

    setWindowFlag(Qt::WindowTransparentForInput, !editable);
    show();
}

void Clock::enableGanzhi(bool enabled) {
    m_hand->m_enabledGanzhi = enabled;
}

void Clock::enableLunar(bool enabled) {
    m_hand->m_enabledLunar = enabled;
}

void Clock::resizeEvent(QResizeEvent *event) {
    m_face->resize(event->size());
    m_hand->resize(event->size());
}
void Clock::enterEvent(QEvent *event) {
    QWidget::enterEvent(event);
    if(windowFlags() & ~Qt::WindowTransparentForInput)
        QGuiApplication::setOverrideCursor(QCursor(Qt::DragMoveCursor));
}

void Clock::leaveEvent(QEvent *event) {
    QWidget::leaveEvent(event);
    if(windowFlags() & ~Qt::WindowTransparentForInput)
        QGuiApplication::restoreOverrideCursor();
}

void Clock::mouseMoveEvent(QMouseEvent *event) {
    QWidget::mouseMoveEvent(event);
    if (event->buttons().testFlag(Qt::LeftButton))
        if(QWindow *window=windowHandle()) window->startSystemMove();
}

#include "clock.moc"