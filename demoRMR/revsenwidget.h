#ifndef REVSENWIDGET_H
#define REVSENWIDGET_H

#include <QWidget>
#include <QColor>

class RevSenWidget : public QWidget
{
    Q_OBJECT

public:
    explicit RevSenWidget(QWidget *parent = nullptr);
    // ZMENA: Nová funkcia s 8 úrovňami
    void updateSensorData(int front, int frontRight, int right, int backRight,
                          int back, int backLeft, int left, int frontLeft);

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    QColor getColorForLevel(int level) const;

    // 4 povodne urovne
    int frontLevel;
    int backLevel;
    int leftLevel;
    int rightLevel;

    // 4 diagonalne senzory
    int frontRightLevel;
    int backRightLevel;
    int backLeftLevel;
    int frontLeftLevel;
};

#endif // REVSENWIDGET_H
