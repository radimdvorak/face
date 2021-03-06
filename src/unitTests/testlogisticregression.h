#ifndef TESTLOGISTICREGRESSION_H
#define TESTLOGISTICREGRESSION_H

#include "linalg/logisticregression.h"

class TestLogisticRegression
{
public:
    static void test()
    {
        QVector<Vector> data;
        QVector<int> labels;
        for (int x = 0; x <= 10; x++)
        {
            for (int y = 0; y <= 10; y++)
            {
                if (x == y) continue;

                QVector<double> values;
                values << x << y;
                data << Vector(values);

                labels << ((x < y) ? 1 : 0);
            }
        }

        LogisticRegression logR;
        logR.learn(data, labels);

        for (double x = 0; x <= 10; x += 0.5)
        {
            for (double y = 0; y <= 10; y += 0.5)
            {
                QVector<double> values;
                values << x << y;
                Vector probe(values);

                qDebug() << x << y << logR.classify(probe);
            }
            qDebug() << "";
        }
    }
};

#endif // TESTLOGISTICREGRESSION_H
