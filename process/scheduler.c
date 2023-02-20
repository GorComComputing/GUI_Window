#include <stdio.h>
#include <stdlib.h>

#define _WIN32_WINNT 0x0A00
#include <windows.h>

#include "..\GUI.h"
#include "..\display.h"
#include "..\BGL.h"


extern HDC dc;

void ProcPixels();
void exitPixels();

void ProcLady();
void exitLady();

void ProcLines();
void exitLines();

void ProcKoch();
void exitKoch();

void initArkanoid();
void ProcArkanoidLoop();
void exitArkanoid();

void initGUI();
void DrawWellpaper();

void GameInit();
void GameMove();
void exitGame();

void ProcFlag();
void exitFlag();

typedef struct{
    int id;
    int (*setupFunc)();
    int (*loopFunc)();
    int (*exitFunc)();
    TWindow Form;
} TProcess;

#define MAX_PROCESS 10
TProcess* mas_process[MAX_PROCESS] = {NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL};

#define MAX_WINDOW 10
extern TWindow* mas_window[MAX_WINDOW];


// запуск процесса
int initProcess(int (*setupFunc)(), int (*loopFunc)(), int (*exitFunc)()){
        TProcess *process = malloc(sizeof(TProcess));
        int id;
        for(id = 0; id < MAX_PROCESS; id++)
            if(mas_process[id] != NULL) continue;
            else{
                process->setupFunc = setupFunc;
                process->loopFunc = loopFunc;
                process->exitFunc = exitFunc;
                mas_process[id] = process;
                break;
            }
        if(setupFunc != NULL) mas_process[id]->setupFunc();
        return id;
}


// завершение процесса
void exitProcess(int id){
            if((mas_process[id] != NULL) && (mas_process[id]->exitFunc != NULL)) mas_process[id]->exitFunc();
            free(mas_process[id]);
            mas_process[id] = NULL;
}


// главный цикл Планировщика
void loopScheduler(){
    //DrawDesktop(0x0080C0);

    // главный конвейер процессов
    /*for(int i = 0; i < MAX_PROCESS; i++)
        if(mas_process[i] != NULL ) mas_process[i]->loopFunc();*/


    //if(GetKeyState('A') < 0) initProcess(initGUI, DrawWellpaper, NULL);
    if(GetKeyState('A') < 0) testSD2();


    if(GetKeyState('Z') < 0) {getPIDPixels(initProcess(NULL, ProcPixels, exitPixels));Sleep(350);}
    if(GetKeyState('X') < 0) {getPIDFlag(initProcess(NULL, ProcFlag, exitFlag));Sleep(350);}
    if(GetKeyState('C') < 0) {getPIDLady(initProcess(NULL, ProcLady, exitLady));Sleep(350);}
    if(GetKeyState('V') < 0) {getPIDLines(initProcess(NULL, ProcLines, exitLines));Sleep(350);}
    if(GetKeyState('B') < 0) {getPIDArcanoid(initProcess(initArkanoid, ProcArkanoidLoop, exitArkanoid));Sleep(350);}
    if(GetKeyState('N') < 0) {getPIDGame(initProcess(GameInit, GameMove, exitGame));Sleep(350);}
    if(GetKeyState('M') < 0) {getPIDKoch(initProcess(NULL, ProcKoch, exitKoch));Sleep(350);}


    if(mas_process[0] != NULL ) mas_process[0]->loopFunc();
    for(int id = 0; id < MAX_WINDOW; id++){
            if((mas_window[id] != NULL) && (mas_window[id]->visible == 1)){
                DrawWindow(*mas_window[id]);
                mas_process[mas_window[id]->PID]->loopFunc();
            }
        }



}
