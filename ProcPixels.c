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

//extern int xPos;
//extern int yPos;


// событие по клику по окну
void onFormClick2(){
    Beep(700, 50);
    exitProcess(PID);
}


// получает ID своего процесса
void getPIDPixels(int ID){
    PID = ID;
    form = formCreate(PID, "PIXELS", 50, 150, 104, 103 + 15, STANDART, NULL, NULL, onFormClick2);
    Form1 = *mas_window[form];
}


// завершение процесса
void exitPixels(){
    Beep(500, 50);
    formClose(form);
    Beep(300, 200);
}


// главный цикл приложения
void ProcPixels(){
    for(int i = 2; i < Form1.width - 2; i++)
        for(int j = 17; j < Form1.height - 2; j++)
            PutPixel(Form1.x + i, Form1.y + j, rand()%(256*256*256));
}
