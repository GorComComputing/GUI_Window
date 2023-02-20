// pff_plus.cpp: ���������� ����� ����� ��� ����������� ����������.
//

//#include "stdafx.h"
#include "fs.h"
#include <stdlib.h>
#include <Windows.h>

// Work buffer;
BYTE buf[512];

void abort2(const char* msg) {
  printf("ABORT! %s\n", msg);
  scanf("%c", &msg);
  abort();
}

//int _tmain(int argc, _TCHAR* argv[])
int testSD()
{
  int tmp;

  srand(GetTickCount());

  // *** Init driver ***

  if(fs_init()) abort2("fs_init");

  // *** Get total space ***

  printf("Total space: %i Mb\n", fs_gettotal());

  // *** Get free space ***

  if(fs_getfree()) abort2("fs_getfree");
  printf("Free space: %i Mb\n", fs_tmp);

  // *** Get file list ***

  strcpy((char*)buf, "");
  if(fs_opendir()) abort2("fs_opendir");

  while(1) {
    // Read File
    if(fs_readdir()) abort2("fs_readdir");

    // No files
    if(FS_DIRENTRY[0] == 0) break;

    // For pritnf
    FS_DIRENTRY[DIR_Attr] = 0;
    printf("%s\n", FS_DIRENTRY);
  }

  // *** Delete file ***

  strcpy((char*)buf, "MYDIR/MYFILE.TXT");
  if(fs_delete()) printf("can't delete MYDIR/MYFILE.TXT\n");
  strcpy((char*)buf, "MYFILE.TXT");
  if(fs_delete()) printf("can't delete MYFILE.TXT\n");
  strcpy((char*)buf, "COPY.TXT");
  if(fs_delete()) printf("can't delete COPY.TXT\n");

  // *** Delete folder ***

  strcpy((char*)buf, "MYDIR");
  if(fs_delete()) printf("can't delete folder\n");

  // *** Create folder ***

  strcpy((char*)buf, "MYDIR");
  if(fs_createdir()) abort2("fs_createdir");

  // *** Create file ***

  strcpy((char*)buf, "MYDIR/MYFILE.TXT"); //
  if(fs_create()) abort2("fs_craete");

  // *** Write to file ***

  for(int i=0; i<10; i++) {
    strcpy((char*)buf, "MYDIR/MYFILE.TXT");
    char txt[64];
    sprintf(txt, "Hello world %i\r\n", i);
    if(fs_write((BYTE*)txt, strlen(txt))) abort2("fs_write");
  }

  // *** Write to file without additional buffer ***

  for(int i=0; i<100; i++) {
    fs_wtotal = 13;
    while(fs_wtotal) {
     fs_write_start();  // Fragment on every sector
     memcpy((char*)fs_file_wbuf, "HELLO WORLD\r\n" + (13-fs_wtotal), fs_file_wlen);
     fs_write_end();
    }
  }

  // *** Seek ***

 if(fs_lseek(-10, 2)) abort2("fs_seek");

  // *** Truncate file ***

  if(fs_write_eof()) abort2("fs_write_eof");

  // *** Move file/dir ***

  if(fs_move("MYDIR/MYFILE.TXT", "MYFILE.TXT")) abort2("fs_move");

  // *** Open file ***

  strcpy((char*)buf, "MYFILE.TXT");
  if(fs_open()) abort2("fs_open");

  // *** Read file ***

  while(1) {
    BYTE txt[65];
    WORD readed;
    if(fs_read(txt, sizeof(txt)-1, &readed)) abort2("fs_read");

    // EOF
    if(readed==0) break;

    txt[readed] = 0;
    printf("%s", txt);
  }
  printf("\r\n");

  // *** Copy file ***

  printf("copy");
  strcpy((char*)buf, "COPY.TXT");
  if(fs_create()) abort2("fs_open");
  fs_swap();
  strcpy((char*)buf, "MYFILE.TXT");
  if(fs_open()) abort2("fs_open");
  while(1) {
    BYTE tmp[512];
    WORD readed;
    if(fs_read(tmp, sizeof(tmp), &readed)) abort2("fs_read");
    if(readed == 0) break;
    fs_swap();
    if(fs_write(tmp, readed)) abort2("fs_write");
    fs_swap();
    printf(".");
  }
  printf("\n");

  // *** End ***

  scanf("%c",&tmp);
	return 0;
}



int testSD2()
{
  int tmp;

  srand(GetTickCount());

  // *** Init driver ***

  if(fs_init()) abort2("fs_init");

  // *** Get total space ***

  printf("Total space: %i Mb\n", fs_gettotal());

  // *** Get free space ***

  if(fs_getfree()) abort2("fs_getfree");
  printf("Free space: %i Mb\n", fs_tmp);

    // *** Delete folder ***

  strcpy((char*)buf, "B");
  if(fs_delete()) printf("can't delete folder\n");

  // *** Get file list ***

  strcpy((char*)buf, "");
  if(fs_opendir()) abort2("fs_opendir");

  while(1) {
    // Read File
    if(fs_readdir()) abort2("fs_readdir");

    // No files
    if(FS_DIRENTRY[0] == 0) break;

    // For pritnf
    FS_DIRENTRY[DIR_Attr] = 0;
    printf("%s\n", FS_DIRENTRY);
  }
}

int testSD3()
{
char SysNameBuffer[512];

	if (GetVolumeInformation("E:\\", 0, 0, 0, 0, 0, SysNameBuffer, sizeof(SysNameBuffer)))
		printf("%s\n",SysNameBuffer);
}
