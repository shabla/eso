#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include "CurveComparer.h"
#include "GLWidget.h"

namespace Ui
{
    class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

    private:
        Ui::MainWindow* ui;
        GLWidget*       glView;
        CurveComparer*  cc;
        QLabel*         label;

        Curve*      loadProbeCurve( const QString& filename );
        QString     curveValidityString( CurveValidity::Status validity ) const;
    
    public:
        explicit MainWindow( QWidget *parent = 0 );
        ~MainWindow();
        void keyPressEvent( QKeyEvent *keyEvent );
};

#endif // MAINWINDOW_H
