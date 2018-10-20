#ifndef SHAPEDCLOCK_H
#define SHAPEDCLOCK_H

#include <QWidget>
#include <QColor>

class ArcClock : public QWidget
{
    Q_OBJECT

public:
    ArcClock(QWidget *parent = nullptr);
    ~ArcClock() override;
    QSize sizeHint() const override;

protected:
    void mouseMoveEvent(QMouseEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void paintEvent(QPaintEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;
    void closeEvent(QCloseEvent *event) override;

private:
    QPoint dragPosition;
    int initWidth;
    int initHeight;
    void initVars(void);
    void readSettings(bool);
    void writePosition(void);
    bool showDate;
    bool showRings;
    QString hourColor;
    QString minuteColor;
    QString timeColor;
    QString dateColor;
    QString dialColor;
    QString textFont;
    QString timeFormat;
    int posX;
    int posY;

private slots:
    void onConfig(void);
    void prefsChanged(void);
};

#endif
