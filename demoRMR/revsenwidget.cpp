#include "revsenwidget.h"
#include <QPainter>
#include <QColor>
#include <QRectF>
#include <QtMath>
#include <cmath>
#include <QPainterPath>

// Pre definíciu M_PI, ak chýba v cmath alebo QtMath
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// ==========================================================
// KONŠTRUKTOR
// ==========================================================
RevSenWidget::RevSenWidget(QWidget *parent) : QWidget(parent)
{
    setAttribute(Qt::WA_TranslucentBackground, true);
    setAutoFillBackground(false);

    // Inicializácia 4 pôvodných úrovní
    frontLevel = 0;
    backLevel = 0;
    leftLevel = 0;
    rightLevel = 0;

    // Inicializácia 4 nových diagonálnych úrovní
    frontRightLevel = 0;
    backRightLevel = 0;
    backLeftLevel = 0;
    frontLeftLevel = 0;
}

// ==========================================================
// AKTUALIZÁCIA DÁT (8 senzorov)
// ==========================================================
void RevSenWidget::updateSensorData(int front, int frontRight, int right, int backRight,
                                    int back, int backLeft, int left, int frontLeft)
{
    frontLevel = front;
    frontRightLevel = frontRight;
    rightLevel = right;
    backRightLevel = backRight;
    backLevel = back;
    backLeftLevel = backLeft;
    leftLevel = left;
    frontLeftLevel = frontLeft;

    update(); // Vynúti prekreslenie widgetu
}

// ==========================================================
// POMOCNÁ FUNKCIA: FARBA PODĽA ÚROVNE
// ==========================================================
QColor RevSenWidget::getColorForLevel(int level) const
{
    switch (level) {
    case 4: return QColor(255, 0, 0);       // Červená (najbližšie)
    case 3: return QColor(0, 0, 153);     // Oranžová
    case 2: return QColor(51, 51, 255);     // Žltá
    case 1: return QColor(51, 153, 255);       // Zelená (najďalej)
    default: return Qt::transparent;        // Ak nie je aktívna
    }
}

// ==========================================================
// VYKRESLOVANIE (PAINT EVENT)
// ==========================================================
void RevSenWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    QPoint center(width() / 2, height() / 2);
    int maxRadius = qMin(width(), height()) / 2;

    // Definovanie veľkosti prstencov
    int radiusStep = (maxRadius * 0.7) / 4;
    int centralRadius = maxRadius - (4 * radiusStep);
    if (centralRadius < 0) centralRadius = 0;

    // --- Definície pre 8 sektorov ---
    const int spanAngle = 45; // Rozsah 45 stupňov (360 / 8 = 45)

    // Začiatočné uhly pre oblúky (Qt: 0° je +X-os, stúpa proti smeru hodinových ručičiek)
    // Sú zoradené tak, aby vykresľovali 8 segmentov po 45 stupňoch, začínajúc pri 337.5° (Pravá/Dolná)
    double startAngle[8] = {
        -22.5,  // Sektor 0: Začína na -22.5 (-22.5° je to isté ako 337.5°)
        22.5,  // Sektor 1: Začína na 22.5°
        67.5,  // Sektor 2: Začína na 67.5°
        112.5,  // Sektor 3: Začína na 112.5°
        157.5,  // Sektor 4: Začína na 157.5°
        202.5,  // Sektor 5: Začína na 202.5°
        247.5,  // Sektor 6: Začína na 247.5°
        292.5   // Sektor 7: Začína na 292.5°
    };

    // Pole ukazovateľov na úrovne senzorov (v rovnakom poradí ako startAngle)
    int *levels[8] = {
        &frontRightLevel, // Sektor 0: 337.5 - 22.5 (P-P)
        &rightLevel,      // Sektor 1: 22.5 - 67.5 (P)
        &frontLevel,      // Sektor 2: 67.5 - 112.5 (P-L)
        &frontLeftLevel,  // Sektor 3: 112.5 - 157.5 (Ľ)
        &leftLevel,       // Sektor 4: 157.5 - 202.5 (Z-Ľ)
        &backLeftLevel,   // Sektor 5: 202.5 - 247.5 (Z)
        &backLevel,       // Sektor 6: 247.5 - 292.5 (Z-P)
        &backRightLevel   // Sektor 7: 292.5 - 337.5 (P-P)
    };

    QPen linePen(Qt::black);
    linePen.setWidth(2);

    // ==========================================================
    // KROK 1: Vykreslenie prstencov pre 8 sektorov
    // ==========================================================
    for (int sector = 0; sector < 8; ++sector) { // Použitá hodnota 8 priamo
        int start = startAngle[sector];
        int level = *levels[sector];

        // Prechádzame 4 úrovne
        for (int l = 1; l <= 4; ++l) {
            int currentOuterRadius = l * radiusStep + centralRadius;
            int currentInnerRadius = (l - 1) * radiusStep + centralRadius;

            if (currentInnerRadius < centralRadius) currentInnerRadius = centralRadius;

            QRectF outerRect(center.x() - currentOuterRadius,
                             center.y() - currentOuterRadius,
                             currentOuterRadius * 2,
                             currentOuterRadius * 2);
            QRectF innerRect(center.x() - currentInnerRadius,
                             center.y() - currentInnerRadius,
                             currentInnerRadius * 2,
                             currentInnerRadius * 2);

            QPainterPath path;

            // 1. Začína na vonkajšom radiálnom delení
            path.arcMoveTo(outerRect, start);

            // 2. Kreslí vonkajší oblúk (proti smeru hodinových ručičiek)
            path.arcTo(outerRect, start, spanAngle);

            // 3. Čiara smerom dovnútra (radiálne delenie)
            double angle_rad_end = (start + spanAngle) * M_PI / 180.0;
            QPointF p_inner_line_end(center.x() + currentInnerRadius * qCos(angle_rad_end),
                                     center.y() - currentInnerRadius * qSin(angle_rad_end));
            path.lineTo(p_inner_line_end);

            // 4. Kreslí vnútorný oblúk (späť - v smere hodinových ručičiek)
            path.arcTo(innerRect, start + spanAngle, -spanAngle);

            path.closeSubpath();

            // Vykreslenie
            if (l <= level) {
                painter.setBrush(getColorForLevel(l));
                painter.setPen(linePen);
                painter.drawPath(path);
            } else {
                painter.setBrush(Qt::NoBrush);
                painter.setPen(linePen);
                painter.drawPath(path);
            }
        }
    }

    // ==========================================================
    // KROK 2: Vykreslenie centrálneho modrého kruhu
    // ==========================================================
    painter.setBrush(Qt::black);
    painter.setPen(QPen(Qt::black, 2));
    painter.drawEllipse(center, centralRadius, centralRadius);


    // ==========================================================
    // KROK 3: Vykreslenie deliacich radiálnych čiar
    // ==========================================================
    QPen thickLinePen(Qt::black);
    thickLinePen.setWidth(4); // Hrubé čiary
    painter.setPen(thickLinePen);

    // Deliace čiary v uhloch 0, 45, 90, 135, 180, 225, 270, 315 (každých 45 stupňov)
    double lineAngles[8] = {
        22.5, 67.5, 112.5, 157.5, 202.5, 247.5, 292.5, 337.5
    };

    for (int i = 0; i < 8; ++i) { // Použitá hodnota 8 priamo
        double currentAngleRad = lineAngles[i] * M_PI / 180.0;

        int endX = center.x() + (maxRadius * qCos(currentAngleRad));
        int endY = center.y() - (maxRadius * qSin(currentAngleRad));

        int startX = center.x() + (centralRadius * qCos(currentAngleRad));
        int startY = center.y() - (centralRadius * qSin(currentAngleRad));

        painter.drawLine(QPoint(startX, startY), QPoint(endX, endY));
    }
}
