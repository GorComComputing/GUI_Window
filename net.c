#include <conio.h>
#include <winsock2.h>

// ���
void chat(SOCKET s){
    char mes[200];
    WSAEventSelect(s, 0, FD_READ);  // �������� ��������� ��� ��������
    do{
        if((GetKeyState(VK_F1) < 0) && (GetForegroundWindow() == GetConsoleWindow())){ // �������� ��������� ������ ��� F1 � ���� � ������
            printf("message: ");
            while(getch() != 0); // ������� ������ ������� � ������
            scanf("\n%200[0-9a-zA-Z.,! ]", mes);
            send(s, mes, sizeof(mes), 0); // ���������� ���������
        }

        if(recv(s, mes, sizeof(mes), 0) > 0)
            printf("%s\n", mes);
    }while(GetKeyState(VK_ESCAPE) >= 0);  // ����� �� ESC
}
