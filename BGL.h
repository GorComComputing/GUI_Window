#ifndef BGL_H
#define BGL_H

#include "display.h"

typedef struct tPoint{
    int x;
    int y;
} tPoint;

//typedef  struct tPoint tPoly[100];

#define NMAX 100    // количество точек пересечения с контуром
typedef struct tXbuf{
    int m;          // количество точек контура
    int x[NMAX];    // координаты точек контура
} tXbuf;



#endif // BGL_H
