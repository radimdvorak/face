#ifndef EVALUATEKINECT_H
#define EVALUATEKINECT_H

#include <QDir>
#include <QFileInfoList>
#include <QFileInfo>
#include <QApplication>

#include "facelib/glwidget.h"
#include "facelib/mesh.h"
#include "facelib/surfaceprocessor.h"
#include "linalg/common.h"
#include "biometrics/featureextractor.h"
#include "biometrics/isocurveprocessing.h"
#include "biometrics/evaluation.h"
#include "biometrics/scorelevefusion.h"
#include "biometrics/histogramfeatures.h"
#include "facelib/landmarkdetector.h"
#include "facelib/facealigner.h"
#include "linalg/kernelgenerator.h"
#include "linalg/serialization.h"
#include "linalg/loader.h"
#include "linalg/matrixconverter.h"
#include "biometrics/facetemplate.h"

class EvaluateKinect
{
public:

    static void evaluateReferenceDistances()
    {
        FaceClassifier faceClassifier("../../test/kinect/classifiers2/");

        QMap<int, FaceTemplate *> references;

        QVector<QString> templateFiles = Loader::listFiles("../../test/kinect/", "*.yml", AbsoluteFull);
        foreach(const QString &path, templateFiles)
        {
            int id = QFileInfo(path).baseName().split("-")[0].toInt();
            FaceTemplate *t = new FaceTemplate(id, path, faceClassifier);
            references.insertMulti(id, t);
        }

        foreach (int id, references.uniqueKeys())
        {
            qDebug() << id;
            QList<FaceTemplate *> ref = references.values(id);
            foreach (const FaceTemplate *probe, ref)
            {
                qDebug() << "  " << faceClassifier.compare(ref, probe, FaceClassifier::CompareMeanDistance);
            }
        }
    }

    static void evaluateRefeference()
    {
        FaceClassifier faceClassifier("../../test/kinect/classifiers/");

        QHash<int, FaceTemplate *> references;
        QVector<FaceTemplate *> testTemplates;

        QVector<QString> templateFiles = Loader::listFiles("../../test/kinect/", "*.yml", AbsoluteFull);
        foreach(const QString &path, templateFiles)
        {
            int id = QFileInfo(path).baseName().split("-")[0].toInt();
            FaceTemplate *t = new FaceTemplate(id, path, faceClassifier);

            if (!references.contains(id) || references.values(id).count() < 2)
            {
                references.insertMulti(id, t);
            }
            else
            {
                testTemplates << t;
            }
        }

        Evaluation e = faceClassifier.evaluate(references, testTemplates, FaceClassifier::CompareMeanDistance);
        qDebug() << e.eer; // << e.fnmrAtFmr(0.01) << e.fnmrAtFmr(0.001) << e.fnmrAtFmr(0.0001);
        qDebug() << e.maxSameDistance << e.minDifferentDistance;
        //e.outputResults("kinect", 10);
    }

    static void evaluateSimple()
    {
        FaceClassifier faceClassifier("../../test/kinect/classifiers/");
        QVector<FaceTemplate*> templates;

        QVector<QString> templateFiles = Loader::listFiles("../../test/kinect/", "*.yml", AbsoluteFull);
        foreach(const QString &path, templateFiles)
        {
            int id = QFileInfo(path).baseName().split("-")[0].toInt();
            templates << new FaceTemplate(id, path, faceClassifier);
        }

        Evaluation e = faceClassifier.evaluate(templates);
        qDebug() << e.eer;

        foreach (FaceTemplate *t, templates)
        {
            delete t;
        }
        //e.outputResults("kinect", 15);
    }

    static void createTemplates()
    {
        FaceAligner aligner(Mesh::fromOBJ("../../test/meanForAlign.obj", false));
        FaceClassifier faceClassifier("../../test/kinect/classifiers2/");

        QVector<QString> binFiles = Loader::listFiles("../../test/kinect/", "*.bin", AbsoluteFull);
        foreach(const QString &path, binFiles)
        {
            QString baseName = QFileInfo(path).baseName();
            int id = baseName.split("-")[0].toInt();
            Mesh face = Mesh::fromBIN(path);
            aligner.icpAlign(face, 10, FaceAligner::NoseTipDetection);

            FaceTemplate t(id, face, faceClassifier);
            t.serialize("../../test/kinect/" + baseName + ".yml.gz", faceClassifier);
        }
    }

    static void learnFromFrgc()
    {
        FaceAligner aligner(Mesh::fromOBJ("../../test/meanForAlign.obj", false));
        FaceClassifier faceClassifier("../../test/frgc/classifiers/");
        QVector<FaceTemplate*> templates;

        QVector<QString> binFiles = Loader::listFiles("../../test/kinect/", "*.bin", AbsoluteFull);
        foreach(const QString &path, binFiles)
        {
            int id = QFileInfo(path).baseName().split("-")[0].toInt();
            Mesh face = Mesh::fromBIN(path);
            aligner.icpAlign(face, 10, FaceAligner::NoseTipDetection);
            templates << new FaceTemplate(id, face, faceClassifier);
        }

        ScoreSVMFusion newFusion = faceClassifier.relearnFinalFusion(templates);
        //faceClassifier.serialize("../../test/kinect/classifiers2");
        //newFusion.serialize("../../test/kinect/classifiers2/final");
    }

    static void evaluateKinect()
    {
        FaceAligner aligner(Mesh::fromOBJ("../../test/meanForAlign.obj", false));
        FaceClassifier faceClassifier("../../test/kinect/classifiers/");
        QVector<FaceTemplate*> templates;

        QVector<QString> binFiles = Loader::listFiles("../../test/kinect/", "*.bin", AbsoluteFull);
        foreach(const QString &path, binFiles)
        {
            int id = QFileInfo(path).baseName().split("-")[0].toInt();
            Mesh face = Mesh::fromBIN(path, true);
            aligner.icpAlign(face, 10, FaceAligner::NoseTipDetection);
            templates << new FaceTemplate(id, face, faceClassifier);
        }

        Evaluation eval = faceClassifier.evaluate(templates);
        qDebug() << eval.eer;
        //eval.outputResults("kinect", 10);
    }

};
#endif // EVALUATEKINECT_H
