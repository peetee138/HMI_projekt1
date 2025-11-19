#ifndef LIDARWIDGET_H
#define LIDARWIDGET_H

//#endif // LIDARWIDGET_H
#include <QWidget>
#include <QPainter>
#include "robot.h"  //definovaný LaserMeasurement

class LidarWidget : public QWidget
{
    Q_OBJECT

public:
    explicit LidarWidget(QWidget *parent = nullptr);
    void updateLidarData(const LaserMeasurement &data);

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    LaserMeasurement lidarData;
    bool hasData = false;
};

#endif // LIDARWIDGET_H
