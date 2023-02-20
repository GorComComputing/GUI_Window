#define _WIN32_WINNT 0x0A00
#include <windows.h>

#include "../display.h"
#include "../BGL.h"


static int t = 0; // הכÿ פכאדא


// װכאד ֳמנÿקוג
void bootFlag(){
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

    int x = BITMAP_WIDTH/2-100;
    int y = BITMAP_HEIGHT/2-200;
    int sizeX = 129;
    int sizeY = 66;
    int scale = 2;


    int xstart = x;
    int ystart = y;
    int dy = 0;
    SetBackColor(0x000000);
    ClearDevice();

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

        Sleep(150);
}
