#include "game.h"
#include "GUI.h"

extern HDC dc;

// Счет игроков
int player1Score = 0;
int player2Score = 0;



static TWindow Form1;
static int form = 0;
static int PID = 0;
#define MAX_WINDOW 10
extern TWindow* mas_window[MAX_WINDOW];


// событие по клику по окну
void onFormClickGame(){
    Beep(700, 50);
    exitProcess(PID);
}


// получает ID своего процесса
void getPIDGame(int ID){
    PID = ID;
    form = formCreate(PID, "GAME", 800, 300, 804, 603 + 15, STANDART, NULL, NULL, onFormClickGame);
    Form1 = *mas_window[form];
}



// Возвращает точку по двум координатам
TPoint point(float x, float y){
    TPoint pt;
    pt.x = x;
    pt.y = y;
    return pt;
}


// Проверка столкновения
BOOL ObjectCollision(TObject o1, TObject o2){
    return ((o1.pos.x + 8*o1.size.x) > o2.pos.x) && (o1.pos.x < (o2.pos.x + 8*o2.size.x)) &&
           ((o1.pos.y + 8*o1.size.y) > o2.pos.y) && (o1.pos.y < (o2.pos.y + 8*o2.size.y));
}


// Инициализация объекта
void ObjectInit(TObject *obj, float xPos, float yPos, float width, float height, char objType){
   obj->pos = point(xPos, yPos);
   obj->size = point(width, height);
   obj->brush = RGB(0, 255, 0);
   obj->speed = point(0, 0);
   obj->oType = objType;
   if(objType == 'p') obj->brush = 0xEDC400;
   if(objType == 'e') obj->brush = 0xF14A64;
   if(objType == 'x') obj->brush = 0x83A9FA;
   obj->isDel = FALSE;
}


// Рисование объекта
void ObjectShow(TObject obj, HDC dc, int x, int y){

    char monster1[] = \
    "   pp   "
    "  pppp  "
    " pppppp "
    "pp pp pp"
    "pppppppp"
    "  p  p  "
    " p pp p "
    "p p  p p"
    ;//8x8
    char monster2[] = \
    "    pppp    "
    " pppppppppp "
    "pppppppppppp"
    "ppp  pp  ppp"
    "pppppppppppp"
    "   pp  pp   "
    "  pp    pp  "
    "pp        pp"
    ;//12x8
    char monster31[] = \
    "  p     p  "
    "   p   p   "
    "  ppppppp  "
    " pp ppp pp "
    "ppppppppppp"
    "p ppppppp p"
    "p p     p p"
    "   pp pp   "
    ;//11x8
    char monster32[] = \
    "    p p    "
    "   p   p   "
    "  ppppppp  "
    " pp ppp pp "
    "ppppppppppp"
    "p ppppppp p"
    "p p     p p"
    "   pp pp   "
    ;//11x8

#define ADDY 150
    int yPos = (int)(obj.pos.y) + ADDY;
    SetColor(obj.brush);
    SetBackColor(0x000000);
    if(obj.oType == 'p'){
        //if(obj.sprite)
            DrawBitmapTransparent(monster31, (int)(obj.pos.x)+x, yPos+y,
               (int)(obj.size.x), (int)(obj.size.y), 8);
       /* else
            DrawBitmapTransparent(monster32, (int)(obj.pos.x), yPos,
               (int)(obj.size.x), (int)(obj.size.y), 8);*/
    }
    if(obj.oType == 'e'){
        DrawBitmapTransparent(monster1, (int)(obj.pos.x)+x, yPos+y,
               (int)(obj.size.x), (int)(obj.size.y), 8);
    }
    if(obj.oType == 'x'){
        DrawBitmapTransparent(monster2, (int)(obj.pos.x)+x, (int)yPos+y,
               (int)(obj.size.x), (int)(obj.size.y), 8);
    }

    SetColor(0xFFFFFF);
    //SetBackColor(0x000000);
    SetBackColor(STANDART);
    LinePP(0+x, 0+ADDY+y, 84*8+x, 0+ADDY+y);
    LinePP(0+x, 0+ADDY+y, 0+x, 48*8+ADDY+y);
    LinePP(0+x, 48*8+ADDY+y, 84*8+x, 48*8+ADDY+y);
    LinePP(84*8+x, 0+ADDY+y, 84*8+x, 48*8+ADDY+y);

    TextOutgl("GAME", 50+x, 50+y, 4);

    TextOutgl("0 - FLAG", 450+x, 20+y, 2);
    TextOutgl("9 - LINES", 450+x, 20+16+y, 2);
    TextOutgl("8 - GAME", 450+x, 20+32+y, 2);
    TextOutgl("7 - KOCH", 450+x, 20+48+y, 2);
    TextOutgl("6 - PIXELS", 450+x, 20+64+y, 2);
    TextOutgl("5 - CONNECTING", 450+x, 20+80+y, 2);
    TextOutgl("ESC - EXIT", 450-28+x, 20+96+y, 2);


    char playerScoreStr[3];

    SetColor(0xEDC400);
    SetBackColor(STANDART);
    itoa(player1Score, playerScoreStr, 10); // Преобразуем число в строку
    TextOutgl(playerScoreStr, 730+x, 300+y, 2);
    SetColor(0x83A9FA);
    SetBackColor(STANDART);
    itoa(player2Score, playerScoreStr, 10);  // Преобразуем число в строку
    TextOutgl(playerScoreStr, 730+x, 332+y, 2);

    LadyBitmap(650+x, 0+y, 50, 50, 3);

}


// Движение врагов
void ObjectSetDestPoint(TObject *obj, float xPos1, float yPos1, float xPos2, float yPos2, float vecSpeed){
    // Находим расстояние между объектом и точкой назначения
    float dx;
    float dy;
    if((xPos1 - obj->pos.x) < (xPos2 - obj->pos.x)) dx = xPos1 - obj->pos.x;
    else dx = xPos2 - obj->pos.x;
    if((yPos1 - obj->pos.y) < (yPos2 - obj->pos.y)) dy = yPos1 - obj->pos.y;
    else dy = yPos2 - obj->pos.y;

    TPoint xyLen = point(dx, dy);

    // Находим расстояние по прямой
    float dxy = sqrt(xyLen.x*xyLen.x + xyLen.y*xyLen.y);
    // Задаем скорость по каждой оси
    obj->speed.x = xyLen.x/dxy*vecSpeed;
    obj->speed.y = xyLen.y/dxy*vecSpeed;
}


TObject player1; // Создаем персонажа 1
TObject player2; // Создаем персонажа 2
PObject mas = NULL; // массив врагов
int masCnt = 0; // Счетчик количества врагов


// Устанавливает полученную скорость
void setSpeed(int x, int y){
    player2.speed.x = x;
    player2.speed.y = y;
}


// Устанавливает полученный счет
void setScore(int score1, int score2){
    if(score1 > player1Score) MelodyLevelUp();
    if(score2 > player2Score) MelodyGameOver();
    player1Score = score1;
    player2Score = score2;
}




// Перемещение объектов
void ObjectMove(TObject *obj){
    if(obj->oType == 'e')
        if(rand()%40 == 1){
            static float enemySpeed = -1.5;
            ObjectSetDestPoint(obj, player1.pos.x, player1.pos.y, player2.pos.x, player2.pos.y, enemySpeed);
        }

    if(!(((obj->pos.x + obj->speed.x) < 0) || ((obj->pos.x + obj->speed.x + 8*obj->size.x) > 84*8)))
        obj->pos.x += obj->speed.x;
    if(!(((obj->pos.y + obj->speed.y) < 0) || ((obj->pos.y + obj->speed.y + 8*obj->size.y) > (48*8))))
        obj->pos.y += obj->speed.y;

    /*for(int i = 0; i < masCnt; i++){
        if((obj->oType == 'p')   && (ObjectCollision(*obj, mas[i]))){
            player1Score++;
            mas[i].isDel = TRUE;
            obj->isDel = TRUE;
            MelodyLevelUp();
        }
        if((obj->oType == 'x')  && (ObjectCollision(*obj, mas[i]))){
            player2Score++;
            mas[i].isDel = TRUE;
            obj->isDel = TRUE;
            MelodyGameOver();
        }
    }*/
}


// Создает элементы массива врагов
PObject NewObject(){
    masCnt++;
    mas = realloc(mas, sizeof(*mas)*masCnt);
    return mas + masCnt - 1;
}


// Устанавливает создание нового врага
void setNew(int pos1, int pos2){
    //int pos_x = pos1*672/84;
    //int pos_y = pos2*384/48;
    AddEnemy(pos1*672/84, pos2*384/48);
}


// Устанавливает удаление нового врага
void setDel(int i){
    mas[i].isDel = TRUE;
}


// Устанавливает создание нового врага
void setPosition(int pos1, int pos2, int i){
    mas[i].speed.x = pos1;//*672/84;
    mas[i].speed.y = pos2;//*384/48;

}


// Генерация новых врагов
void GenNewEnemy(){
    int pos1 = rand()%60*8;
    int pos2 = rand()%30*8;
    int k = rand()%100;
    if(k == 1) AddEnemy(pos1, pos2);
}


// Удаляет помеченные объекты
void DelObject(){
    int i = 0;
    while(i < masCnt){
        if(mas[i].isDel){
            masCnt--;
            mas[i] = mas[masCnt];
            mas = realloc(mas, sizeof(*mas)*masCnt);
        }
        else{
            i++;
        }
    }
}


// Обработчик клавиш
void PlayerControl(){
    static int playerSpeed = 4;
    player1.speed.x = 0;
    player1.speed.y = 0;
    if(GetKeyState(VK_UP) < 0) player1.speed.y = -playerSpeed;
    if(GetKeyState(VK_DOWN) < 0) player1.speed.y = playerSpeed;
    if(GetKeyState(VK_LEFT) < 0) player1.speed.x = -playerSpeed;
    if(GetKeyState(VK_RIGHT) < 0) player1.speed.x = playerSpeed;
    if((player1.speed.x != 0) && (player1.speed.y != 0)) player1.speed = point(player1.speed.x * 0.7, player1.speed.y * 0.7);

    extern SOCKET client_socket; // для сети
    char mes[10] = "\0";
    sprintf(mes, "%d %d", (int)player1.speed.x, (int)player1.speed.y);
    send(client_socket, mes, sizeof(mes), 0); // Отправляем сообщение

    /*float speed_x = 0;
    float speed_y = 0;
    player2.speed.x = 0;
    player2.speed.y = 0;
    if(GetKeyState('W') < 0) speed_y = -playerSpeed; //player2.speed.y = -playerSpeed;
    if(GetKeyState('S') < 0) speed_y = playerSpeed; //player2.speed.y = playerSpeed;
    if(GetKeyState('A') < 0) speed_x = -playerSpeed; //player2.speed.x = -playerSpeed;
    if(GetKeyState('D') < 0) speed_x = playerSpeed; //player2.speed.x = playerSpeed;
    //if((player2.speed.x != 0) && (player2.speed.y != 0)) player2.speed = point(player2.speed.x * 0.7, player2.speed.y * 0.7);

    setSpeed(speed_x, speed_y);*/
}


// Инициализация игры
void GameInit(){
    ObjectInit(&player1, 100, 100, 11, 8, 'p');
    ObjectInit(&player2, 500, 100, 12, 8, 'x');
    //ObjectInit(NewObject(), 400, 100, 8, 8, 'e');
    //ObjectInit(NewObject(), 400, 300, 8, 8, 'e');
}


// завершение процесса
void exitGame(){
    Beep(500, 50);
    formClose(form);
    Beep(300, 200);
}


// Перемещение объектов
void GameMove(){
    PlayerControl();
    ObjectMove(&player1);
    ObjectMove(&player2);

    for(int i = 0; i < masCnt; i++){
        ObjectMove(mas + i);
    }

    //GenNewEnemy();
    DelObject();


    ObjectShow(player1, dc, Form1.x+2, Form1.y+17);
    ObjectShow(player2, dc, Form1.x+2, Form1.y+17);

    for(int i = 0; i < masCnt; i++)
        ObjectShow(mas[i], dc, Form1.x+2, Form1.y+17);


    Sleep(9);
}

