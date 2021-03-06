#ifndef DLGREALTIMECOMPARE_H
#define DLGREALTIMECOMPARE_H

#include <QDialog>

#include "facetrack/realtimetrack.h"
#include "facelib/mesh.h"
#include "biometrics/facetemplate.h"
#include "facelib/facealigner.h"
#include "biometrics/realtimeclassifier.h"

namespace Ui {
class DlgRealTimeCompare;
}

class DlgRealTimeCompare : public QDialog
{
    Q_OBJECT

public:
    explicit DlgRealTimeCompare(RealTimeClassifier *classifier, const QMap<int, QString> &mapIdToName,
                                const QString &faceHaarPath, QWidget *parent = 0);
    ~DlgRealTimeCompare();

private:
    Ui::DlgRealTimeCompare *ui;
    RealTimeClassifier *classifier;
    const QMap<int, QString> &mapIdToName;
    QTimer *timer;
    RealTimeTracker tracker;
    ImageGrayscale frame;
    u_int8_t rgbBuffer[640*480*3];
    double depthBuffer[640*480];
    bool mask[640*480];

private slots:
    void showFace();
};

#endif // DLGREALTIMECOMPARE_H
