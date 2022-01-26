/**
 *  Library to perfom homography transforms
 *  Author: C Deenen
 */


#ifndef HOMOGRAPHY_H
#define HOMOGRAPHY_H

#include "Arduino.h"
#include "../MatrixMath/MatrixMath.h"
#include "math.h"

    class homography
    {
        public:
            homography();
            void calculateHomographyMatrix();
            void calculateCoordinates(double, double);
            double getX();
            double getY();
            void setCalArray(int,int,int);
            int getCalArray(int, int);
            void orderCalArray();

        private:
            uint8_t findMax(uint16_t ARRAY[]);
            mtx_type _H[3][3];
            double _x;
            double _y;
            uint16_t _bounds[2][4];
            int16_t _calArray[2][4];
    };
#endif
