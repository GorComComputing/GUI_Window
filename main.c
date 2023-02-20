#include <stdio.h>
#include <stdlib.h>

#include <winsock2.h>

#define _WIN32_WINNT 0x0A00
#include <windows.h>

#include "GUI.h"
#include "display.h"

void ProcPixels();
void ProcLady();
void ProcLines();
void ProcKoch();

void initGUI();
void DrawWellpaper();


SOCKET s;
SOCKET client_socket; // для сети
SOCKADDR_IN client_addr;
int client_addr_size = sizeof(client_addr);
char mes[200] = "YES!";
int isConnect = 0;

int program = 0;
extern int (*func[10])();
int xPos;
int yPos;


#define MAX_WINDOW 10
extern TWindow* mas_window[MAX_WINDOW];


HDC dc;
RECT rct;   //Размеры клиентской области
LPRECT rctScr;      // Размер экрана
HCURSOR hCursor; // Курсор мыши


// Обработчик событий окна
LRESULT WndProc(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam){
    if(message == WM_DESTROY) // Если закрытие окна
        PostQuitMessage(0);   // Создаем в очереди сообщение о закрытии приложения
    else if(message == WM_KEYDOWN){ // При нажатии клавиатуры
        if(wparam == VK_ESCAPE) PostQuitMessage(0); // Если Esc, закрываем программу
        else if(wparam == '1'){ // Если 1, убираем рамку окна и раскрываем на весь экран
            SetWindowLongPtr(hwnd, GWL_STYLE, WS_POPUP | WS_VISIBLE);
            SetWindowPos(hwnd, HWND_TOP, 0, 0, rctScr[0].right, rctScr[0].bottom, SWP_SHOWWINDOW);
            hCursor = SetCursor(NULL);
        }
        else if(wparam == '2'){ // Если 2, возвращаем окно назад
            SetWindowLongPtr(hwnd, GWL_STYLE, WS_OVERLAPPEDWINDOW | WS_VISIBLE);
            SetWindowPos(hwnd, HWND_TOP, 10, 10, BITMAP_WIDTH, BITMAP_HEIGHT, SWP_SHOWWINDOW);
            SetCursor(hCursor);
        }
        else if(wparam == '3'){ // Если 3, меняем разрешение экрана
            DEVMODE deviceMode;
            {
                deviceMode.dmSize = sizeof(deviceMode);
                deviceMode.dmPelsWidth = 800;//1600;
                deviceMode.dmPelsHeight = 600;//900;
                deviceMode.dmFields = DM_PELSHEIGHT | DM_PELSWIDTH;
            }
            ChangeDisplaySettings(&deviceMode, CDS_FULLSCREEN);
        }
        else if(wparam == '4'){ // Если 4, возвращаем разрешение экрана
            ChangeDisplaySettings(0, 0);
        }
        else if(wparam == 'Q') program = 10;
        else if(wparam == 'W') TestLineTime(dc);
        else if(wparam == 'E') {program = 11; SetCursor(LoadCursor(NULL, IDC_ARROW)); initProcess(initGUI, DrawWellpaper, NULL);}//SetCursor(hCursor);}
        //else if(wparam == WM_LBUTTONDOWN) func[0] = ProcPixels;
        //else if(wparam == 'X') func[0] = NULL;

        else if(wparam == '0') program = 0;
        else if(wparam == '9') program = 9;
        else if(wparam == '8') program = 8;
        else if(wparam == '7') program = 7;
        else if(wparam == '6') program = 6;
        else if(wparam == '5'){
            SetColor(0xFFFFFF);
            SetBackColor(0x000000);
            ClearDevice();
            TextOutgl("WAITING CONNECT", 200, 300, 4);
            Draw(dc);
            // Ждем входящего соединения
            if(client_socket = accept(s, &client_addr, &client_addr_size)){
                isConnect = 1;
                send(client_socket, mes, sizeof(mes), 0); // Отправляем сообщение
                printf("connect OK\n");
                program = 8;
            }
        }
        else if(wparam == VK_SPACE){
            Melody();
        }
        printf("code = %d\n", wparam);
    }
    else if(message == WM_CHAR)    // Символы нажатых клавиш
        printf("%c\n", wparam);
    else if(message == WM_MOUSEMOVE){ // При перемещении мыши в клиентской области
        xPos = LOWORD(lparam);
        yPos = HIWORD(lparam);
        printf("mouse [%d,%d]\n", xPos, yPos);
    }
    else if(message == WM_LBUTTONDOWN)  // При нажатии левой кнопки мыши в клиентской области
    {
        xPos = LOWORD(lparam);
        yPos = HIWORD(lparam);
        printf("mouse Left Button Down [%d,%d]\n", xPos, yPos);


        for(int id = 0; id < MAX_WINDOW; id++){
            if((mas_window[id] != NULL) && (mas_window[id]->x < xPos) && (mas_window[id]->x + mas_window[id]->width > xPos) && (mas_window[id]->y < yPos) && (mas_window[id]->y + mas_window[id]->height > yPos)){
                printf("on %s\n", mas_window[id]->caption);
                mas_window[id]->onFormClick();
            }
        }
    }

    else if(message == WM_SIZE)
        GetClientRect(hwnd, &rct);
    else return DefWindowProcA(hwnd, message, wparam, lparam); // Вызываем стандартный обработчик событий
}

int main(){
    rctScr = malloc(sizeof(*rctScr));            //
    GetClientRect(GetDesktopWindow(), rctScr);   // Получаем размер экрана

    WNDCLASSA wcl;
    memset(&wcl, 0, sizeof(WNDCLASSA)); // Заполняем сначала всё нулями
    wcl.lpszClassName = "my Window";    // Имя класса окна
    wcl.lpfnWndProc = WndProc;  //DefWindowProcA;   // Обработчик событий окна по умолчанию
    RegisterClassA(&wcl);   // Создаем класс окна

    dc = SetDisplay();

    DEVMODE deviceMode;
            {
                deviceMode.dmSize = sizeof(deviceMode);
                deviceMode.dmPelsWidth = BITMAP_WIDTH; //1600
                deviceMode.dmPelsHeight = BITMAP_HEIGHT; //900
                deviceMode.dmFields = DM_PELSHEIGHT | DM_PELSWIDTH;
            }
    hCursor = SetCursor(NULL);
    ChangeDisplaySettings(&deviceMode, CDS_FULLSCREEN);

    /////////////////////// Инициализируем сокет
    WSADATA ws;
    WSAStartup(MAKEWORD(2, 2), &ws); // Инициализируем сокеты

    s = socket(AF_INET, SOCK_STREAM, 0); // Создаем сокет TCP

    SOCKADDR_IN sa; // Структура с настройками соединения
    memset(&sa, 0, sizeof(sa));
    sa.sin_family = AF_INET;
    sa.sin_port = htons(1234);
    ///////////////////////////////////////
    bind(s, &sa, sizeof(sa));  // Устанавливаем настройки соединения на соккет
    listen(s, 100); // Слушаем очередь

    char buf[200];     // буфер для приема данных
    memset(buf, 0, sizeof(buf));
    /////////////////////////////////////////////////




    MSG msg;
    // Главный цикл
    while(1){
        // Обрабатываем сообщения
        if(PeekMessageA(&msg, NULL, 0, 0, PM_REMOVE)){
            if(msg.message == WM_QUIT) break;
            TranslateMessage(&msg); // Передает сообщение о нажатии символа
            DispatchMessage(&msg);  // Передаем сообщение окну
        }
        else{
            // Обрабатываем сетевые сообщения
            WSAEventSelect(client_socket, 0, FD_READ);  // Получает сообщение без таймаута
            char mes2[20] = "\0";
            if(recv(client_socket, mes2, sizeof(mes2), 0) > 0){
                int position = 0;      // Позиция
                char **tokens = malloc(sizeof(char*) * 20);   // Выделяем память

                char *mes_str = strtok(mes2, "\n");
                while(mes_str != NULL){
                    printf("->%s\n", mes_str);
                    tokens[position] = mes_str;
                    position++;
                    mes_str = strtok(NULL, "\n");
                }
                int count = 0;
                while(count < position){
                char *cmd_mes = strtok(tokens[count], " ");
                int cmd = atoi(cmd_mes);
                //printf("%s->%d\n", mes2, cmd);
                if(cmd == 0){
                    char *mes_x = strtok(NULL, " \n");
                    char *mes_y = strtok(NULL, " \n");
                    int speed_x = atoi(mes_x);
                    int speed_y = atoi(mes_y);
                    setSpeed(speed_x, speed_y);
                    //printf("speed: %s.%s\n", mes_x, mes_y);
                }
                if(cmd == 1){
                    char *mes_score1 = strtok(NULL, " ");
                    char *mes_score2 = strtok(NULL, " ");
                    int score1 = atoi(mes_score1);
                    int score2 = atoi(mes_score2);
                    setScore(score1, score2);
                    printf("score: %s.%s\n", mes_score1, mes_score2);
                }
                if(cmd == 2){
                    char *mes_pos1 = strtok(NULL, " ");
                    char *mes_pos2 = strtok(NULL, " ");
                    //char *mes_cnt = strtok(NULL, " \n");
                    int pos1 = atoi(mes_pos1);
                    int pos2 = atoi(mes_pos2);
                    setNew(pos1, pos2);
                    printf("new: %s.%s\n", mes_pos1, mes_pos2);
                }
                if(cmd == 3){
                    char *mes_i = strtok(NULL, " ");
                    int i = atoi(mes_i);
                    setDel(i);
                    printf("del: %s\n", mes_i);
                }
                /*if(cmd == 4){
                    char *mes_sp1 = strtok(NULL, " ");
                    char *mes_sp2 = strtok(NULL, " ");
                    char *mes_i = strtok(NULL, " ");
                    int sp1 = atoi(mes_sp1);
                    int sp2 = atoi(mes_sp2);
                    int i = atoi(mes_i);
                    //if(pos1 && pos2)
                        setPosition(sp1, sp2, i);
                    printf("pos: %s.%s.%s\n", mes_sp1, mes_sp2, mes_i);
                }*/

                count++;
                }
                free(tokens);

            }

            if(program == 0) bootFlag();
            else if(program == 9) {TestLine();} //Line(0, 590, 400, 500);
            else if(program == 8) GameMove(dc);
            else if(program == 7) {DrawKoch(7, 50, 200, 60, 100); Sleep(150);}
            else if(program == 6) FillLBrnd();
            else if(program == 10) {ClearDevice(); LadyBitmap(200, 100, 50, 50, 8);}
            else if(program == 11) loopScheduler();

            Draw(dc);
        }
    }
    DeleteDisplay();
    closesocket(s); // Закрываем сокет
/*

    LPPOINT pPnt;               // Координаты
    pPnt = malloc(sizeof(*pPnt));

    LPRECT rct;                 // Размер области
    rct = malloc(sizeof(*rct));
    GetClientRect(hwnd, rct);   // Получаем размер клиентской области

    LPRECT rctScr;                 // Размер области
    rctScr = malloc(sizeof(*rct));
    GetClientRect(GetDesktopWindow(), rctScr);   // Получаем размер клиентской области

    /*SelectObject(dc, GetStockObject(DC_BRUSH));
    SetDCBrushColor(dc, RGB(255, 0, 255));
    Rectangle(dc, 100, 100, 110, 110);
    Ellipse(dc, 200, 100, 300, 200);

    int i = 0;
    char c[1024];*/
   /* do{
        system("cls");

        GetCursorPos(pPnt); // Получаем координаты курсора
        printf("\n Global cursor pos: %d, %d\n", pPnt[0].x, pPnt[0].y);

        ScreenToClient(hwnd, pPnt); // Преобразуем глобальные координаты в локальные
        printf("\n Local cursor pos: %d, %d\n", pPnt[0].x, pPnt[0].y);

        printf("\n Client size: %d x %d\n", rct[0].right, rct[0].bottom);

        printf("\n Screen size: %d x %d\n", rctScr[0].right, rctScr[0].bottom);

        if(GetKeyState(VK_LBUTTON) < 0)
            printf("\n Left Mouse Button is down\n");

        Sleep(100);


        /*i++;
        SelectObject(dc, GetStockObject(DC_BRUSH));
        SetDCBrushColor(dc, RGB(255, 255, 255));
        SelectObject(dc, GetStockObject(DC_PEN));
        SetDCPenColor(dc, RGB(255, 0, 255));
        Rectangle(dc, 0, 0, 640, 300);

        SelectObject(dc, GetStockObject(DC_BRUSH));
        SetDCBrushColor(dc, RGB(0, 255, 0));
        Rectangle(dc, 0+i, 0, 100+i, 100);

        sprintf(c, "My text");
        TextOut(dc, 20, 50, c, strlen(c) + 1);

        Sleep(10);*/
  //  } while(GetKeyState(VK_ESCAPE) >= 0);

    return 0;
}









