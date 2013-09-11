#ifndef GLWIDGET_H
#define GLWIDGET_H

#include <QtOpenGL>
#include <QGLWidget>
#include <GL/GLU.h>
#include <QList>

#include "CurveComparer.h"

class GLWidget : public QGLWidget
{
    Q_OBJECT

    private:
        CurveComparer*  cc;
        Curve*          probeCurve;
        QTimer*         timer;

        float posX, posY, posZ;
        float rotationY, rotationX;

        void setColor( PointValidity::Status validity );

    public slots:
        void timeOutSlot();

    public:
        explicit GLWidget( CurveComparer* cc, QWidget *parent = 0 );
        void initializeGL();
        void resizeGL( int width, int height );
        void paintGL();

        // accessors
        void posXOffset( float value );
        void posYOffset( float value );
        void posZOffset( float value );
        void rotationXOffset( float value );
        void rotationYOffset( float value );
        void resetView();
};

#endif // GLWIDGET_H
