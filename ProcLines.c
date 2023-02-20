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


// событие по клику по окну
void onFormClickLines(){
    Beep(700, 50);
    exitProcess(PID);
}


// получает ID своего процесса
void getPIDLines(int ID){
    PID = ID;
    form = formCreate(PID, "LINES", 500, 100, 104, 103 + 15, STANDART, NULL, NULL, onFormClickLines);
    Form1 = *mas_window[form];
}


// завершение процесса
void exitLines(){
    Beep(500, 50);
    formClose(form);
    Beep(300, 200);
}


// главный цикл приложения
void ProcLines(){
    SetColor(rand()%(256*256*256));
    LineDDAgl(Form1.x + 2 + rand()%(Form1.width - 4), Form1.y + 1 + 15 + rand()%(Form1.height - 17), Form1.x + 2 + rand()%(Form1.width - 4), Form1.y + 1 + 15 + rand()%(Form1.height - 17));
}
