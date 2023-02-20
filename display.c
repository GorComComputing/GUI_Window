// ������� �������
#include <stdio.h>
#include <stdlib.h>

#define _WIN32_WINNT 0x0A00
#include <windows.h>

#include "display.h"


BITMAPINFOHEADER    bih;
PDWORD              pBmp;
HBITMAP             memBM;
HDC                 memDC;


// ������������� ���� ������
HDC SetDisplay(){
    HWND hwnd = CreateWindow("my Window", "Window Name", WS_OVERLAPPEDWINDOW,
                             10, 10, BITMAP_WIDTH, BITMAP_HEIGHT, NULL, NULL, NULL, NULL); // ������� ����  //640 480
    HDC dc = GetDC(hwnd);

    memset(&bih, 0, sizeof bih);        // ������� ��������� �����������
    bih.biSize          = sizeof bih;   // ��������� ��� ������ �����������
    bih.biPlanes        = 1;
    bih.biBitCount      = 8 << 2;       // 32 ���� �� �������
    bih.biCompression   = BI_RGB;       // ������� ������
    bih.biWidth         = BITMAP_WIDTH;
    bih.biHeight        = BITMAP_HEIGHT;
    bih.biSizeImage     = bih.biWidth * bih.biHeight * (1 << 2);

    memDC = CreateCompatibleDC(dc); // ������� ����������� �������� � ������
    memBM = CreateDIBSection(NULL, (PBITMAPINFO)&bih, DIB_RGB_COLORS, (PVOID *)&pBmp, NULL, NULL);
    //HBITMAP memBM = CreateCompatibleBitmap(dc, rct.right - rct.left, rct.bottom - rct.top);  // �������� �� ������� ����� �������� ����� ����������� ��������
    SelectObject(memDC, memBM);  // �������� ����������� ��������

    // ������������� ����� �� ��������� � ������� �����
    SetColor(0x000000);
    SetBackColor(0xFFFFFF);
    ClearDevice();

    // ���������� ���� �� ���� ����� 800 600
    SetWindowLongPtr(hwnd, GWL_STYLE, WS_POPUP | WS_VISIBLE);
    SetWindowPos(hwnd, HWND_TOP, 0, 0, BITMAP_WIDTH, BITMAP_HEIGHT, SWP_SHOWWINDOW);

    ShowWindow(hwnd, SW_SHOWNORMAL);
    return dc;
}


// ������� �������� ������
void DeleteDisplay(){
    DeleteDC(memDC);
    DeleteObject(memBM);
}

// ��������� ����������
void FillLB(int start, int count, int value){
    for(int i = start; i <= start+count-1; i++)
        pBmp[i] = value;
}


// ��������� ���������� ��������
void FillLBrnd(){
    for(int i = 0; i < SIZE; i++)
        if(pBmp[i] != 1)
            pBmp[i] = rand()%(256);
}


// �������� ���� �������
int GetPixelgl(int x, int y){
    //printf("%d", pBmp[y*BITMAP_WIDTH + x]);
    return pBmp[y*BITMAP_WIDTH + x];
}


// ������� ���������� �� �����
void Draw(HDC dc){
    BitBlt(dc, 0, 0, BITMAP_WIDTH, BITMAP_HEIGHT, memDC, 0, 0, SRCCOPY);
    //BitBlt(dc, 0, 0, BITMAP_WIDTH, BITMAP_HEIGHT, hBmpDC, 0, 0, SRCCOPY);
}

