#include "procrustes.h"

#include <cassert>
#include <cmath>

/***
 * Procrustes analysis
 */
void Procrustes::procrustesAnalysis(QVector<Matrix> &vectors, bool scale, double eps, int maxIterations)
{
    qDebug() << "Procrustes analysis";

    int count = vectors.count();
    assert(count > 0);

    int len = vectors[0].rows;
    assert(len > 0);
    assert(len % 2 == 0);
    //int n = len / 2;

    // Translate each vector to the center of origin
    centralize(vectors);

    // Choose one example as an initial estimate of the mean shape and scale so that |x| = 1.
    Matrix mean = scale ? Vector::normalized(vectors[0]) : vectors[0];
    Matrix oldMean = mean.clone();
    double oldDelta = 1e300;

    int iteration = 1;
    bool end = false;
    while (!end)
    {
        // Align all the shapes with the current estimate of the mean shape.
        for (int i = 0; i < count; i++)
        {
            if (scale)
            {
                RotateAndScaleCoefs c = align(vectors[i], mean);
                rotateAndScale(vectors[i], c);
            }
            else
            {
                double theta = getOptimalRotation(vectors[i], mean);
                RotateAndScaleCoefs c; c.theta = theta;
                rotateAndScale(vectors[i], c);
            }
        }

        // Re-estimate mean from aligned shapes
        mean = getMeanShape(vectors);
        /*Matrix::zeros(mean.rows, mean.cols, CV_64F);
        for (int i = 0; i < count; i++)
        {
            mean = mean + vectors[i];
        }
        Vector::mul(mean, 1.0/count);*/

        // Apply constraints on the current estimate of the mean
        if (scale)
            Vector::normalize(mean);

        // If not converged, iterate
        Matrix diff = mean-oldMean;
        double delta = Vector::sqrMagnitude(diff);

        if (delta <= eps || iteration > maxIterations)
            end = true;

        oldMean = mean.clone();
        oldDelta = delta;

        if (iteration % 100 == 0)
            qDebug() << " iteration:" << iteration << "; delta:" << delta;

        iteration += 1;
    }
}

void Procrustes::translate(Matrix &vector, TranslationCoefs &coefs)
{
    int n = vector.rows/2;
    for (int i = 0; i < n; i++)
    {
        vector(i, 0) += coefs.xt;
        vector(i+n, 0) += coefs.yt;
    }
}

TranslationCoefs Procrustes::centralizedTranslation(Matrix &vector)
{
    double meanx = 0.0;
    double meany = 0.0;
    int n = vector.rows/2;

    for (int i = 0; i < n; i++)
    {
        meanx += vector(i, 0);
        meany += vector(i+n, 0);
    }
    meanx /= n;
    meany /= n;

    TranslationCoefs c;
    c.xt = -meanx;
    c.yt = -meany;
    return c;
}

void Procrustes::centralize(Matrix &vector)
{
    TranslationCoefs c = centralizedTranslation(vector);
    translate(vector, c);
}

void Procrustes::centralize(QVector<Matrix> &vectors)
{
	int n = vectors.count();
	for (int i = 0; i < n; i++)
		centralize(vectors[i]);
}

void Procrustes::rotateAndScale(Matrix &vector, RotateAndScaleCoefs &coefs)
{
    double sint = coefs.s * sin(coefs.theta);
    double cost = coefs.s * cos(coefs.theta);

    int n = vector.rows/2;
    for (int i = 0; i < n; i++)
    {
        double oldx = vector(i, 0);
        double oldy = vector(i+n, 0);
        double x = cost*oldx - sint*oldy;
        double y = sint*oldx + cost*oldy;

        vector(i, 0) = x;
        vector(i+n, 0) = y;
    }
}

void Procrustes::transformate(Matrix &vector, TransformationCoefs &coefs)
{
    int n = vector.rows/2;
    for (int i = 0; i < n; i++)
    {
        double oldx = vector(i, 0);
        double oldy = vector(i+n, 0);
        double x = coefs.a * oldx + -coefs.b * oldy;
        double y = coefs.b * oldx + coefs.a * oldy;

        vector(i, 0) = x;
        vector(i+n, 0) = y;
    }
}

double Procrustes::getOptimalRotation(Matrix &from, Matrix &to)
{
    int n = from.rows/2;
    double numerator = 0.0;
    double denumerator = 0.0;
    for (int i = 0; i < n; i++)
    {
        numerator += (from(i, 0) * to(n+i, 0) - from(n+i, 0) * to(i, 0));
        denumerator += (from(i, 0) * to(i, 0) + from(n+i, 0) * to(n+i, 0));
    }

    return atan(numerator/denumerator);
}

RotateAndScaleCoefs Procrustes::align(Matrix &from, Matrix &to)
{
    Matrix reference = to.clone();
    double referenceScale = 1.0/Vector::magnitude(reference);
    Vector::mul(reference, referenceScale);

    Matrix vector = from.clone();
    double vectorScale = 1.0/Vector::magnitude(vector);
    Vector::mul(vector, vectorScale);

    /*int n = vector.rows/2;
    double numerator = 0.0;
    double denumerator = 0.0;
    for (int i = 0; i < n; i++)
    {
        numerator += (vector(i, 0) * reference(n+i, 0) - vector(n+i, 0) * reference(i, 0));
        denumerator += (vector(i, 0) * reference(i, 0) + vector(n+i, 0) * reference(n+i, 0));
    }*/

    double s = vectorScale/referenceScale;
    //double theta = atan(numerator/denumerator);
    double theta = getOptimalRotation(vector, reference);
    RotateAndScaleCoefs c(s, theta);

    //reference.mult(1.0/referenceScale);
    //vector.mult(1.0/vectorScale);

    return c;
}

Matrix Procrustes::getMeanShape(QVector<Matrix> &vectors)
{
	int n = vectors.count();
    Matrix mean = Matrix::zeros(vectors[0].rows, 1);
	for (int i = 0; i < n; i++)
	{
		mean += vectors[0];
	}
	mean = mean / ((double)n);

	return mean;
}

/*TransformationCoefs Procrustes::AlignAlt(Vector &from, Vector &to)
{
    int n = from.len()/2;
    double vecSqr = from.sqrMagnitude();

    double a = (from*to)/vecSqr;
    double b = 0.0;
    for (int i = 0; i < n; i++)
        b = b+(from.data.at(i) * to.data.at(i+n) + from.data.at(i+n) * to.data.at(i));
    b = b/vecSqr;

    TransformationCoefs coefs;
    coefs.a = a;
    coefs.b = b;
    return coefs;
}*/

cv::Point3d Procrustes::centralizedTranslation(const QVector<cv::Point3d> &shape)
{
    cv::Point3d mean(0,0,0);
    int numberOfPoints = shape.count();
    for (int j = 0; j < numberOfPoints; j++)
    {
        mean += shape[j];
    }
    mean.x /= numberOfPoints;
    mean.y /= numberOfPoints;
    mean.z /= numberOfPoints;

    cv::Point3d shift = -mean;
    return shift;
}

Matrix Procrustes::alignRigid(QVector<cv::Point3d> &from, QVector<cv::Point3d> &to, bool centralize)
{
    int n = from.count();
    assert(n == to.count());

    // centralize
    if (centralize)
    {
        cv::Point3d cumulativeA(0, 0, 0);
        cv::Point3d cumulativeB(0, 0, 0);
        for (int i = 0; i < n; i++)
        {
            cumulativeA += from[i];
            cumulativeB += to[i];
        }
        cumulativeA.x = cumulativeA.x/n;
        cumulativeA.y = cumulativeA.y/n;
        cumulativeA.z = cumulativeA.z/n;
        cumulativeB.x = cumulativeB.x/n;
        cumulativeB.y = cumulativeB.y/n;
        cumulativeB.z = cumulativeB.z/n;

        for (int i = 0; i < n; i++)
        {
            from[i] = from[i] - cumulativeA;
            to[i] = to[i] - cumulativeB;
        }
    }

    Matrix R = getOptimalRotation(from, to);
    transform(from, R);
    return R;
}

Matrix Procrustes::getOptimalRotation(QVector<cv::Point3d> &from, QVector<cv::Point3d> &to)
{
    int n = from.count();
    assert(n == to.count());

    // calculate H
    Matrix H = Matrix::zeros(3, 3);
    for (int i = 0; i < n; i++)
    {
        Matrix A = (Matrix(3,1) << from[i].x, from[i].y, from[i].z);
        Matrix B = (Matrix(3,1) << to[i].x, to[i].y, to[i].z);
        H += (A * B.t());
    }

    // SVD(H)
    cv::SVD svd(H);

    // R = VU^T
    Matrix R = svd.vt.t() * svd.u.t();
    return R;
}

void Procrustes::transform(cv::Point3d &p, Matrix &m)
{
    Matrix A = (Matrix(3,1) << p.x, p.y, p.z);
    A = m*A;
    p.x = A(0); p.y = A(1); p.z = A(2);
}


void Procrustes::transform(QVector<cv::Point3d> &points, Matrix &m)
{
    int n = points.count();
    for (int i = 0; i < n; i++)
    {
        cv::Point3d &p = points[i];
        transform(p, m);
    }
}

cv::Point3d Procrustes::getOptimalScale(const QVector<cv::Point3d> &from, const QVector<cv::Point3d> &to)
{
    cv::Point3d scaleParamsNumerator(0.0, 0.0, 0.0);
    cv::Point3d scaleParamsDenominator(0.0, 0.0, 0.0);
    int pointCount = from.count();
    for (int i = 0; i < pointCount; i++)
    {
        scaleParamsNumerator.x += from[i].x * to[i].x;
        scaleParamsNumerator.y += from[i].y * to[i].y;
        scaleParamsNumerator.z += from[i].z * to[i].z;

        scaleParamsDenominator.x += from[i].x * from[i].x;
        scaleParamsDenominator.y += from[i].y * from[i].y;
        scaleParamsDenominator.z += from[i].z * from[i].z;
    }
    cv::Point3d scaleParams(scaleParamsNumerator.x/scaleParamsDenominator.x,
                            scaleParamsNumerator.y/scaleParamsDenominator.y,
                            scaleParamsNumerator.z/scaleParamsDenominator.z);
    return scaleParams;

}

void Procrustes::scale(QVector<cv::Point3d> &points, cv::Point3d scaleParams)
{
    int n = points.count();
    for (int i = 0; i < n; i++)
    {
        points[i].x = points[i].x * scaleParams.x;
        points[i].y = points[i].y * scaleParams.y;
        points[i].z = points[i].z * scaleParams.z;
    }
}

void Procrustes::translate(QVector<cv::Point3d> &points, cv::Point3d shift)
{
    int n = points.count();
    for (int i = 0; i < n; i++)
    {
        points[i] += shift;
    }
}

Procrustes3DResult Procrustes::SVDAlign(QVector<QVector<cv::Point3d> > &vectorOfPointclouds)//, bool centralize, double eps, int maxIterations)
{
    Procrustes3DResult result;

    int numberOfPointclouds = vectorOfPointclouds.count();
    assert(numberOfPointclouds > 0);
    int numberOfPoints = vectorOfPointclouds[0].count();
    assert(numberOfPoints > 0);

    result.translations = QVector<cv::Point3d>(numberOfPointclouds, cv::Point3d(0,0,0));
    result.scaleParams = QVector<cv::Point3d>(numberOfPointclouds, cv::Point3d(1,1,1));
    result.rotations = QVector<Matrix>(numberOfPointclouds, Matrix::eye(3,3));

    QVector<cv::Point3d> mean = getMeanShape(vectorOfPointclouds);
    //double variation = getShapeVariation(vectorOfPointclouds, mean);
    //qDebug() << "SVDAlign input variation:" << variation;

    for (int i = 0; i < numberOfPointclouds; i++)
    {
        QVector<cv::Point3d> &pointcloud = vectorOfPointclouds[i];

        Matrix R = getOptimalRotation(pointcloud, mean);
        transform(pointcloud, R);
        result.rotations[i] = R; // * result.rotations[i];
    }

    //variation = getShapeVariation(vectorOfPointclouds, mean);
    //qDebug() << "SVDAlign result variation:" << variation;

    return result;
}

QVector<cv::Point3d> Procrustes::getMeanShape(QVector<QVector<cv::Point3d> > &vectorOfPointclouds)
{
    int numberOfPointclouds = vectorOfPointclouds.count();
    int numberOfPoints = vectorOfPointclouds[0].count();
    QVector<cv::Point3d> mean(numberOfPoints, cv::Point3d(0,0,0));

    for (int i = 0; i < numberOfPointclouds; i++)
    {
        QVector<cv::Point3d> &pointcloud = vectorOfPointclouds[i];
        for (int j = 0; j < numberOfPoints; j++)
        {
            mean[j] = mean[j] + pointcloud[j];

            /*if (j == 0)
                qDebug() << mean[j].x << mean[j].y << mean[j].z;*/
        }
    }

    for (int j = 0; j < numberOfPoints; j++)
    {
        mean[j].x /= numberOfPointclouds;
        mean[j].y /= numberOfPointclouds;
        mean[j].z /= numberOfPointclouds;
    }

    return mean;
}

double Procrustes::getShapeVariation(QVector<QVector<cv::Point3d> > &vectorOfPointclouds, QVector<cv::Point3d> &mean)
{
    int numberOfPointclouds = vectorOfPointclouds.count();
    int numberOfPoints = vectorOfPointclouds[0].count();

    //QVector<cv::Point3d> variation(numberOfPoints, cv::Point3d(0,0,0));
    double variation = 0;
    for (int i = 0; i < numberOfPointclouds; i++)
    {
        QVector<cv::Point3d> &pointcloud = vectorOfPointclouds[i];
        for (int j = 0; j < numberOfPoints; j++)
        {
            cv::Point3d diff = mean[j] - pointcloud[j];
            variation += (diff.x*diff.x);
            variation += (diff.y*diff.y);
            variation += (diff.z*diff.z);
        }
    }

    variation = variation / (3*numberOfPointclouds*numberOfPoints);
    return variation;
}