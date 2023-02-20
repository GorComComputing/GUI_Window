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

static int t = 0;
//extern int xPos;
//extern int yPos;

// ������� �� ����� �� ����
void onFormClickFlag(){
    Beep(700, 50);
    exitProcess(PID);
}


// �������� ID ������ ��������
void getPIDFlag(int ID){
    PID = ID;
    form = formCreate(PID, "FLAG", 350, 200, 334, 243 + 15, 0x000000, NULL, NULL, onFormClickFlag);
    Form1 = *mas_window[form];
}


// ���������� ��������
void exitFlag(){
    Beep(500, 50);
    formClose(form);
    Beep(300, 200);
}


// ������� ���� ����������
void ProcFlag(){
    char flag[] = \
    "                                                                                                                                 "
    "                                                                                                                                 "
    "                                                                                                                                 "
    "                                                                                                                                 "
    "                                                                                                                                 "
    "                                                                                                                                 "
    "                                                                                                                                 "
    "                                                                                                                                 "
    "                                                                                                                                 "
    "                                                                                                                                 "
    "                                                                                                                                 "
    "                                                                                                                                 "
    "                                                                                                                                 "
    "                                                                                                                                 "
    "                                                                                                                                 "
    "                                                                                                                                 "
    "                                                                                                                                 "
    "                                                                                                                                 "
    "                                                                                                                                 "
    "                                                                                                                                 "
    "                                                                                                                                 "
    "                                                                                                                                 "
    "                                                                                                                                 "
    "                                                                                                                                 "
    "                                                                                                                                 "
    "             ppppppppppp      ppppppp      pppppppppp      pppppppppp    ppp     ppp    ppppppppppp    pppppppppp                "
    "             ppppppppppp    ppppppppppp    ppppppppppp    ppppppppppp    ppp     ppp    ppppppppppp    ppppppppppp               "
    "             ppp            ppp     ppp    ppp     ppp    ppp     ppp    ppp     ppp    ppp            ppp     ppp               "
    "             ppp            ppp     ppp    ppp     ppp    ppp     ppp    ppp     ppp    ppp            ppp     ppp               "
    "             ppp            ppp     ppp    ppp     ppp    ppp     ppp    ppp     ppp    ppppppppppp    pppppppppp                "
    "             ppp            ppp     ppp    ppppppppppp    ppppppppppp    ppp     ppp    ppppppppppp    pppppppppp                "
    "             ppp            ppp     ppp    pppppppppp      pppppppppp     pppppppppp    ppp            ppp     ppp               "
    "             ppp            ppp     ppp    ppp                ppp ppp            ppp    ppp            ppp     ppp               "
    "             ppp            ppppppppppp    ppp              ppp   ppp            ppp    ppppppppppp    ppppppppppp               "
    "             ppp             ppppppppp     ppp            ppp     ppp            ppp    ppppppppppp    pppppppppp                "
    "                                                                                                                                 "
    "                                                                                                                                 "
    "                                                                                                                                 "
    "                                                                                                                                 "
    "                                                                                                                                 "
    "                                                                                                                                 "
    "                                                                                                                                 "
    "                                                                                                                                 "
    "                                                                                                                                 "
    "                                                                                                                                 "
    "                                                                                                                                 "
    "                                                                                                                                 "
    "                                                                                                                                 "
    "                                                                                                                                 "
    "                                                                                                                                 "
    "                                                                                                                                 "
    "                                                                                                                                 "
    "                                                                                                                                 "
    "                                                                                                                                 "
    "                                                                                                                                 "
    "                                                                                                                                 "
    "                                                                                                                                 "
    "                                                                                                                                 "
    "                                                                                                                                 "
    "                                                                                                                                 "
    "                                                                                                                                 "
    "                                                                                                                                 "
    "                                                                                                                                 "
    "                                                                                                                                 "
    "                                                                                                                                 "
    "                                                                                                                                 "
    ;


    t += 20;
    if(t > 1000) t = 0;
    FlagF(flag, Form1.x + 30, Form1.y + 30, 129, 66, 2, t);

}


// ���� �������
void FlagF(char flag[], int x, int y, int sizeX, int sizeY, int scale, int t){
    int xstart = x;
    int ystart = y;
    int dy = 0;
    SetBackColor(0x000000);
    //ClearDevice();

    /*SetColor(0x000000);
    struct tPoint p[4];

    p[0].x = x;
    p[0].y = y;

    p[1].x = x + sizeX*scale;
    p[1].y = y;

    p[2].x = x + sizeX*scale;
    p[2].y = y + sizeY*scale;

    p[3].x = x;
    p[3].y = y + sizeY*scale;

    FillPoly(4, p);*/


    int sd = rand()%(129);

    for(int i = 0; i < sizeY; i++){
        for(int scaleY = scale; scaleY > 0; scaleY--){
        for(int j = 0; j < sizeX; j++){
                if(flag[i*sizeX + j] == 'p'){
                    for(int scaleX = scale; scaleX > 0; scaleX--){
                    PutPixel((int)(x + j + 5*cos(i/12.-3.14/2.0-t)), (int)(y + i + 5*sin(j/12.-t)), 0x9FAF00+0x101000*(int)(0x6*(sin(j/24.-t))));
                    if(scaleX > 1) x++;
                    }
                }
                else{
                    for(int scaleX = scale; scaleX > 0; scaleX--){
                    PutPixel((int)(x + j + 5*cos(i/12.-3.14/2.0-t)), (int)(y + i + 5*sin(j/12.-t)), 0xAF0000+0x100000*(int)(0x6*(sin(j/24.-t))));
                    if(scaleX > 1) x++;
                    }
                }
                x += 0; //+2
                if(x%2 == 0) y++;
        }
        x = xstart;
        if(scaleY > 1) ystart++;
        y = ystart;
        }
    }
}
