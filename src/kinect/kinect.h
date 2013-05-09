#ifndef KINECT_H_
#define KINECT_H_

#include "linalg/common.h"
#include "facelib/mesh.h"
#include "facelib/map.h"

class Kinect
{
private:
    static const int n = 640*480;
    static const double alpha = 0.00307110156374;
    static const double beta = 3.33094951605675;
    static const double mindistance = -10;
    static const double scaleFactor = 0.0021;

public:
    static bool getDepth(double *depth, bool *mask = NULL, double minDistance = 200, double maxDistance = 2500);

    static bool getDepth(double *depth, int scansCount, bool *mask = NULL, double minDistance = 200, double maxDistance = 2500);

    static bool getRGB(uint8_t *rgb);

    static bool getRGB(uint8_t *rgb, int scansCount);

    static VectorOfPoints depthToVectorOfPoints(double *depth);

    static Mesh createMesh(double *depth, uint8_t *rgb);

    static ImageBGR RGBToColorMatrix(uint8_t *rgb);

    static ImageGrayscale RGBToGrayscale(uint8_t *rgb);

    static Mesh scanFace(int scanIterations);

    static void PCL()
    {

    }
};

#endif /* KINECT_H_ */
