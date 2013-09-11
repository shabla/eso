#include "Mannequin.h"

Mannequin::Mannequin( const QString& filename )
{
    loadMannequin( filename );
    loadSettings();
    qDebug() << name << "loaded. It contains " << size() << " points.";
}

void Mannequin::loadSettings()
{
    // these settings should be loaded from a file at the same time as the mannequin file
    radius = 2.0f;
    curveLengthThreshold = 0.15f;
    maxY = 23.0f;
    maxIntervalMedian = 2.0f;

    endOfStomach = Point( 2.1306f, -17.9064f, -2.1112f );  // probe1.csv
    //endOfStomach = Point( 1.9190f, -18.2423f, -1.9777f );  // probe2.csv
    //endOfStomach = Point( 2.1231f, -17.7418f, -2.1759f );  // probe3.csv
}

void Mannequin::loadMannequin( const QString& filename )
{
    // remove all elements before adding new ones
    this->clear();

    QDomDocument doc( "mannequin" );
    QFile file( filename );

    if( !file.open( QIODevice::ReadOnly ) )
        return;

    if( !doc.setContent( &file ) )
    {
        file.close();
        return;
    }
    file.close();

    QDomElement docElem = doc.documentElement();
    QDomNode n = docElem.firstChild();

    while( !n.isNull() )
    {
        QDomElement e = n.toElement(); // try to convert the node to an element.

        if( e.tagName() == "Mannequin" )
        {
            this->name = e.attribute("mannequinID");
        }

        if( e.tagName() == "TEECurve" )
        {
            if( n.firstChildElement( "CurveInfo" ).attribute("CurveType") == "Mechanical" )
            {
                QDomNodeList list = n.childNodes();
                for( int i=1; i<list.size(); ++i ) // start at 1 to skip the CurveInfo node
                {
                    Point p( list.at(i).toElement().attribute("PositionX").toFloat(),
                             list.at(i).toElement().attribute("PositionY").toFloat(),
                             list.at(i).toElement().attribute("PositionZ").toFloat() );
                    this->append(p);
                }
            }
        }

        n = n.nextSibling();
    }
}

float Mannequin::getMaxIntervalMedian() const
{
    return maxIntervalMedian;
}

QString Mannequin::getName() const
{
    return name;
}

void Mannequin::setRadius( float value )
{
    if( value < 0.0f )
        radius = 0.0f;
    else
        radius = value;
}

float Mannequin::getRadius() const
{
    return radius;
}

Point Mannequin::getEndOfStomach() const
{
    return endOfStomach;
}

float Mannequin::getCurveLengthThreshold() const
{
    return curveLengthThreshold;
}

float Mannequin::getMaxY() const
{
    return maxY;
}
