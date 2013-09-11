#ifndef POINT_H
#define POINT_H

#include <QString>

namespace PointValidity
{
    enum Status
    {
        NotTested = 0,
        Valid = 1,
        Invalid = 2,
        Ignored = 3
    };
}

struct Point
{
    float x, y, z;
    PointValidity::Status validity;

    Point() {}
    Point( PointValidity::Status v ) : validity(v) {}
    Point( float x, float y, float z ) : x(x), y(y), z(z) { validity = PointValidity::NotTested; }

    Point operator+( const Point& right ) const { return Point( x + right.x, y + right.y, z + right.z ); }
    Point operator/( float& right ) const { return Point( x / right, y / right, z / right ); }
    Point operator*( float& right ) const { return Point( x * right, y * right, z * right ); }
    bool operator==( const Point& right ) const
    {
        if( x == right.x &&
            y == right.y &&
            z == right.z )
            return true;
        else
            return false;
    }

    bool operator!=( const Point& right ) const
    {
        if( x != right.x ||
            y != right.y ||
            z != right.z )
            return true;
        else
            return false;
    }
};

#endif // POINT_H
