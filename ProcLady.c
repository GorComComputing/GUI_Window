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
void onFormClick3(){
    Beep(700, 50);
    exitProcess(PID);
}


// получает ID своего процесса
void getPIDLady(int ID){
    PID = ID;
    form = formCreate(PID, "LADY", 30, 300, 104, 103 + 15, STANDART, NULL, NULL, onFormClick3);
    Form1 = *mas_window[form];
}


// завершение процесса
void exitLady(){
    Beep(500, 50);
    formClose(form);
    Beep(300, 200);
}


// главный цикл приложения
void ProcLady(){
    LadyBitmap(Form1.x + 2, Form1.y + 1 + 15, 50, 50, 2);
}
