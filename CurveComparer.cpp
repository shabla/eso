#include "CurveComparer.h"

CurveComparer::CurveComparer()
{
    validity = CurveValidity::NotTested;
    currentMannequin = 0;
    probeCurve = 0;
}

CurveValidity::Status CurveComparer::getValidity() const
{
    return validity;
}

// Add an additionnal mannequin in the CurveComparer (ex. BOB001, BOB002, ARN001, CAT001, ...)
void CurveComparer::addMannequin( Mannequin* mannequin )
{
    mannequins.insert( mannequin->getName(), mannequin );
}

CurveValidity::Status CurveComparer::isCurveValid( const QString& mannequinId, Curve* curve )
{
    validity = CurveValidity::NotTested;
    currentMannequin = mannequins.value( mannequinId );
    probeCurve = curve;

    // if the mannequin id received doesn't exist, return false, else test the curve
    if( !mannequins.contains( mannequinId ) )
    {
        currentMannequin = 0;
        probeCurve = 0;
        return CurveValidity::MannequinUnavailable;
    }
    else
    {
        currentMannequin = mannequins.value( mannequinId );

        // PROBLEM: If the probe go up and down the eso while recording the points, these additional points should be valid, but
        // this will make the valid segment length go up and the length validity result will be wrong.

        // SOLUTION: SORT the non-ignored points (those that need to be tested) by y so that the additional points will give more
        // data, making the curve more accurate. Could be implemented by modifying the setIgnoredPoints() method.

        int validPointsCount = 0;
        int invalidPointsCount = 0;
        int ignoredPointsCount = 0;

        int firstValidPointIndex = -1;
        int lastValidPointIndex = -1;

        setIgnoredPoints();

        // for each points in the probeCurve
        for( int i=0; i<probeCurve->size(); i++ )
        {
            qDebug() << "Current point : probeCurve[" << i << "]";
            qDebug() << "Probe point: (" << probePoint(i).x << ", " << probePoint(i).y << ", " << probePoint(i).z << ")";

            // if the point is IGNORED don't test it
            if( probePoint(i).validity == PointValidity::Ignored )
            {
                ignoredPointsCount++;
                qDebug() << "This point is ignored.\n";
            }
            else
            {
                // find the probePoint equivalent in mecanicaCurve where probePoint.y = mecanicalPoint.y
                // this will always return a point because all the points above the maxY and the points below the first mecanicalPoint.y are set to ignored in defineIgnoredPoints()
                Point mecanicalPoint = findEquivalentPoint( i );
                qDebug() << "Mecanical point: (" << mecanicalPoint.x << ", " << mecanicalPoint.y << ", " << mecanicalPoint.z << ")";
                qDebug() << "Radius: " << currentMannequin->getRadius();

                // determine if the probePoint is within the mecanicalPoint's radius
                float dist = distanceBetween2Points( mecanicalPoint, probePoint(i) );
                qDebug() << "Distance: " << dist;

                // the point is VALID
                if( dist <= currentMannequin->getRadius() )
                {
                    probePoint(i).validity = PointValidity::Valid;

                    // keep the first and last valid point in order to calculate the length of the valid segment at the end
                    // make sure not to set endOfStomach as the first point (all the points between endOfStomach and the first mecanicalPoint will always be ignored)
                    if( firstValidPointIndex == -1 && mecanicalPoint != currentMannequin->getEndOfStomach() )
                        firstValidPointIndex = i;
                    else
                        lastValidPointIndex = i;

                    validPointsCount++;

                    qDebug() << "This point is valid.\n";
                }
                // the point is INVALID
                else
                {
                    probePoint(i).validity = PointValidity::Invalid;
                    invalidPointsCount++;
                    validity = CurveValidity::Invalid;

                    qDebug() << "This point is invalid.\n";
                }
            }
        }

        qDebug() << "====================================================";
        qDebug() << "SUMMARY";
        qDebug() << "====================================================";
        qDebug() << "Number of curvePoints:" << probeCurve->size();
        qDebug() << "Valid points:" << validPointsCount;
        qDebug() << "Invalid points:" << invalidPointsCount;
        qDebug() << "Ignored points:" << ignoredPointsCount;
        qDebug() << "First valid point:" << firstValidPointIndex;
        qDebug() << "Last valid point:" << lastValidPointIndex;

        // Check for curve validity
        if( validity == CurveValidity::NotTested )
            validity = isThereEnoughData( firstValidPointIndex, lastValidPointIndex );
        else
            validity = CurveValidity::Invalid;

        return validity;
    }
}

float CurveComparer::findMedianLength( Curve* curve, int startIndex, int endIndex )
{
    // make sure there's at least 2 element (to test at least one segment without crashing)
    if( endIndex - startIndex + 1 >= 2 )
    {
        // a map is used because it is automatically sorted by key
        // here the key will be the probeCurve.y and the value its index (not used)
        QMap<float, int> values;

        // set the first min and max with the deltaY of the 2 first points
        float min = distanceBetween2Points( curve->at(startIndex), curve->at(startIndex+1) );
        float max = min;
        float avg = min;
        float med;

        values.insertMulti( min, 0 );

        // start at the second point since we already did the first
        for( int i=startIndex+1; i<=endIndex; ++i )
        {
            float dist = distanceBetween2Points( curve->at(i), curve->at(i+1) );

            values.insertMulti( dist, i );

            if( dist < min )
                min = dist;
            if( dist > max )
                max = dist;

            avg += dist;
        }

        avg /= values.size();

        int index = values.keys().size() / 2;
        if( values.keys().size() % 2 != 0 )
            med = ( values.keys()[index] + values.keys()[index-1] ) / 2;
        else
            med = values.keys()[index];

        qDebug() << "Minimum:" << min;
        qDebug() << "Maximum:" << max;
        qDebug() << "Average:" << avg;
        qDebug() << "Median:" << med;
        qDebug() << "Max median:" << currentMannequin->getMaxIntervalMedian() << "\n";

        return med;
    }

    return -1;  // median of lengths can't be negative, so this means that there is not enough data
}

CurveValidity::Status CurveComparer::isThereEnoughData( int firstValidPointIndex, int lastValidPointIndex )
{
    qDebug() << "\nDistance between 2 valid points:";
    float probeMedian = findMedianLength( probeCurve, firstValidPointIndex, lastValidPointIndex );

    // Test the median of the length between the points of probeCurve
    // high median = bigger space between points = bad
    // -1 means that there is not enough data to calculate the median
    // mecanical median should always be smaller since we expect the mecanical curve to have more points in the same range of y as the probe curve
    // The test to do (replace this): probeMedian should be smaller than a fixed value (determined in the settings file, by mannequin)
    if( probeMedian > currentMannequin->getMaxIntervalMedian() || probeMedian == -1 )
        return CurveValidity::NotEnoughDataPoints;

    // calculate the length of the mecanicalCurve and the length of the valid part of the probeCurve
    float mecanicalLength = segmentLength( currentMannequin, 0, currentMannequin->size() );
    float probeValidSegmentLength = segmentLength( probeCurve, firstValidPointIndex, lastValidPointIndex );

    qDebug() << "Mecanical curve length:" << mecanicalLength;
    qDebug() << "Probe valid segment length:" << probeValidSegmentLength;
    qDebug() << "Minimum valid length:" << (1.0f - currentMannequin->getCurveLengthThreshold()) * mecanicalLength << "\n";

    // Test the length of the valid segment
    // if the valid length is bigger than a certain threshold of mecanicalLength's length, it's invalid
    if( fabs( mecanicalLength - probeValidSegmentLength ) > currentMannequin->getCurveLengthThreshold() * mecanicalLength )
        return CurveValidity::NotEnoughDataLength;

    return CurveValidity::Valid;
}

Point CurveComparer::findEquivalentPoint( int probePointIndex )
{
    // it's the first point in the list, compare it to the endOfStomach point
    if( probePointIndex == 0 )
        return currentMannequin->getEndOfStomach();

    // Search for the point after the one we are looking for (the first mecanical point with mecanicalPoint.y > probePoint.y)
    int pointAfterIndex = -1;
    for( int i=1; i<currentMannequin->size(); ++i )
    {
        // If the mecanicalCurve has a point with the exact same y as probePoint, return it as result
        // this point has validity = CurveNotTested (set in the constructor (float, float, float) )
        if( mecanicalPoint(i).y == probePoint(probePointIndex).y )
            return mecanicalPoint(i);
        // choose the first point with mecanicalPoint.y > probePoint.y
        else if( mecanicalPoint(i).y > probePoint(probePointIndex).y )
        {
            pointAfterIndex = i;
            break;
        }
    }

    // Return the point between pointAfterIndex and pointAfterIndex-1 with y = probePoint.y

    // targetCurve[after] is the point after (on the y axis) the one we are searching
    // targetCurve[after-1] is the point before (on the y axis) the one we are searching
    // there should never be an out of range problem with targetCurve[after-1] since after cannot be the first Point in the list.

    // these 2 are just for readability
    Point before = mecanicalPoint(pointAfterIndex-1);
    Point after = mecanicalPoint(pointAfterIndex);

    Point line( after.x - before.x,
                after.y - before.y,
                after.z - before.z );
    float t = (probePoint(probePointIndex).y - before.y) / line.y;

    float x = before.x + line.x * t;
    float y = probePoint(probePointIndex).y;
    float z = before.z + line.z * t;

    return Point(x, y, z);
}

float CurveComparer::segmentLength( Curve* curve, int startIndex, int endIndex )
{
    float result = 0.0f;

    // go through all the points from startIndex to endIndex and calculate the total
    // length of the segment
    for( int i=startIndex; i<endIndex-1; ++i )
    {
        result += distanceBetween2Points( (*curve)[i], (*curve)[i+1] );
    }

    return result;
}

void CurveComparer::setIgnoredPoints()
{
    int i;
    for( i=0; i<probeCurve->size(); ++i )
    {
        // beginning of the mecanical curve (bottom):
        // except the first element (it has to be tested with endOfStomach)
        // all points with a y lower than the first mecanicalCurve point are ignored
        if( i > 0 && probePoint(i).y < mecanicalPoint(0).y )
            probePoint(i).validity = PointValidity::Ignored;

        // end of the mecanical curve (top):
        // find the first point with an y > pointMaxY
        // from this point on, points should not be tested (most of them are supposed to be outside the mannequin)
        if( probePoint(i).y > currentMannequin->getMaxY() )
        {
            probePoint(i).validity = PointValidity::Ignored;
            break;
        }
    }

    // once you found it, set its validity and all the following points to Ignored
    for( int j=i+1; j<probeCurve->size(); ++j )
    {
        probePoint(j).validity = PointValidity::Ignored;
    }
}

float CurveComparer::distanceBetween2Points( const Point& p1, const Point& p2 )
{
    // distance between 2 points in a 3d space
    // distance = sqrt( (p1.x - p2.x)^2 + (p1.y - p2.y)^2 + (p1.z - p2.z)^2 )
    float dx = p1.x - p2.x;
    float dy = p1.y - p2.y;
    float dz = p1.z - p2.z;

    return sqrt( dx*dx + dy*dy + dz*dz );
}

//  Accessors for members variables, for the OpenGL view
/********************************************************************************/

Mannequin* CurveComparer::getCurrentMannequin() const
{
    return currentMannequin;
}

Curve* CurveComparer::getProbeCurve() const
{
    return probeCurve;
}

// these 2 are just for code readability, private
Point& CurveComparer::probePoint( int i )
{
    return (*probeCurve)[i];
}

Point& CurveComparer::mecanicalPoint( int i )
{
    return (*currentMannequin)[i];
}

// these 2 are public
Point CurveComparer::getProbePoint( int i ) const
{
    return (*probeCurve)[i];
}

Point CurveComparer::getMecanicalPoint( int i ) const
{
    return (*currentMannequin)[i];
}
