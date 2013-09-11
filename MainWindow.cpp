#include "MainWindow.h"
#include "ui_MainWindow.h"

MainWindow::MainWindow( QWidget* parent ) : QMainWindow( parent ), ui( new Ui::MainWindow )
{
    ui->setupUi(this);

    label = new QLabel( "", this );
    label->setGeometry( 820, 10, 220, 100 );

    // First point in all lists (mecanical and probe) should be the lowest y of the curve (starts in the stomach)

    Curve* probeCurve = loadProbeCurve( "zigzag_fast.csv" );

    cc = new CurveComparer();
    cc->addMannequin( new Mannequin( "bob2.mannequin" ) );

    qDebug() << curveValidityString( cc->isCurveValid( "BOB002", probeCurve ) );

    glView = new GLWidget( cc, this );
    glView->setGeometry( 10, 10, 800, 600 );

    setFixedSize( 1050, 620 );
}

QString MainWindow::curveValidityString( CurveValidity::Status validity ) const
{
    switch( validity )
    {
    case CurveValidity::Invalid:
        label->setText( "Invalid" );
        label->setStyleSheet( "background-color: #ff0000; color: #ffffff; text-align: center; font-size: 16px; font-weight: bold;" );
        return "Invalid";
        break;

    case CurveValidity::MannequinUnavailable:
        label->setText( "This mannequin doesn't exist." );
        label->setStyleSheet( "background-color: #ffff00; color: #000000; text-align: center; font-size: 16px; font-weight: bold;" );
        return "MannequinUnavailable";
        break;

    case CurveValidity::NotEnoughDataLength:
        label->setText( "Curve not long enough" );
        label->setStyleSheet( "background-color: #ffff00; color: #000000; text-align: center; font-size: 16px; font-weight: bold;" );
        return "NotEnoughDataLength";
        break;

    case CurveValidity::NotEnoughDataPoints:
        label->setText( "Not enough points" );
        label->setStyleSheet( "background-color: #ffff00; color: #000000; text-align: center; font-size: 16px; font-weight: bold;" );
        return "NotEnoughDataPoints";
        break;

    case CurveValidity::Valid:
        label->setText( "Valid" );
        label->setStyleSheet( "background-color: #00ff00; color: #ffffff; text-align: center; font-size: 16px; font-weight: bold;" );
        return "Valid";
        break;

    case CurveValidity::NotTested:  // should not heppen
        label->setText( "WTF NOT TESTED" );
        label->setStyleSheet( "background-color: #ff0000; color: #ffffff; text-align: center; font-size: 16px; font-weight: bold;" );
        return "NotTested";
        break;

    default: return ""; break;      // should not happen
    }
}

void MainWindow::keyPressEvent( QKeyEvent* keyEvent )
{
    switch( keyEvent->key() )
    {
        case Qt::Key_Left:
            glView->posXOffset( 1.0f );
            break;

        case Qt::Key_Right:
            glView->posXOffset( -1.0f );
            break;

        case Qt::Key_Down:
            glView->posYOffset( 1.0f );
            break;

        case Qt::Key_Up:
            glView->posYOffset( -1.0f );
            break;

        case Qt::Key_Minus:
            glView->posZOffset( -1.0f );
            break;

        case Qt::Key_Plus:
            glView->posZOffset( 1.0f );
            break;

        case Qt::Key_Asterisk:  //this is for testing
            cc->getCurrentMannequin()->setRadius( cc->getCurrentMannequin()->getRadius() + 0.1f );
            qDebug() << curveValidityString( cc->isCurveValid( "BOB002", cc->getProbeCurve() ) );
            break;

        case Qt::Key_Slash:     //this is for testing
            cc->getCurrentMannequin()->setRadius( cc->getCurrentMannequin()->getRadius() - 0.1f );
            qDebug() << curveValidityString( cc->isCurveValid( "BOB002", cc->getProbeCurve() ) );
            break;

        case Qt::Key_1:
            glView->rotationYOffset( -5.0f );
            break;

        case Qt::Key_3:
            glView->rotationYOffset( 5.0f );
            break;

        case Qt::Key_5:
            glView->rotationXOffset( -5.0f );
            break;

        case Qt::Key_2:
            glView->rotationXOffset( 5.0f );
            break;

        case Qt::Key_7:
            glView->resetView();
            break;
    }
}

// This method load the points of the probe's curve from a csv file.
// The data is collected from a metrics csv file, parsed to be easier to read
// mostly intended for test purpose only since the probe curve points aquisition will probably not come from a file
Curve* MainWindow::loadProbeCurve( const QString& filename )
{
    QFile file( filename );
    Curve* probeCurve = new Curve();

    if( file.open( QIODevice::ReadOnly | QIODevice::Text ) )
    {
        while( !file.atEnd() )
        {
            QString line = file.readLine();
            QStringList pos = line.split(";");
            if( pos.size() == 3 )
            {
                float x = pos[0].toFloat();
                float y = pos[1].toFloat();
                float z = pos[2].toFloat();
                probeCurve->append( Point(x, y, z) );
            }
            else
                qDebug() << "File " + filename + " doesn't have the right format.";
        }
        file.close();
    }

    qDebug() << filename << "loaded. It contains " << probeCurve->size() << " points.";

    return probeCurve;
}

MainWindow::~MainWindow()
{
    delete glView;
    delete ui;
    delete cc;
}
