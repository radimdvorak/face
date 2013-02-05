#include "morphable3dfacemodel.h"

#include "linalg/procrustes.h"
#include "facelib/surfaceprocessor.h"
#include "landmarks.h"

Morphable3DFaceModel::Morphable3DFaceModel(const QString &pcaPath, const QString &maskPath,
                                           const QString &landmarksPath, int width)
{
    pca = PCA(pcaPath);
    landmarks = Landmarks(landmarksPath);

    mask = Vector::fromFile(maskPath);
    int maskRows = mask.rows;
    Map faceDepth(width, maskRows/width);
    for (int i = 0; i < maskRows; i++)
    {
        faceDepth.flags[i] = mask(i) ? true : false;
    }

    Matrix mean = pca.getMean();
    assert(mean.rows == faceDepth.flags.count(1));
    int depthIndex = 0;
    for (int i = 0; i < maskRows; i++)
    {
        if (faceDepth.flags[i])
        {
            faceDepth.values[i] = mean(depthIndex);
            depthIndex++;
        }
    }

    mesh = Mesh::fromMap(faceDepth, true);
}

void Morphable3DFaceModel::setModelParams(Matrix &params)
{
    Matrix newValuesMatrix = pca.backProject(params);
    int n = newValuesMatrix.rows;
    assert(n == mesh.points.count());

    for (int i = 0; i < n; i++)
    {
        mesh.points[i].z = newValuesMatrix(i);
    }
}

void Morphable3DFaceModel::align(Mesh &inputMesh, Landmarks &inputLandmarks, int iterations)
{
    // centralize
    cv::Point3d shift = Procrustes::centralizedTranslation(inputLandmarks.points);
    Procrustes::translate(inputLandmarks.points, shift);
    inputMesh.move(shift);

    for (int iteration = 0; iteration < iterations; iteration++)
    {
        // rotate
        Matrix rotation = Procrustes::getOptimalRotation(inputLandmarks.points, this->landmarks.points);
        Procrustes::transform(inputLandmarks.points, rotation);
        inputMesh.transform(rotation);

        // scale
        cv::Point3d scaleParams = Procrustes::getOptimalScale(inputLandmarks.points, this->landmarks.points);
        Procrustes::scale(inputLandmarks.points, scaleParams);
        inputMesh.scale(scaleParams);
    }
}

void Morphable3DFaceModel::morphModel(Mesh &alignedMesh)
{
    MapConverter converter;
    Map depthmap = SurfaceProcessor::depthmap(alignedMesh, converter, cv::Point2d(-100,-100), cv::Point2d(100,100), 1);
    for (int i = 0; i < mask.rows; i++)
    {
        if (mask(i) == 0)
        {
            depthmap.flags[i] = false;
        }
        else if (!depthmap.flags[i])
        {
            qDebug() << "Morphable3DFaceModel::morphModel, Missing value in input aligned mesh";
            //TODO!
            //depthmap.flags[i] = true;
            //depthmap.values[i] = ???
        }
    }

    QVector<double> usedValues = depthmap.getUsedValues();
    Matrix inputValues = Vector::fromQVector(usedValues);
    Matrix params = pca.project(inputValues);
    Matrix normalizedParams = pca.normalizeParams(params);
    setModelParams(normalizedParams);
}

void Morphable3DFaceModel::align(QVector<Mesh> &meshes,
                                 QVector<VectorOfPoints> &controlPoints,
                                 int iterations)
{
    int meshCount = meshes.count();

    // centralize
    for (int i = 0; i < meshCount; i++)
    {
        VectorOfPoints &pointcloud = controlPoints[i];
        cv::Point3d shift = Procrustes::centralizedTranslation(pointcloud);

        Procrustes::translate(pointcloud, shift);
        meshes[i].move(shift);
    }

    VectorOfPoints meanShape = Procrustes::getMeanShape(controlPoints);
    qDebug() << "Initial variation:" << Procrustes::getShapeVariation(controlPoints, meanShape);

    for (int iteration = 0; iteration < iterations; iteration++)
    {
        // rotate
        Procrustes3DResult rotResult = Procrustes::SVDAlign(controlPoints);
        for (int i = 0; i < meshCount; i++)
        {
            meshes[i].transform(rotResult.rotations[i]);
        }
        meanShape = Procrustes::getMeanShape(controlPoints);
        qDebug() << "Iteration:" << iteration << "after SVD:" << Procrustes::getShapeVariation(controlPoints, meanShape);

        // scale
        for (int meshIndex = 0; meshIndex < meshCount; meshIndex++)
        {
            cv::Point3d scaleParams = Procrustes::getOptimalScale(controlPoints[meshIndex], meanShape);
            Procrustes::scale(controlPoints[meshIndex], scaleParams);
            meshes[meshIndex].scale(scaleParams);
        }

        meanShape = Procrustes::getMeanShape(controlPoints);
        qDebug() << "Iteration:" << iteration << "after scaling:" << Procrustes::getShapeVariation(controlPoints, meanShape);
    }
}

void Morphable3DFaceModel::create(QVector<Mesh> &meshes, QVector<VectorOfPoints> &controlPoints, int iterations,
                                  const QString &pcaFile, const QString &flagsFile, const QString &meanControlPointsFile,
                                  Map &depthMapMask)
{
    align(meshes, controlPoints, iterations);
    VectorOfPoints meanControlPoints = Procrustes::getMeanShape(controlPoints);
    Landmarks l(meanControlPoints);
    l.serialize(meanControlPointsFile);

    QVector<Map> depthMaps;
    Map resultMap(depthMapMask.w, depthMapMask.h);
    resultMap.setAll(0);
    resultMap.add(depthMapMask);
    for (int index = 0; index < meshes.count(); index++)
    {
        Mesh &mesh = meshes[index];
        MapConverter converter;
        Map depth = SurfaceProcessor::depthmap(mesh, converter,
                                               cv::Point2d(-depthMapMask.w/2, -depthMapMask.h/2),
                                               cv::Point2d(depthMapMask.w/2, depthMapMask.h/2),
                                               1);
        resultMap.add(depth);
        depthMaps.append(depth);

        if (index == 0)
        {
            cv::imshow("First Aligned mesh", depth.toMatrix());
            cv::waitKey(0);
        }
    }

    resultMap.linearTransform(1.0/meshes.count(), 1.0);
    //Matrix resultMatrix = resultMap.toMatrix() * 255;
    //cv::imwrite(meanImageFile.toStdString(), resultMatrix);

    QVector<Matrix> matrices;
    for (int index = 0; index < meshes.count(); index++)
    {
        Map &depth = depthMaps[index];
        SurfaceProcessor::smooth(depth, 1, 2);
        depth.flags = resultMap.flags;

        QVector<double> values = depth.getUsedValues();
        Matrix valuesMatrix = Vector::fromQVector(values);
        matrices << valuesMatrix;
    }

    PCA pca(matrices);
    pca.serialize(pcaFile);

    int n = resultMap.flags.count();
    QVector<double> flags(n, 0.0);
    for (int i = 0; i < n; i++)
    {
        if (resultMap.flags[i])
        {
            flags[i] = 1.0;
        }
    }
    Matrix flagsMatrix = Vector::fromQVector(flags);
    Vector::toFile(flagsMatrix, flagsFile);
}