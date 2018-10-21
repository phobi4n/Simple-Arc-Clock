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
    /* Get various geometries to calculate ring and text sizes */
    int side = qMin(width(), height());
//    QTime time = QTime( 10, 52, 10);
    QTime time = QTime::currentTime();
    int arcThickness = side / 30;
    int minuteArcOffset = 8;
    int hourArcOffset = minuteArcOffset + arcThickness;

    /* Setup to draw time */
    QPainter timePainter(this);
    timePainter.setRenderHint(QPainter::Antialiasing);
    timePainter.setPen(Qt::NoPen);
    timePainter.setBrush(QColor(dialColor));
    QString timeText = time.toString(timeFormat);

    /* If drawing AM/PM, reduce font size to fit */
    qreal fontAmPmAdjust = 0.0;

    if (timeFormat.endsWith("p"))
        fontAmPmAdjust = 10.0;

    QFont font(textFont);
    font.setPointSizeF((side/5.0) - fontAmPmAdjust);

    /* If time width is greater than inside diameter of hour ring, reduce font size */
    QFontMetrics fm(font);
    int timewidth = fm.width(timeText);
    int diameter = side - (hourArcOffset * 2 + arcThickness * 2);

    if (timewidth > diameter - 4)
        font.setPointSizeF((side/5.0) - fontAmPmAdjust - 4.0);

//    font.setBold(true);
    timePainter.setFont(font);

    /* Create a rectangle the size of our widget and draw the time text
     * in the centre of  it */
    QRect rect(0, 0, side, side);
    timePainter.setPen(QColor(timeColor));
    timePainter.drawText(rect, Qt::AlignHCenter | Qt::AlignVCenter, timeText);


    if (showDate) {
        QRect rect2(0, side / 2 + fm.height() / 2, side, side / 2);
        QFont font2(textFont, side/18);
        timePainter.setFont(font2);
        timePainter.setPen(QColor(dateColor));
        timePainter.drawText(rect2, Qt::AlignHCenter, QDate::currentDate().toString("d MMM yy"));
        QRect rect3(0, 0, side, (side / 2) - (fm.height() / 2));
        timePainter.drawText(rect3, Qt::AlignHCenter | Qt::AlignBottom, QDate::currentDate().toString("dddd"));
    }

    /* Calculate arc angles from the current time */
    int twelve = (time.hour() > 12) ? time.hour() - 12 : time.hour();
    qreal hourPosition = -30.0 * twelve - time.minute() / 2;
    qreal minutePosition = -6.0 * time.minute();
    /* Hue, saturation, luminance and alpha to calculate ring groove colours */
    int h, s, l, a;
    /* Create the rectangles that hold our arcs */
    QRect hourRect(hourArcOffset, hourArcOffset, side - 2 * hourArcOffset, side - 2 * hourArcOffset);
    QRect minuteRect(minuteArcOffset, minuteArcOffset, side - 2 * minuteArcOffset, side - 2 * minuteArcOffset);

    /* If wanted, paint the hour groove */
    if (showRings) {
        QPainter hourGroove(this);
        QPainterPath hourGroovePath;
        hourGroovePath.arcMoveTo(hourRect, 90.0);
        hourGroovePath.arcTo(hourRect, 90.0, 360.0 + hourPosition);
        hourGroove.setRenderHint(QPainter::Antialiasing);
        QPen hourGroovePen;
        hourGroovePen.setWidth(arcThickness);
        hourGroovePen.setCapStyle(Qt::FlatCap);
        QColor hourGrooveColor(hourColor);
        hourGrooveColor.getHsl(&h, &s, &l, &a);
        hourGrooveColor.setHsl(h, s/2, l, a/2);
        hourGroovePen.setColor(hourGrooveColor);
        hourGroove.setPen(hourGroovePen);
        hourGroove.drawPath(hourGroovePath);
    }

    /* Paint the hour arc */
    QPainter hourArc(this);
    QPainterPath hourArcPath;
    hourArc.setRenderHint(QPainter::Antialiasing);
    hourArcPath.arcMoveTo(hourRect, 90.0);
    hourArcPath.arcTo(hourRect, 90.0, hourPosition);
    QPen hourPen;
    hourPen.setWidth(arcThickness);
    hourPen.setColor(QColor(hourColor));
    hourPen.setCapStyle(Qt::RoundCap);

    if (showRings)
        hourPen.setCapStyle(Qt::FlatCap);

    hourArc.setPen(hourPen);
    hourArc.drawPath(hourArcPath);

    /* If wanted, paint the minute groove */
    if (showRings) {
        QPainter minuteGroove(this);
        QPainterPath minuteGroovePath;
        minuteGroovePath.arcMoveTo(minuteRect, 90.0);
        minuteGroovePath.arcTo(minuteRect, 90.0, 360 + minutePosition);
        minuteGroove.setRenderHint(QPainter::Antialiasing);
        QPen minuteGroovePen;
        minuteGroovePen.setWidth(arcThickness);
        minuteGroovePen.setCapStyle(Qt::FlatCap);
        QColor minuteGrooveColor(minuteColor);
        minuteGrooveColor.getHsl(&h, &s, &l, &a);
        minuteGrooveColor.setHsl(h, s/2, l, a/2);
        minuteGroovePen.setColor(minuteGrooveColor);
        minuteGroove.setPen(minuteGroovePen);
        minuteGroove.drawPath(minuteGroovePath);
    }

    /* Paint the minute arc */
    QPainter minuteArc(this);
    QPainterPath minutePath;
    minuteArc.setRenderHint(QPainter::Antialiasing);
    minutePath.arcMoveTo(minuteRect, 90.5);
    minutePath.arcTo(minuteRect, 90.0, minutePosition);
    QPen minutePen;
    minutePen.setWidth(arcThickness);
    minutePen.setColor(QColor(minuteColor));
    minutePen.setCapStyle(Qt::RoundCap);

    if (showRings)
        minutePen.setCapStyle(Qt::FlatCap);

    minuteArc.setPen(minutePen);
    minuteArc.drawPath(minutePath);
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
    settings.setValue("rings", false);
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
    showRings   = settings.value("rings").toBool();
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
