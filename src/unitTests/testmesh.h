#ifndef TESTMESH_H
#define TESTMESH_H

#include <QApplication>
#include <QDir>
#include <QFileInfoList>

#include "facelib/glwidget.h"
#include "facelib/mesh.h"
#include "facelib/landmarks.h"
#include "facelib/surfaceprocessor.h"

class TestMesh
{
public:

    static void testReadWriteCharArray(const QString &frgcPath)
    {
        QString path = frgcPath + "zbin-aligned/02463d652.binz";
        Mesh m1 = Mesh::fromBINZ(path);
        int n;
        char *data = m1.toCharArray(&n);
        Mesh m2 = Mesh::fromCharArray(data, n);
        delete [] data;
        assert(m1.equals(m2));
    }

    static void testXYZLodaderOBJWriter(const QString &srcDirPath, const QString &dstDirPath)
    {
        QDir srcDir(srcDirPath, "*.xyz");
        QFileInfoList srcEntries = srcDir.entryInfoList();
        foreach (const QFileInfo &srcFileInfo, srcEntries)
        {
            Mesh srcMesh = Mesh::fromXYZ(srcFileInfo.absoluteFilePath(), true);
            QString dstFilePath = dstDirPath + QDir::separator() + srcFileInfo.baseName() + ".obj";
            srcMesh.writeOBJ(dstFilePath, '.');
        }
    }

    static int testBinReadWrite(int argc, char *argv[], const QString &dir)
    {
        Mesh mesh = Mesh::fromXYZ(dir + "xyz/02463d652.abs.xyz", false);

        mesh.writeBIN("face.bin");
        Mesh mesh2 = Mesh::fromBIN("face.bin");

        QApplication app(argc, argv);
        GLWidget widget;
        widget.setWindowTitle("GL Widget");
        widget.addFace(&mesh2);
        widget.show();
        return app.exec();
    }

    static int testReadBinWriteBinzReadBinz(int argc, char *argv[], const QString &dir)
    {
        Mesh mesh = Mesh::fromBIN(dir + "bin/02463d652.bin", true);
        mesh.writeBINZ("mesh.binz");
        mesh = Mesh::fromBINZ("mesh.binz");
        mesh.printStats();

        QApplication app(argc, argv);
        GLWidget widget;
        widget.setWindowTitle("GL Widget");
        widget.addFace(&mesh);
        widget.show();
        return app.exec();
    }

    static int readAbsWithTexture(int argc, char *argv[])
    {
        Mesh mesh = Mesh::fromABS("/run/media/stepo/My Book/3D-FRGC-data/nd1/Spring2004range/02463d652.abs",
                                  "/run/media/stepo/My Book/3D-FRGC-data/nd1/Spring2004range/02463d653.ppm", true);

        QApplication app(argc, argv);
        GLWidget widget;
        widget.setWindowTitle("GL Widget");
        widget.addFace(&mesh);
        widget.show();
        return app.exec();
    }
};

#endif // TESTMESH_H

