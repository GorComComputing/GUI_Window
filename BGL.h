#ifndef BGL_H
#define BGL_H

#include "display.h"

typedef struct tPoint{
    int x;
    int y;
} tPoint;

//typedef  struct tPoint tPoly[100];

#define NMAX 100    // ���������� ����� ����������� � ��������
typedef struct tXbuf{
    int m;          // ���������� ����� �������
    int x[NMAX];    // ���������� ����� �������
} tXbuf;



#endif // BGL_H
