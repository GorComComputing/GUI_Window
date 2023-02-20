#ifndef GUI_H
#define GUI_H

typedef struct Window{
    int PID;
    char caption[20];// = "Form 1";
    int x;
    int y;
    int width;
    int height;
    int BC;
    int visible;
    //bool onFocus;
    int (*onCreate)();
    int (*onClose)();
    int (*onFormClick)();
} TWindow;

#define STANDART 0xD8DCC0

#endif // GUI_H
