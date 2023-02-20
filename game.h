#ifndef GAME_H
#define GAME_H

#define _WIN32_WINNT 0x0A00
#include <windows.h>

#define AddEnemy(a,b) ObjectInit(NewObject(), a, b, 8, 8, 'e') // ������ �������� ������

// ��������� ��������
typedef struct SPoint{
    float x, y;
} TPoint;


// ������
typedef struct SObject{
    TPoint pos;
    TPoint size;
    COLORREF brush;
    TPoint speed;
    char oType;
    BOOL isDel;
    //BOOL sprite;
} TObject, *PObject;


#endif // GAME_H
