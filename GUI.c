#include <stdio.h>
#include <stdlib.h>

#include "GUI.h"
#include "display.h"
#include "BGL.h"

#define MAX_WINDOW 10
TWindow* mas_window[MAX_WINDOW] = {NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL};

//#define MAX_PROCESS 10
//extern TProcess* mas_process[MAX_PROCESS];

static char Wellpaper[1916*1074*3] = {0}; // спрайт обои
static char MenuStart[10*100*3] = {0}; // спрайт меню Пуск

static char BtnMin[14*14*3] = {0}; // спрайты кнопок окна
static char BtnMax[14*14*3] = {0}; //
static char BtnClose[14*14*3] = {0}; //


// создание окна
int formCreate(int PID, char name[], int x, int y, int sizeX, int sizeY, int BC, int (*onCreate)(), int (*onClose)(), int (*onFormClick)()){
     TWindow *form = malloc(sizeof(TWindow));
        int id;
        for(id = 0; id < MAX_WINDOW; id++)
            if(mas_window[id] != NULL) continue;
            else{
                form->PID = PID;
                sprintf(form->caption, name);
                form->x = x;
                form->y = y;
                form->width = sizeX;
                form->height = sizeY;
                form->BC = BC;
                form->visible = 1;
                form->onCreate = onCreate;
                form->onClose = onClose;
                form->onFormClick = onFormClick;

                mas_window[id] = form;
                break;
            }
        if(onCreate != NULL) mas_window[id]->onCreate();
        return id;

}


// удаление окна
void formClose(int id){
            if((mas_window[id] != NULL) && (mas_window[id]->onClose != NULL)) mas_window[id]->onClose();
            free(mas_window[id]);
            mas_window[id] = NULL;
}


// инициализация рабочего стола
void initGUI(){
    loadBMP(Wellpaper, "bmp\\wellpapers\\dom_derevia_gory_501789_1920x1080_1.bmp");
    loadBMP(MenuStart, "bmp\\window\\menu.bmp");

    loadBMP(BtnMin, "bmp\\window\\w1.bmp");
    loadBMP(BtnMax, "bmp\\window\\w2.bmp");
    loadBMP(BtnClose, "bmp\\window\\w3.bmp");
}


// Рабочий стол
void DrawDesktop(int BC){
    SetColor(BC);
    struct tPoint p[4];

    p[0].x = 2;
    p[0].y = 2;

    p[1].x = BITMAP_WIDTH-1;
    p[1].y = 2;

    p[2].x = BITMAP_WIDTH-1;
    p[2].y = BITMAP_HEIGHT-1;

    p[3].x = 2;
    p[3].y = BITMAP_HEIGHT-1;

    FillPoly(4, p);
}


// Обои на рабочий стол
void DrawWellpaper(){
    //1916*1074
    showBMP(Wellpaper, 2, 2, 1916, 1074);

    DrawTaskbar(2, BITMAP_HEIGHT - 28, BITMAP_WIDTH - 3, 24+4, 0x30B410);
    DrawStartMenu(2, BITMAP_HEIGHT - 28 - 110, 90, 110, STANDART);//0x005499);//0x30B410);
}


// Панель задач
void DrawTaskbar(int x, int y, int sizeX, int sizeY, int BC){
    SetColor(BC);
    struct tPoint p[4];

    p[0].x = x;
    p[0].y = y;

    p[1].x = x + sizeX;
    p[1].y = y;

    p[2].x = x + sizeX;
    p[2].y = y + sizeY;

    p[3].x = x;
    p[3].y = y + sizeY;

    FillPoly(4, p);

    SetColor(0xFFFFFF);
    SetBackColor(BC);
    TextOutgl("START", x + 8, y + 8, 1);


    SetColor(0xFFFFFF);
    SetBackColor(BC);
    TextOutgl("18:34", x - 42, y + 8, 1);

    SetColor(0xA0DC88);
    LinePP(x, y - 1, sizeX + 2, y - 1);
    LinePP(x, y - 1, 2, y + sizeY - 2);
    SetColor(0x80C848);
    LinePP(x+1, y, sizeX + 2, y);
    LinePP(x+1, y, x+1, y + sizeY - 2);
    SetColor(0x089000);
    LinePP(x+2, y + sizeY - 3, sizeX + 2, y + sizeY - 3);
    LinePP(x + sizeX - 1, y, sizeX + 1, y + sizeY - 3);
    SetColor(0x005C00);
    LinePP(x, y + sizeY - 2, sizeX + 2, y + sizeY - 2);
    LinePP(x + sizeX, y - 1, sizeX + 2, y + sizeY - 2);




    int count = 0;
    for(int id = 0; id < MAX_WINDOW; id++){
        if((mas_window[id] != NULL) && (mas_window[id]->visible == 1)){
            SetColor(0x50A0F8);

            p[0].x = x + 8 + 70 + 70*count;
            p[0].y = y + 2;

            p[1].x = x + 70 + 70 + 70*count;
            p[1].y = y + 2;

            p[2].x = x + 70 + 70 + 70*count;
            p[2].y = y + sizeY - 2;

            p[3].x = x + 8 + 70 + 70*count;
            p[3].y = y + sizeY - 2;

            FillPoly(4, p);

            SetColor(0xFFFFFF);
            SetBackColor(0x50A0F8);
            TextOutgl(mas_window[id]->caption, x + 8 + 70 + 70*count + 10, y + 8, 1);

            count++;
        }
    }
}


// Меню Пуск
void DrawStartMenu(int x, int y, int sizeX, int sizeY, int BC){
    SetColor(BC);
    struct tPoint p[4];

    p[0].x = x;
    p[0].y = y;

    p[1].x = x + sizeX;
    p[1].y = y;

    p[2].x = x + sizeX;
    p[2].y = y + sizeY;

    p[3].x = x;
    p[3].y = y + sizeY;

    FillPoly(4, p);

    SetColor(0x000000);
    SetBackColor(BC);
    TextOutgl("PROGRAMS", x + 28, y + 8, 1);

    SetColor(0x000000);
    SetBackColor(BC);
    TextOutgl("GAMES", x + 28, y + 8 + 16, 1);

    SetColor(0x000000);
    SetBackColor(BC);
    TextOutgl("CONSOLE", x + 28, y + 8 + 32, 1);



    SetColor(0xA0DC88);
    LinePP(x, y - 1, sizeX + 2, y - 1);
    LinePP(x, y - 1, 2, y + sizeY - 2);
    SetColor(0x80C848);
    LinePP(x+1, y, sizeX + 2, y);
    LinePP(x+1, y, x+1, y + sizeY - 2);
    SetColor(0x089000);
    LinePP(x+2, y + sizeY - 3, sizeX + 2, y + sizeY - 3);
    LinePP(x + sizeX - 1, y, sizeX + 1, y + sizeY - 3);
    SetColor(0x005C00);
    LinePP(x, y + sizeY - 2, sizeX + 2, y + sizeY - 2);
    LinePP(x + sizeX, y - 1, sizeX + 2, y + sizeY - 2);

    showBMP(MenuStart, x + 2, y, 10, 100);
}


// Окно
void DrawWindow(TWindow Form){
    SetColor(Form.BC);
    struct tPoint p[4];

    p[0].x = Form.x;
    p[0].y = Form.y;

    p[1].x = Form.x + Form.width - 1;
    p[1].y = Form.y;

    p[2].x = Form.x + Form.width - 1;
    p[2].y = Form.y + Form.height;

    p[3].x = Form.x;
    p[3].y = Form.y + Form.height;

    FillPoly(4, p);

    SetColor(0x0055E5);
    p[0].x = Form.x + 2;
    p[0].y = Form.y + 2;

    p[1].x = Form.x + Form.width - 2;
    p[1].y = Form.y + 2;

    p[2].x = Form.x + Form.width - 2;
    p[2].y = Form.y + 15 + 2;

    p[3].x = Form.x + 2;
    p[3].y = Form.y + 15 + 2;

    FillPoly(4, p);


    SetColor(0xFFFFFF);
    SetBackColor(0x0055E5);
    TextOutgl(Form.caption, Form.x + 4, Form.y + 6, 1);

    SetColor(0xF8FCF8);
    LinePP(Form.x, Form.y - 1, Form.x + Form.width - 1, Form.y - 1);
    LinePP(Form.x, Form.y - 1, Form.x, Form.y + Form.height - 2);
    SetColor(0xE0E0E0);
    LinePP(Form.x+1, Form.y, Form.x + Form.width - 1, Form.y);
    LinePP(Form.x+1, Form.y, Form.x+1, Form.y + Form.height - 2);
    SetColor(0x787C78);
    LinePP(Form.x+2, Form.y + Form.height - 3, Form.x + Form.width - 1, Form.y + Form.height - 3);
    LinePP(Form.x + Form.width - 2, Form.y, Form.x + Form.width - 2, Form.y + Form.height - 3);
    SetColor(0x000000);
    LinePP(Form.x, Form.y + Form.height - 2, Form.x + Form.width - 1, Form.y + Form.height - 2);
    LinePP(Form.x + Form.width - 1, Form.y - 1, Form.x + Form.width - 1, Form.y + Form.height - 2);


    showBMP(BtnMin, Form.x + Form.width - 2-14*3, Form.y + 1, 14, 14);
    showBMP(BtnMax, Form.x + Form.width - 2-14*2, Form.y + 1, 14, 14);
    showBMP(BtnClose, Form.x + Form.width - 2-14, Form.y, 14, 14);
}


