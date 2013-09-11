#include "GLWidget.h"

GLWidget::GLWidget( CurveComparer* cc, QWidget *parent ) : QGLWidget( parent )
{
    this->cc = cc;

    int timerInterval = 1000 / 30; // second / fps
    timer = new QTimer( this );
    connect( timer, SIGNAL(timeout()), this, SLOT(timeOutSlot()));
    timer->start( timerInterval );

    posX = 0.0f;
    posY = -7.0f;
    posZ = -50.0f;
    rotationY = 0.0f;
    rotationX = 0.0f;
}

void GLWidget::paintGL()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    if( cc->getValidity() != CurveValidity::MannequinUnavailable &&
        cc->getValidity() != CurveValidity::NotTested )
    {
        glLoadIdentity();
        glTranslatef( posX, posY, posZ );
        glRotatef( rotationY, 0.0f, 1.0f, 0.0f );
        glRotatef( rotationX, 1.0f, 0.0f, 0.0f );

        float radius = cc->getCurrentMannequin()->getRadius();

        glBegin(GL_LINES);
            for( int i=0; i<cc->getCurrentMannequin()->size()-1; ++i )
            {
                // Mecanical curve lines : YELLOW
                glColor3f( 255.0f, 255.0f, 0.0f );
                glVertex3d( cc->getCurrentMannequin()->at(i).x, cc->getCurrentMannequin()->at(i).y, cc->getCurrentMannequin()->at(i).z );
                glVertex3d( cc->getCurrentMannequin()->at(i+1).x, cc->getCurrentMannequin()->at(i+1).y, cc->getCurrentMannequin()->at(i+1).z);

                // Radius lines : MAGENTA
                glColor3f( 255.0f, 0.0f, 255.0f );
                glVertex3d( cc->getCurrentMannequin()->at(i).x + radius, cc->getCurrentMannequin()->at(i).y, cc->getCurrentMannequin()->at(i).z );
                glVertex3d( cc->getCurrentMannequin()->at(i+1).x + radius, cc->getCurrentMannequin()->at(i+1).y, cc->getCurrentMannequin()->at(i+1).z);
                glVertex3d( cc->getCurrentMannequin()->at(i).x - radius, cc->getCurrentMannequin()->at(i).y, cc->getCurrentMannequin()->at(i).z );
                glVertex3d( cc->getCurrentMannequin()->at(i+1).x - radius, cc->getCurrentMannequin()->at(i+1).y, cc->getCurrentMannequin()->at(i+1).z);
            }

            // Radius line stomach : TEAL
            glColor3f( 0.0f, 100.0f, 255.0f );
            glVertex3d( cc->getCurrentMannequin()->at(0).x + radius,
                        cc->getCurrentMannequin()->at(0).y,
                        cc->getCurrentMannequin()->at(0).z );
            glVertex3d( cc->getCurrentMannequin()->getEndOfStomach().x + radius,
                        cc->getCurrentMannequin()->getEndOfStomach().y,
                        cc->getCurrentMannequin()->getEndOfStomach().z );
            glVertex3d( cc->getCurrentMannequin()->at(0).x - radius,
                        cc->getCurrentMannequin()->at(0).y,
                        cc->getCurrentMannequin()->at(0).z );
            glVertex3d( cc->getCurrentMannequin()->getEndOfStomach().x - radius,
                        cc->getCurrentMannequin()->getEndOfStomach().y,
                        cc->getCurrentMannequin()->getEndOfStomach().z );

            // Probe curve lines
            // Valid : GREEN
            // Invalid : RED
            // Ignored : TEAL
            for( int i=0; i<cc->getProbeCurve()->size()-1; ++i )
            {
                // RED line if at least one of the two points is invalid.
                // GREEN line if both points are valid.
                // TEAL otherwise.
                if( cc->getProbePoint(i).validity == PointValidity::Ignored &&
                    cc->getProbePoint(i+1).validity == PointValidity::Ignored )  // both points are ignored
                    setColor( PointValidity::Ignored );
                else if( cc->getProbePoint(i).validity == PointValidity::Invalid ||
                         cc->getProbePoint(i+1).validity == PointValidity::Invalid )  // at least one is invalid
                    setColor( PointValidity::Invalid );
                else if( cc->getProbePoint(i).validity == PointValidity::Valid &&
                         cc->getProbePoint(i+1).validity == PointValidity::Valid ) // both are valid
                    setColor( PointValidity::Valid );
                else
                    setColor( PointValidity::Ignored );


                glVertex3d( cc->getProbePoint(i).x, cc->getProbePoint(i).y, cc->getProbePoint(i).z );
                glVertex3d( cc->getProbePoint(i+1).x, cc->getProbePoint(i+1).y, cc->getProbePoint(i+1).z);
            }
        glEnd();

        glBegin(GL_POINTS);
            // point end of stomach
            glColor3f( 255.0f, 255.0f, 255.0f );
            glVertex3d( cc->getCurrentMannequin()->getEndOfStomach().x,
                        cc->getCurrentMannequin()->getEndOfStomach().y,
                        cc->getCurrentMannequin()->getEndOfStomach().z );

            // draw the probe curve points
            for( int i=0; i<cc->getProbeCurve()->size(); ++i )
            {
                setColor( cc->getProbePoint(i).validity );
                glVertex3d( cc->getProbePoint(i).x, cc->getProbePoint(i).y, cc->getProbePoint(i).z );
            }
        glEnd();
    }
}

void GLWidget::setColor( PointValidity::Status validity )
{
    switch( validity )
    {
    case PointValidity::Valid:
        glColor3f( 0.0f, 255.0f, 0.0f );
        break;
    case PointValidity::Ignored:
        glColor3f( 0.0f, 255.0f, 255.0f );
        break;
    case PointValidity::Invalid:
        glColor3f( 255.0f, 0.0f, 0.0f );
        break;
    case PointValidity::NotTested:
        glColor3f( 128.0f, 128.0f, 128.0f ); // should not happen
        break;
    default: break; // should not happen
    }
}

void GLWidget::resizeGL(int width, int height)
{
    if(height == 0)
        height = 1;
    glViewport(0, 0, width, height);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45.0f, (GLfloat)width/(GLfloat)height, 0.1f, 200.0f);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

void GLWidget::initializeGL()
{
    glShadeModel( GL_SMOOTH );
    glClearColor( 0.0f, 0.0f, 0.0f, 0.0f );
    glClearDepth( 1.0f );
    glEnable( GL_DEPTH_TEST );
    glDepthFunc( GL_LEQUAL );
    glLineWidth( 1.0f );
    glPointSize( 4.0f );
    glHint( GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST );
    glEnable( GL_POINT_SMOOTH );
}

void GLWidget::timeOutSlot()
{
    updateGL();
}

void GLWidget::posXOffset( float value )
{
    posX += value;
}

void GLWidget::posYOffset( float value )
{
    posY += value;
}

void GLWidget::posZOffset( float value )
{
    posZ += value;
}

void GLWidget::rotationXOffset( float value )
{
    rotationX += value;
}

void GLWidget::rotationYOffset( float value )
{
    rotationY += value;
}

void GLWidget::resetView()
{
    rotationX = 0.0f;
    rotationY = 0.0f;
    posX = 0.0f;
    posY = -7.0f;
    posZ = -50.0f;
}
