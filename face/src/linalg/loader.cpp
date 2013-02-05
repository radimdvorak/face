#include "loader.h"

QVector<Matrix> Loader::loadShapes(const QString &dirPath)
{
    QVector<Matrix> shapes;
    QDir dir(dirPath);
    dir.setFilter(QDir::Files | QDir::NoDotAndDotDot);
    QStringList nameFilter;
    nameFilter << "*.shape";
    dir.setNameFilters(nameFilter);
    QStringList filenames = dir.entryList();
    filenames.sort();

    for (int i = 0; i < filenames.count(); i++)
    {
        qDebug() << "loading" << filenames[i];
        Matrix v = Vector::fromTwoColsFile(dirPath + QDir::separator() + filenames[i]);
        shapes.append(v);
    }

    return shapes;
}

QVector<Matrix> Loader::loadImages(const QString &dirPath)
{
    QVector<Matrix> images;
    QDir dir(dirPath);
    dir.setFilter(QDir::Files | QDir::NoDotAndDotDot);
    QStringList nameFilter;
    nameFilter << "*.png";
    dir.setNameFilters(nameFilter);
    QStringList filenames = dir.entryList();
    filenames.sort();

    for (int i = 0; i < filenames.count(); i++)
    {
        qDebug() << "loading" << filenames[i];

        QString imgPath(dirPath + QDir::separator() + filenames[i]);
        Matrix img = MatrixConverter::imageToMatrix(imgPath);
        images.append(img);
    }

    return images;
}

void Loader::loadImages(const QString &dirPath, QVector<Matrix> &images, QVector<int> *classes,
                        const char *extensionFilter, bool convertToColumnVectors,
                        const char *classSeparator, bool qdebug)
{
    qDebug() << "loading files from" << dirPath;

    QDir dir(dirPath);
    dir.setFilter(QDir::Files | QDir::NoDotAndDotDot);
    QStringList nameFilter;
    nameFilter << extensionFilter;
    dir.setNameFilters(nameFilter);
    QStringList filenames = dir.entryList();
    filenames.sort();

    for (int i = 0; i < filenames.count(); i++)
    {
        if (qdebug)
            qDebug() << "loading" << filenames[i];

        QString imgPath(dirPath + QDir::separator() + filenames[i]);
        Matrix img = MatrixConverter::imageToMatrix(imgPath);
        if (!convertToColumnVectors)
            images.append(img);
        else
        {
            Matrix img2 = MatrixConverter::matrixToColumnVector(img);
            images.append(img2);
        }

        if (classes)
        {
            int indexOfSeparator = filenames.at(i).indexOf(classSeparator);
            QString classString = filenames.at(i).left(indexOfSeparator);
            int classNumber = classString.toInt();
            classes->append(classNumber);
        }
    }
}

void Loader::loadVectors(const QString &dirPath, QVector<Matrix> &vectors,
                         QVector<int> &classes, const char *classSeparator, const char *nameFilter)
{
    //qDebug() << "loading vectors from" << dirPath;
    vectors.clear();
    classes.clear();

    QStringList filenames = listFiles(dirPath, nameFilter);
    for (int i = 0; i < filenames.count(); i++)
    {
        QString fullpath(dirPath + QDir::separator() + filenames[i]);
        Matrix vec = Vector::fromFile(fullpath);
        vectors.append(vec);

        int indexOfSeparator = filenames.at(i).indexOf(classSeparator);
        QString classString = filenames.at(i).left(indexOfSeparator);
        int classNumber = classString.toInt();
        classes.append(classNumber);
    }
}

QStringList Loader::listFiles(const QString &path, const QString &filter, bool fullPath)
{
    QDir dir(path);
    dir.setFilter(QDir::Files | QDir::NoDotAndDotDot);
    QStringList nameFilter;
    nameFilter << filter;
    dir.setNameFilters(nameFilter);
    QStringList filenames = dir.entryList();
    filenames.sort();

    if (fullPath)
    {
        for (int i = 0; i < filenames.count(); i++)
            filenames[i] = path + QDir::separator() + filenames[i];
    }

    return filenames;
}