#include <QtWidgets>
#include <QDebug>

#include "arcclock.h"
#include "prefs.h"

ArcClock::ArcClock(QWidget *parent)
    : QWidget(parent, Qt::FramelessWindowHint
              | Qt::WindowStaysOnBottomHint)
{


    setAttribute(Qt::WA_NoSystemBackground);
    setAttribute(Qt::WA_TranslucentBackground);
    setAttribute(Qt::WA_TransparentForMouseEvents);
    setAttribute(Qt::WA_X11NetWmWindowTypeDock);

    this->readSettings(true);

    if ((posX > 0) || (posY > 0))
        this->move(posX, posY);

    QTimer *timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(update()));
    timer->start(1000);

    QAction *quitAction = new QAction(tr("E&xit"), this);
    quitAction->setShortcut(tr("Ctrl+Q"));
    connect(quitAction, SIGNAL(triggered()), this, SLOT(close()));

    QAction *configAction = new QAction(tr("&Preferences"), this);
    configAction->setShortcut(tr("Ctrl+P"));
    connect(configAction, SIGNAL(triggered()), this, SLOT(onConfig()));

    addAction(configAction);
    addAction(quitAction);

    setContextMenuPolicy(Qt::ActionsContextMenu);
    setToolTip(tr("Drag the clock with the left mouse button.\n"
                  "Use the right mouse button to open a context menu."));
    setWindowTitle(tr("Simple Clock"));
}

void ArcClock::closeEvent(QCloseEvent* event)
{
   this->writePosition();
   event->accept();
}

void ArcClock::writePosition()
{
    QSettings settings("Phobian", "Simple Arc Clock");
    settings.setValue("posX", (this->frameGeometry().x() < 0) ? 1 : this->frameGeometry().x());
    settings.setValue("posY", (this->frameGeometry().y() < 0) ? 1 : this->frameGeometry().y());
    qApp->exit();
}

ArcClock::~ArcClock()
{
}

void ArcClock::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        dragPosition = event->globalPos() - frameGeometry().topLeft();
        event->accept();
    }
}

void ArcClock::mouseMoveEvent(QMouseEvent *event)
{
    if (event->buttons() & Qt::LeftButton) {
        move(event->globalPos() - dragPosition);
        event->accept();
    }
}

void ArcClock::paintEvent(QPaintEvent *)
{
    int side = qMin(width(), height());
//    QTime time = QTime( 10, 52, 10);
    QTime time = QTime::currentTime();

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor(dialColor));

    QFont font(textFont, side/7);
    QFontMetrics fm(font);
    font.setBold(true);
    painter.setFont(font);
    QString timeText = time.toString(timeFormat);
    QRect rect(0, 0, side, side);
//    painter.drawRect(rect);
    painter.setPen(QColor(timeColor));
    painter.drawText(rect, Qt::AlignHCenter | Qt::AlignVCenter, timeText);


    if (showDate) {
        QRect rect2(0, side / 2 + fm.height() / 2 - 2, side, side / 2);
        QFont font2(textFont, side/18);
        painter.setFont(font2);
        painter.setPen(QColor(dateColor));
        painter.drawText(rect2, Qt::AlignHCenter, QDate::currentDate().toString("d MMM yy"));
        QRect rect3(0, 0, side, (side / 2) - (fm.height() / 2) + 2);
        painter.drawText(rect3, Qt::AlignHCenter | Qt::AlignBottom, QDate::currentDate().toString("dddd"));
    }

    qint64 twelve = (time.hour() > 12) ? time.hour() - 12 : time.hour();
    qint8 arcThickness = 6;
    qint8 hourArcOffset = 20;
    qint8 minuteArcOffset = 8;

    if (side > 380) {
        arcThickness = 14;
        hourArcOffset = 28;
        minuteArcOffset = 14;
    }
    else if (side > 300) {
        arcThickness = 12;
        hourArcOffset = 32;
        minuteArcOffset = 12;
    }

    qDebug() << arcThickness;
    qreal hourPosition = -30.0 * twelve - time.minute() / 2;
    qreal minutePosition = -6.0 * time.minute();

    QRect hourRect(hourArcOffset, hourArcOffset, side - 2 * hourArcOffset, side - 2 * hourArcOffset);
    QRect minuteRect(minuteArcOffset, minuteArcOffset, side - 2 * minuteArcOffset, side - 2 * minuteArcOffset);

    QPainter hourGroove(this);
    QPainterPath hourArcPathGroove;
    hourArcPathGroove.arcMoveTo(hourRect, 90.0);
    hourArcPathGroove.arcTo(hourRect, 90.0, 360.0 + hourPosition);
    hourGroove.setRenderHint(QPainter::Antialiasing);
    QPen hourArcPenGroove;
    hourArcPenGroove.setWidth(arcThickness);
    hourArcPenGroove.setCapStyle(Qt::FlatCap);
    QColor hourComplete(hourColor);
    int h, s, l, a;
//    hourComplete.setAlpha(40);
    hourComplete.getHsl(&h, &s, &l, &a);
    qDebug() << h << s << l << a;
    hourComplete.setHsl(h, s / 3, l / 2, a / 2);
    hourArcPenGroove.setColor(hourComplete);
    hourGroove.setPen(hourArcPenGroove);
    hourGroove.drawPath(hourArcPathGroove);

    QPainter hourArc(this);
    QPainterPath hourArcPath;
    hourArc.setRenderHint(QPainter::Antialiasing);
    hourArcPath.arcMoveTo(hourRect, 90.0);
    hourArcPath.arcTo(hourRect, 90.0, hourPosition);
    QPen hourArcPen;
    hourArcPen.setWidth(arcThickness);
    hourArcPen.setColor(QColor(hourColor));
    hourArcPen.setCapStyle(Qt::RoundCap);
    hourArc.setPen(hourArcPen);
    hourArc.drawPath(hourArcPath);

    QPainter painter3complete(this);

    QPainter painter3(this);
    QPainterPath minutePath;
    painter3.setRenderHint(QPainter::Antialiasing);
    minutePath.arcMoveTo(minuteRect, 90.5);
    minutePath.arcTo(minuteRect, 90.0, minutePosition);
    QPen minutePen;
    minutePen.setWidth(arcThickness);
    minutePen.setColor(QColor(minuteColor));
    minutePen.setCapStyle(Qt::RoundCap);
    painter3.setPen(minutePen);
    painter3.drawPath(minutePath);
}

void ArcClock::resizeEvent(QResizeEvent * /* event */)
{
}

QSize ArcClock::sizeHint() const
{
    return QSize(initWidth, initHeight);
}

void ArcClock::initVars()
{
    QSettings settings("Phobian", "Simple Arc Clock");

    settings.setValue("initWidth", 180);
    settings.setValue("initHeight", 180);
    settings.setValue("showDate", true);
    settings.setValue("hourColor", "#FFFFFFFF");
    settings.setValue("minuteColor", "#77dbdbdb");
    settings.setValue("timeColor", "#FFFFFFFF");
    settings.setValue("dateColor", "#aadbdbdb");
    settings.setValue("timeFormat", "h:mm");
    settings.setValue("textFont", "Sans");
    settings.setValue("posX", 0);
    settings.setValue("posY", 0);
    settings.setValue("Existant", true);
}

void ArcClock::readSettings(bool startup)
{
    QSettings settings("Phobian", "Simple Arc Clock");

    if (!settings.contains("Existant"))
        initVars();

    settings.sync();

    if (startup) {
        initWidth = settings.value("initWidth").toInt();
        initHeight = settings.value("initHeight").toInt();
        posX = 0;
        posY = 0;

        if (settings.value("posX").toInt() > -1)
            posX = settings.value("posX").toInt();

        if (settings.value("posY").toInt() > -1)
            posY = settings.value("posY").toInt();
    }

    showDate    = settings.value("showDate").toBool();
    hourColor   = settings.value("hourColor").toString();
    minuteColor = settings.value("minuteColor").toString();
    timeColor   = settings.value("timeColor").toString();
    dateColor   = settings.value("dateColor").toString();
    timeFormat  = settings.value("timeFormat").toString();
    textFont    = settings.value("textFont").toString();
}

void ArcClock::onConfig(void)
{
    Prefs *p = new Prefs(this);
    connect(p, SIGNAL(updateSettings()), this, SLOT(prefsChanged()));
    p->show();
}

void ArcClock::prefsChanged()
{
    readSettings(false);
}
