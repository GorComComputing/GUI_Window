#include <conio.h>
#include <winsock2.h>

// чат
void chat(SOCKET s){
    char mes[200];
    WSAEventSelect(s, 0, FD_READ);  // Получает сообщение без таймаута
    do{
        if((GetKeyState(VK_F1) < 0) && (GetForegroundWindow() == GetConsoleWindow())){ // печатаем сообщение только при F1 у окна в фокусе
            printf("message: ");
            while(getch() != 0); // убирает лишние семволы в начале
            scanf("\n%200[0-9a-zA-Z.,! ]", mes);
            send(s, mes, sizeof(mes), 0); // Отправляем сообщение
        }

        if(recv(s, mes, sizeof(mes), 0) > 0)
            printf("%s\n", mes);
    }while(GetKeyState(VK_ESCAPE) >= 0);  // выход по ESC
}
