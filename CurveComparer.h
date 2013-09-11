#ifndef CURVECOMPARER_H
#define CURVECOMPARER_H

#include <cmath>
#include <QDebug>

#include "Mannequin.h"

namespace CurveValidity
{
    enum Status
    {
        NotTested = 0,
        Valid = 1,
        Invalid = 2,
        NotEnoughDataLength = 3,
        NotEnoughDataPoints = 4,
        MannequinUnavailable = 5
    };
}

class CurveComparer
{
    private:
        QMap<QString, Mannequin*>   mannequins;
        Mannequin*                  currentMannequin;
        Curve*                      probeCurve;
        CurveValidity::Status       validity;

        float   segmentLength( Curve* curve, int startIndex, int endIndex );
        float   distanceBetween2Points( const Point& p1, const Point& p2 );
        Point   findEquivalentPoint( int provePointIndex );
        void    setIgnoredPoints();
        float   findMedianLength( Curve* curve, int startIndex, int endIndex );
        CurveValidity::Status isThereEnoughData( int firstValidPointIndex, int lastValidPointIndex );

        // shortcuts
        Point&  mecanicalPoint( int i );
        Point&  probePoint( int i );

    public:
        CurveComparer();

        CurveValidity::Status   isCurveValid( const QString& mannequinId, Curve* curve );
        void                    addMannequin( Mannequin* mannequin );

        // accessors
        CurveValidity::Status getValidity() const;
        Curve*      getProbeCurve() const;
        Mannequin*  getCurrentMannequin() const;
        Point       getMecanicalPoint( int i ) const;
        Point       getProbePoint( int i ) const;

};

#endif // CURVECOMPARER_H
