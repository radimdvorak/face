#include "ldaofpca.h"

#include <QDebug>

LDAofPCA::LDAofPCA (QVector<Vector> &vectors, QVector<int> &classMembership, double pcaSelectionThreshold, bool debug)
{
    learn(vectors, classMembership, pcaSelectionThreshold, debug);
}

void LDAofPCA::learn(QVector<Vector> &vectors, QVector<int> &classMembership, double pcaSelectionThreshold, bool debug)
{
    // pca
    pca.learn(vectors, 0, debug);
    int oldModes = pca.getModes();
    pca.modesSelectionThreshold(pcaSelectionThreshold);
    if (debug)
        qDebug() << "PCA Done; |modes| =" << pca.getModes() << "(was" << oldModes << ")";

    if (debug)
        qDebug() << "PCA projection";
    QVector<Vector> projected;
    for (int i = 0; i < vectors.count(); i++)
    {
        Matrix p = pca.project(vectors[i]);
        projected.append(p);
    }

    // lda
    lda.learn(projected, classMembership, debug);
}

Vector LDAofPCA::project(const Vector &vector)
{
    // pca projection
    Matrix pcaProjection = pca.project(vector);

    // lda projection
    return lda.project(pcaProjection);
}

QVector<Vector> LDAofPCA::project(const QVector<Vector> &vectors)
{
    QVector<Vector> result;
    for (int i = 0; i < vectors.count(); i++)
    {
        Matrix out = project(vectors[i]);
        result.append(out);
    }
    return result;
}
