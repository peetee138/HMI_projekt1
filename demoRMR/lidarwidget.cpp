#include "lidarWidget.h"
#include <cmath>

LidarWidget::LidarWidget(QWidget *parent) : QWidget(parent) {
    setAttribute(Qt::WA_TranslucentBackground,true);
    setAutoFillBackground(false);
}

void LidarWidget::updateLidarData(const LaserMeasurement &data) {
    lidarData = data;
    hasData = true;
    update();
}

void LidarWidget::paintEvent(QPaintEvent *event) {
    Q_UNUSED(event);
    QPainter painter(this);

    QColor semiTransparentBlack(0,0,0,180);
    painter.fillRect(rect(), semiTransparentBlack);

    if (!hasData)
        return;

    painter.setRenderHint(QPainter::Antialiasing);

    // červený bod - robot
    QPen pen(Qt::red);
    pen.setWidth(3);
    painter.setPen(pen);
    QPoint center(width() / 2, height() / 2);
    //painter.drawEllipse(center, 20, 20);
    painter.drawEllipse(center, 14, 14);
    pen.setColor(Qt::gray);
    painter.setPen(pen);
    painter.drawEllipse(center, 17, 17);
    pen.setColor(Qt::white);
    painter.setPen(pen);
    painter.drawEllipse(center, 21, 21);
    painter.drawEllipse(center, 10, 10);
    painter.drawLine(center, QPoint(center.x(), center.y() - 15));

    // modre body - lidar merania
    pen.setColor(Qt::yellow);
    pen.setWidth(2);
    painter.setPen(pen);

    for (int k = 0; k < lidarData.numberOfScans; k++) {
        int dist = lidarData.Data[k].scanDistance / 20; // škálovanie
        double angle = (360.0 - lidarData.Data[k].scanAngle) * M_PI / 180.0;

        int xp = width() - (width() / 2 + dist * 2 * std::sin(angle));
        int yp = height() - (height() / 2 + dist * 2 * std::cos(angle));

        if (rect().contains(xp, yp)) {
            painter.drawEllipse(QPoint(xp, yp), 2, 2);
        }
    }
}
