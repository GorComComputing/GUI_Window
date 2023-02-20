#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define _WIN32_WINNT 0x0A00
#include <windows.h>

#include "GUI.h"
#include "display.h"
#include "BGL.h"





// Тест заполнения рандом линиями ЦДА
void TestLineDDAgl(){
    SetColor(rand()%(256*256*256));
    LineDDAgl(rand()%BITMAP_WIDTH, rand()%BITMAP_HEIGHT, rand()%BITMAP_WIDTH, rand()%BITMAP_HEIGHT);
    Sleep(5);
}


// Тест заполнения рандом линиями Брезенхема
void TestLine(){
    SetColor(rand()%(256*256*256));
    //LinePP(rand()%BITMAP_WIDTH, rand()%BITMAP_HEIGHT, rand()%BITMAP_WIDTH, rand()%BITMAP_HEIGHT);
    //Sleep(5);

    /*for(int y = 0; y < BITMAP_HEIGHT; y++){
        SetColor(rand()%(256*256*256));
        HLine(BITMAP_WIDTH/2, y, BITMAP_WIDTH*3/4);
    }*/

    /*SetColor(0x00FF00);
    Circle(BITMAP_WIDTH/2, BITMAP_HEIGHT/2, 100);
    Circle(BITMAP_WIDTH/2-30, BITMAP_HEIGHT/2, 100);
    SetColor(0x0000FF);
    FloodFillgl(BITMAP_WIDTH/2, BITMAP_HEIGHT/2, 0x00FF00);*/

    struct tPoint p[10];
    //tPoly p;
    //MakeStar(100, 250, 5, p);
    /*double r1 = 10;
    double r2 = 25;
    int n = 5;

    double df = 2*3.14/(float)n;
    double f1 = 3.14/2.;
    double f2 = f1 + df/2.;
    for(int i = 0; i < n - 1; i++){
        p[2*i].x = round(r1*cos(f1));
        printf("x: %d ", p[2*i].x);
        p[2*i].y = round(r1*sin(f1));
        printf("y: %d ", p[2*i].y);
        p[2*i + 1].x = round(r2*cos(f2));
        printf("x: %d ", p[2*i + 1].x);
        p[2*i + 1].y = round(r2*sin(f2));
        printf("y: %d ", p[2*i + 1].y);
        f1 += df;
        f2 += df;
    }

    for(int i = 0; i < 10; i++){
        p[i].x = p[i].x + BITMAP_WIDTH/2;
        p[i].y = p[i].y + BITMAP_HEIGHT/2;
    }

    printf("\n");
    for(int i = 0; i < n - 1; i++){
        printf("x: %d ", p[2*i].x);
        printf("y: %d ", p[2*i].y);
        printf("x: %d ", p[2*i + 1].x);
        printf("y: %d ", p[2*i + 1].y);
    }


    SetColor(0x00FF00);
    FillPoly(10, p);*/
    //SetLength(p, 4);
    /*p[0].x = 300;
    p[0].y = 200;

    p[1].x = 400;
    p[1].y = 100;

    p[2].x = 400;
    p[2].y = 500;

    p[3].x = 200;
    p[3].y = 400;

    FillPoly(4, p);*/



    p[0].x = 400;
    p[0].y = 100;

    p[1].x = 450;
    p[1].y = 150;

    p[2].x = 600;
    p[2].y = 150;

    p[3].x = 450;
    p[3].y = 200;

    p[4].x = 600;
    p[4].y = 400;

    p[5].x = 400;
    p[5].y = 300;

    p[6].x = 200;
    p[6].y = 400;

    p[7].x = 350;
    p[7].y = 200;

    p[8].x = 200;
    p[8].y = 150;

    p[9].x = 350;
    p[9].y = 150;

    FillPoly(10, p);

    Sleep(2000);
}


// Рисует звезду
void MakeStar(double r1, double r2, int n, struct tPoint s[]){
    double df = 2*3.14/(float)n;
    double f1 = 3.14/2.;
    double f2 = f1 + df/2.;
    for(int i = 0; i < n - 1; i++){
        s[2*i].x = round(r1*cos(f1));
        s[2*i].y = round(r1*sin(f1));
        s[2*i + 1].x = round(r2*cos(f2));
        s[2*i + 1].y = round(r2*sin(f2));
    }
}


// Тест заполнения рандом линиями Брезенхема по таймеру
void TestLineTime(HDC dc){
    time_t start = time(NULL); // Текущее время в секундах
    for(int i = 0; i < 1000000; i++){


        SetColor(0x00FF00);
        Circle(BITMAP_WIDTH/2, BITMAP_HEIGHT/2, 100);
        Circle(BITMAP_WIDTH/2-30, BITMAP_HEIGHT/2, 100);
        SetColor(0x0000FF);
        FloodFillgl(BITMAP_WIDTH/2, BITMAP_HEIGHT/2, 0x00FF00);

        //SetColor(rand()%(256*256*256));
        //LinePP(rand()%BITMAP_WIDTH, rand()%BITMAP_HEIGHT, rand()%BITMAP_WIDTH, rand()%BITMAP_HEIGHT);
    }
    char timeStr[10];
    SetColor(0xFFFFFF);
    itoa((time(NULL) - start), timeStr, 10); // Преобразуем число в строку
    TextOutgl(timeStr, 0, 0, 2);
    Draw(dc);
    Sleep(5000);
}


// Фрактал Коха
void DrawKoch(int depth, int x1, int y1, int angle, int length){
    SetColor(rand()%(256*256*256));
    if(depth == 0){
        LinePP(x1, y1, (int)(x1+length*cos(angle*(3.14/180))), (int)(y1+length*sin(angle*(3.14/180))));
    }
    else{
        DrawKoch(depth - 1, x1, y1, angle, length/3);

        x1 = (int)(x1+length*cos((angle-60)*(3.14/180)));
        y1 = (int)(y1+length*sin((angle-60)*(3.14/180)));
        DrawKoch(depth - 1, x1, y1, angle - 60, length/3);

        x1 = (int)(x1+length*cos((angle+60)*(3.14/180)));
        y1 = (int)(y1+length*sin((angle+60)*(3.14/180)));
        DrawKoch(depth - 1, x1, y1, angle + 60, length/3);

        x1 = (int)(x1+length*cos((angle)*(3.14/180)));
        y1 = (int)(y1+length*sin((angle)*(3.14/180)));
        DrawKoch(depth - 1, x1, y1, angle, length/3);
    }
}


// Выводит битмап девушки
void LadyBitmap(int xstart, int ystart, int sizeX, int sizeY, int scale){
    char bitmap[] = \
    "aaaaaaaaaaaaaaccffffffffffffffffffffffffffffffffff"
    "aaaaaaaaaaaaaaccffffffffffffffffffffffffffffffffff"
    "aaaaaaaaaaaaaaaccfffffffffffffffffffffffffffffffff"
    "aaaaaaaaaaaaaaaacfffffffffffffffffffffffffffffffff"
    "aaaaaaaaaaaaaaaaccffffffffffffffffffffffffffffffff"
    "aaaaaaaaaaaaaaaaacfffffbfbbfffffffffffffffffffffff"
    "ggaaaaaaaaajjaaaaccfffbababaaaffffffffffffffffffff"
    "ggggaaaaaaabjjaaaaccffaaaaaaaaaaaabbffffffffffffff"
    "ggggggaaaaaaabjaaaacffcaaaaaaaaaaaaaabffffffffffff"
    "gggggggaaaaabbbaaaaffffcbbbbbccccccfaaabffffffffff"
    "gggggggaabaaaccaaacffffcccbbaaaaccccffbbbbffffffff"
    "gggggggabbcccccaacffffffcccbbbbbaacccffffbbbffffff"
    "gggggggabccffcaaacffffffccbaaaaabbaaccffffffffffff"
    "hggggggabcfffcaacffffffffcaaajjaaabbaccfffffffffff"
    "hhhggggabcffcaaacffffffffcbbjjjhajaabaccffffffffff"
    "hhhhhggabcfcaaacfffffffffcbbjjjhhahaabaccfffffffff"
    "hhhhhhhabcfcaaacfffffffffbcbbjjjhhhjaabbcccfffffff"
    "hhhhhhhabccaaacfffffffffffccbbbjjjjjjabbbcffffffff"
    "hhhhhhhabccaacfffffffffffffccbbbbbbbbcabbcffffffff"
    "hhhhhhhabbaaacffffffffffffffcccbbbcccccbffffffffff"
    "hhhhhhhabaaacfffffffffffffffffcccfffffffffffffffff"
    "ghhhhhhhaaacffffffffffffffffffffffffffffffffffffff"
    "ghhhhhhhaaacffffffffffffffffffffffffffffffffffffff"
    "gghhhhhhaacfffffffffffffffffffffffffffffffffffffff"
    "gghhhhhhaacfffffffffffffffffffffffffffffffffffffff"
    "gghhhhhhaaacffffffffffffffffffffffffffffffffffffff"
    "gghhhhhhaaaacffbaafffcfffffffffffffffffffffffffffc"
    "gghhhhhhaaaaabbfffaacfffffffffffffffffffffffffffcc"
    "gghhhhhhaaccffcfffffffffffffffffffffffffffffffcccc"
    "ggghhhhhaaaffcfffffffffffffffffffffffffffffffffffc"
    "ggghhhhhaaaafffffffffffffffffffffffffffffffffffffc"
    "ggghhhhhaaaaddffffffffffffffffffffffffffffffffffff"
    "ggghhhhhadaaadddffffffffffffffffffffffffffffffffff"
    "ggghhhhhadddaadddffffffffffffffffffffffffffffffffc"
    "ggghhhhhadddddaadddfffffffffffffffffffffffffffffcc"
    "gghhhhhhaaddddddaaddfffffffffffffffffffffffffffccf"
    "ghhhhhhhaaddddddeeedddfffffffffffffffffffffffffccf"
    "ghhhhhhhaaaddddeeeeeeedfffffffffffffffffffffffccff"
    "ghhhhhhhaaaaadddeeeffffffffffffffffffffffffffccfff"
    "ghhhhhhhaaccaacfffffffffffffffffffffffffffffccffff"
    "ghhhhhhhaccffccffffffffffffffffffffffffffffccfffff"
    "ghhhhhhhaccffffcffffffffffffffffffffffffffccffffff"
    "gghhhhhhaccffffffffffffffffffffffffffffccccfffffff"
    "ggghhhhhhacccffffffffffffffffffffffffcccffffffffff"
    "gggghhhhhaccccffffffffffffffffffcccaaccfffffffffff"
    "agggghhhhhaccccffffffffffffffcccaaaccccfffffffffff"
    "aaggggghhhhaacccffffffffffcccaaabbcccfffffffffffff"
    "aaaggggghhhhhaacccccccccccaaaaaaabbcccffffffffffff"
    "aaaaggggghhhhhhaaaaaaaaaaaaaaaaaabbcccffffffffffff"
    "aaaaaaggggghhhhhhgaaaaaaaaaaaaaaaabccccfffffffffff"
    ;

    int x = xstart;
    int y = ystart;

    for(int i = 0; i < sizeY; i++){
        for(int scaleY = scale; scaleY > 0; scaleY--){
        for(int j = 0; j < sizeX; j++){
            if(bitmap[i*sizeX + j] == 'a'){
                for(int scaleX = scale; scaleX > 0; scaleX--){
                PutPixel(x + j, y + i, 0x201E1C);
                if(scaleX > 1) x++;
                }
            }
            else if(bitmap[i*sizeX + j] == 'b'){
                for(int scaleX = scale; scaleX > 0; scaleX--){
                PutPixel(x + j, y + i, 0x5E6368);
                if(scaleX > 1) x++;
                }
            }
            else if(bitmap[i*sizeX + j] == 'c'){
                for(int scaleX = scale; scaleX > 0; scaleX--){
                PutPixel(x + j, y + i, 0xADB0B5);
                if(scaleX > 1) x++;
                }
            }
            else if(bitmap[i*sizeX + j] == 'd'){
                for(int scaleX = scale; scaleX > 0; scaleX--){
                PutPixel(x + j, y + i, 0x7D0428);
                if(scaleX > 1) x++;
                }
            }
            else if(bitmap[i*sizeX + j] == 'e'){
                for(int scaleX = scale; scaleX > 0; scaleX--){
                PutPixel(x + j, y + i, 0xF14A64);
                if(scaleX > 1) x++;
                }
            }
            else if(bitmap[i*sizeX + j] == 'f'){
                for(int scaleX = scale; scaleX > 0; scaleX--){
                PutPixel(x + j, y + i, 0xF3EBCC);
                if(scaleX > 1) x++;
                }
            }
            else if(bitmap[i*sizeX + j] == 'g'){
                for(int scaleX = scale; scaleX > 0; scaleX--){
                PutPixel(x + j, y + i, 0x003B8F);
                if(scaleX > 1) x++;
                }
            }
            else if(bitmap[i*sizeX + j] == 'h'){
                for(int scaleX = scale; scaleX > 0; scaleX--){
                PutPixel(x + j, y + i, 0x83A9FA);
                if(scaleX > 1) x++;
                }
            }
            else if(bitmap[i*sizeX + j] == 'j'){
                for(int scaleX = scale; scaleX > 0; scaleX--){
                PutPixel(x + j, y + i, 0xFEFEFE);
                if(scaleX > 1) x++;
                }
            }
        }
        x = xstart;
        if(scaleY > 1) y++;
        }
    }
}


