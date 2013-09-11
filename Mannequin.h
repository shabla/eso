#ifndef MANNEQUIN_H
#define MANNEQUIN_H

#include <QtXml>
#include <QString>
#include <QList>

#include "Point.h"

typedef QList<Point> Curve;

class Mannequin : public Curve
{
    private:
        QString name;               // name of the mannequin (ex. BOB001, BOB002, ARN001, CAT001, ...)
        float radius;               // a probe point is valid if it is within this radius of it's equivalent mecanical point
        Point endOfStomach;         // point when the probe is at the end of the stomach
        float curveLengthThreshold; // threshold of validity for the probe curve compared to the mecanical curve (ex. 0.1 = 10%)
        float maxY;                 // max value of y after which probe points won't be tested anymore (low y is closer to the stomach)
        float maxIntervalMedian;    // max value that the median of the distance between each probe points can be for the curve to be valid

        void loadSettings();

    public:
        Mannequin( const QString& filename );

        void loadMannequin( const QString& filename );

        float getMaxIntervalMedian() const;
        QString getName() const;
        void setRadius( float value );
        float getRadius() const;
        Point getEndOfStomach() const;
        float getCurveLengthThreshold() const;
        float getMaxY() const;
};

#endif // MANNEQUIN_H
