#ifndef TESTGLWIDGET_H
#define TESTGLWIDGET_H

#include <QApplication>

#include "facelib/glwidget.h"
#include "facelib/mesh.h"
#include "facelib/landmarks.h"

class TestGlWidget
{
public:
    static int test(int argc, char *argv[], const QString &dir)
    {
        Mesh mesh = Mesh::fromOBJ(dir + "02463d652.obj", false);
        mesh.printStats();
        Landmarks landmarks(dir + "02463d652.xml");

        /*mesh.move(-landmarks.Nosetip);
        landmarks.LeftInnerEye -= landmarks.Nosetip;
        landmarks.RightInnerEye -= landmarks.Nosetip;
        landmarks.Nosetip = cv::Point3d(0,0,0);*/

        QApplication app(argc, argv);
        GLWidget widget;
        widget.setWindowTitle("GL Widget");
        widget.addFace(&mesh);
        widget.addLandmarks(&landmarks);
        widget.show();
        return app.exec();
    }
};

#endif // TESTGLWIDGET_H