#ifndef CLOCK_H
#define CLOCK_H

#include <QWidget>

class FaceWidget;
class HandWidget;

class Clock : public QWidget{
    public:
        Clock(int s, bool enableGanzhi, bool enableLunar);
        void edit(bool editable);
        void enableGanzhi(bool enabled);
        void enableLunar(bool enabled);

    protected:
        void resizeEvent(QResizeEvent *event) override;
        void enterEvent(QEvent *event) override;
        void leaveEvent(QEvent *event) override;
        void mouseMoveEvent(QMouseEvent *event) override;

    private:
        FaceWidget *m_face;
        HandWidget *m_hand;
};

#endif