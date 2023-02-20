/*
It is an open source software to implement FAT file system to
small embedded systems. This is a free software and is opened for education,
research and commercial developments under license policy of following trems.

(C) 2013 vinxru
(C) 2010, ChaN, all right reserved.

It is a free software and there is NO WARRANTY.
No restriction on use. You can use, modify and redistribute it for
personal, non-profit or commercial use UNDER YOUR RESPONSIBILITY.
Redistributions of source code must retain the above copyright notice.

Version 0.99 5-05-2013
*/

#ifndef _FS_H_
#define _FS_H_

#include "common.h"

/* Disable checking and kill the file system */
/* � ������� ����� ������� ����� ��������� ��������, ��� �� ���������� ��� � ����� �������� �������.*/
/* #define FS_DISABLE_CHECK */

/* Two opened files */
/* ��� �������� ����� */
/* #define FS_DISABLE_SWAP */

/* Calculation of free disk space */
/* ����������� ���������� ����� �� ����� */
/* #define FS_DISABLE_GETFREESPACE */

/* Work buffer for filesystem */
/* ����� ��� ������ �������� ������� */
extern BYTE buf[512];

/* Error */
/* ������ �������� ������� */

#define ERR_OK              0  // ��� ������
#define ERR_NO_FILESYSTEM   1  // �������� ������� �� ����������
#define ERR_DISK_ERR        2  // ������ ������/������
#define	ERR_NOT_OPENED      3  // ����/����� �� �������
#define	ERR_NO_PATH         4  // ����/����� �� �������
#define ERR_DIR_FULL        5  // ����� �������� ������������ ���-�� ������
#define ERR_NO_FREE_SPACE   6  // ��� ���������� �����
#define ERR_DIR_NOT_EMPTY   7  // ������ ������� �����, ��� �� �����
#define ERR_FILE_EXISTS     8  // ����/����� � ����� ������ ��� ����������
#define ERR_NO_DATA         9  // fs_file_wtotal=0 ��� ������ ������� fs_write_begin

#define ERR_MAX_FILES       10 // �� ������������ �������� ��������, ������
#define ERR_RECV_STRING     11 // �� ������������ �������� ��������, ������
#define ERR_INVALID_COMMAND 12 // �� ������������ �������� ��������, ������

#define ERR_ALREADY_OPENED  13 // ���� ��� ������ (fs_swap)

/* Filesystem variables. Can change */
/* ���������� �������� �������. ����� �������� */

extern DWORD fs_tmp;
extern BYTE lastError;       /* ��������� ������ �������� ������� ��� ����� */
extern WORD fs_wtotal;       /* ������������ ��������� fs_write_start, fs_write_end*/

/* Maximal length of file name */
/* ������������ ����� ����� */

#define FS_MAXFILE  469

/* Result of fs_readdir */
/* ��������� ����������� �������� fs_readdir */

#define FS_DIRENTRY  (buf + 480)

/* Functions. Returns not 0 if error occured. Destroy buf variable*/
/* ��������. ��������� �� 0, ���� ������. ��� ������� ������ buf */

BYTE fs_init();                                  /* ������������� �������� ������� / Init filesystem */
BYTE fs_check();                                 /* �������� ������� �����, � ���� �����, �� ��� ������������� / Checking the disk and, if necessary, it will be initialized */
BYTE fs_readdir();                               /* ��������� ��������� ����� � DIRENTRY / Read folder contents in DIRENTRY */
BYTE fs_delete();                                /* ������� ���� ��� �����, ��� � buf / Delete file or folder, name in buf */
BYTE fs_open0(BYTE what);                        /* �������/������� ���� ��� �����, ��� � buf. Open/create file or foder, name in buf */
BYTE fs_move0();                                 /* ����������� ����/�����. Move file or folder, source file must been opened, destination name in buf */
BYTE fs_move(const char* from, const char* to);  /* ����������� ����/�����. Move file or folder. */
BYTE fs_lseek(DWORD ptr, BYTE mode);             /* ���������� ��������� ������/������ �����, ����������� ������ �����. / Set file pointer, enlarge file size. */
BYTE fs_tell();                                  /* �������� ��������� ������/������ ����� � fs_tmp / Get file pointer in fs_tmp. */
BYTE fs_getfilesize();                           /* �������� ������ ����� / Get file size. */
BYTE fs_read0(BYTE* ptr, WORD len);              /* ��������� �� �����. ������ �������� �� ������� �����! / Read from the file, DO NOT COME OUT FILE */
BYTE fs_read(BYTE* ptr, WORD len, WORD* readed); /* ��������� �� �����. / Read from the file. */
BYTE fs_write(CONST BYTE* ptr, WORD len);        /* �������� � ����, ����������� ������ �����. / �������� � ����, enlarge file size. */
BYTE fs_write_eof();                             /* ���������� ����� �����, ��������� ������ �����. / Set end of file, reduces file size */
BYTE fs_gettotal();                              /* ����� ����� �� ����� � fs_tmp � ���������� / Total disk space in fs_tmp (megabytes) */

#ifndef FS_DISABLE_SWAP
void fs_swap();                                  /* ������������ �� ������ ���� / Switch to the second file */
#endif

#ifndef FS_DISABLE_GETFREESPACE
BYTE fs_getfree();                               /* ��������� ����� �� ����� � fs_tmp  � ���������� / Free disk space in fs_tmp (megabytes) */
#endif


/* ������� ���� ��������� ������������ ��� ������ ����� (buf).
*  The functions below can be used to write the buffer (buf).
*
*  wtotal = ������ ������ ��� ������;
*  while(wtotal) {
*    fs_write_start(&wtotal);
*    �������� fs_file_wlen ���� � fs_file_wbuf
*    �� ���� ������� �� �� ������ ���������� ����� ����
*    fs_write_end(&wtotal);
*  }
*
* See the source of fs_write
* ��� ������ ��������� fs_write
*/

#define fs_file_wlen   (*(WORD*)&fs_tmp)
#define fs_file_woff   ((WORD*)&fs_tmp)[1]
#define fs_file_wbuf   (buf + fs_file_woff)

BYTE fs_write_start();
BYTE fs_write_end();

/* Derived from the function fs_open0 */
/* ����������� �� ������� fs_open0 */

BYTE fs_open();                                  /* ������� ���� */
BYTE fs_opendir();                               /* ������� ����� */
#define fs_openany()    fs_open0(OPENED_NONE)    /* ������� ���� ��� ����� */
#define fs_create()     fs_open0(OPENED_FILE)    /* ������� ���� */
#define fs_createdir()  fs_open0(OPENED_DIR)     /* ������� ����� */


/* Values ??fs_opened */
/* �������� fs_opened */

#define OPENED_NONE    0
#define OPENED_FILE    1
#define OPENED_DIR     2

/* File attributes */
/* �������� ������ */

#define	AM_RDO         0x01  /* Read only */
#define	AM_HID         0x02  /* Hidden */
#define	AM_SYS         0x04  /* System */
#define	AM_VOL         0x08  /* Volume label */
#define AM_DIR         0x10  /* Directory */
#define AM_ARC         0x20  /* Archive */

/* Fodler descriptor (FS_DIRENTRY) */
/* ��������� */

#define	DIR_Name       0
#define	DIR_Attr       11
#define	DIR_NTres      12
#define	DIR_CrtTime    14
#define	DIR_CrtDate    16
#define	DIR_FstClusHI  20
#define	DIR_WrtTime    22
#define	DIR_WrtDate    24
#define	DIR_FstClusLO  26
#define	DIR_FileSize   28

#endif
