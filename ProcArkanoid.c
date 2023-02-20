#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define _WIN32_WINNT 0x0A00
#include <windows.h>

#include "GUI.h"
#include "BGL.h"




static TWindow Form1;
static int form = 0;
static int PID = 0;
#define MAX_WINDOW 10
extern TWindow* mas_window[MAX_WINDOW];

#define WDTH 65
#define HGHT 25

char masPole[HGHT*WDTH] = {0}; // Игровое поле HGHT*WDTH
char Wall[20*20*3] = {0}; // спрайт стены
char Racket1[20*20*3] = {0}; // спрайт ракетки
char Racket2[20*20*3] = {0}; // спрайт ракетки
char Racket3[20*20*3] = {0}; // спрайт ракетки

typedef struct{
    int x;
    int y;
    int w;
} TRacket;

TRacket racket;


// инициализация ракетки
void initRacket(){
    racket.w = 7;
    racket.x = (WDTH - racket.w)/2;
    racket.y = HGHT - 1;
}


// показывает ракетку на экране
void PutRacket(){
    masPole[racket.y*WDTH + racket.x] = '1';
    for(int i = racket.x + 1; i < racket.x + racket.w - 1; i++)
        masPole[racket.y*WDTH + i] = '@';
    masPole[racket.y*WDTH + racket.x + racket.w - 1] = '2';
}


// Инициализация игрового поля
void init(){
        for(int j = 0; j < WDTH; j++)
            masPole[j] = '#';

        for(int i = 0; i < HGHT; i++){
            masPole[i*WDTH] = '#';
            masPole[i*WDTH+WDTH-1] = '#';
        }

        for(int j = 1; j < WDTH - 1; j++)
            masPole[(HGHT - 1)*WDTH + j] = '\0';
}


// показать игровое поле
void show(int x, int y){
    struct tPoint p[4];
    SetColor(0x3E1AB2);

    p[0].x = Form1.x + 2;
    p[0].y = Form1.y + 17;

    p[1].x = Form1.x + WDTH*20;
    p[1].y = Form1.y + 17;

    p[2].x = Form1.x + WDTH*20;
    p[2].y = Form1.y + 17 + HGHT*20;

    p[3].x = Form1.x + 2;
    p[3].y = Form1.y + 17 + HGHT*20;

    FillPoly(4, p);


    for(int Mi = 0; Mi < HGHT; Mi++){
        for(int Mj = 0; Mj < WDTH; Mj++){
            if(masPole[Mi*WDTH+Mj] == '#')
                showBMP(Wall, x + Mj*20, y + Mi*20, 20, 20);

            else if(masPole[Mi*WDTH+Mj] == '@')
                showBMP(Racket2, x + Mj*20, y + Mi*20, 20, 20);

            else if(masPole[Mi*WDTH+Mj] == '1')
                showBMP(Racket1, x + Mj*20, y + Mi*20, 20, 20);

            else if(masPole[Mi*WDTH+Mj] == '2')
                showBMP(Racket3, x + Mj*20, y + Mi*20, 20, 20);

        }
    }
}


// Перемещение ракетки
void moveRacket(int x){
    racket.x = x;
    if(racket.x < 1) racket.x = 1;
    if(racket.x + racket.w > WDTH - 1) racket.x = WDTH - 1 - racket.w;
}


// Обработчик клавиш
void PlayerControlArkanoid(){
    if(GetKeyState(VK_LEFT) < 0) moveRacket(racket.x - 1);
    if(GetKeyState(VK_RIGHT) < 0) moveRacket(racket.x + 1);
}


// событие по клику по окну
void onFormClick(){
    Beep(700, 50);
    exitProcess(PID);
}


// инициализация игры
void initArkanoid(){
    loadBMP(Wall, "bmp\\arkanoid\\Wall.bmp");
    loadBMP(Racket1, "bmp\\arkanoid\\Racket1.bmp");
    loadBMP(Racket2, "bmp\\arkanoid\\Racket2.bmp");
    loadBMP(Racket3, "bmp\\arkanoid\\Racket3.bmp");

    initRacket();
}


// завершение процесса
void exitArkanoid(){
    Beep(500, 50);
    formClose(form);
    Beep(300, 200);
}


// получает ID своего процесса
void getPIDArcanoid(int ID){
    PID = ID;
    form = formCreate(PID, "ARKANOID", 250, 250, 80*20+4, 25*20+4+15, STANDART, NULL, NULL, onFormClick);
    Form1 = *mas_window[form];
}


// Главный цикл
void ProcArkanoidLoop(){
    init();

    PutRacket();

    show(Form1.x + 2, Form1.y + 15);
    PlayerControlArkanoid();
}
