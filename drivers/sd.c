// ������� Flash-�����
// ������ 4 ��, ���� � ������� 512
// FAT32
// � MBR ���������� � �������� ������ 1BEh
// 8-� ���� ��������� �� ������ ������ ������� ������� (4 ����� � 1C6h �� 1C9h)
// ��������� Hex2Dec (0000h 003Fh = 63)
// Ctrl+G ������� � ������ 63 LBA - ��� ����������� ������ �������
// � 3h-�� ������ 8 ���� - ������ ������������ ������� (MSDOS5.0)
// �� ������ Dh - ���������� �������� �� ������� (08h) 08*512=4096 (4�� ������ ��������)
// �� ������ Eh (2 �����) ���������� �������� ����� ������ FAT 0448h = 1096, 1096*512=561152
// �� ������ 10h ���������� FAT ������ (02h)
// ���������� �������� ���������� ����� FAT �� ������ 24h (2 �����) = 1DDCh = 7644 ��������
// ������ ������� FAT 63+1096=1159 (Ctrl+G) (F8FFFF0F FFFFFFFF 39680200...)
// ��������� ������� FAT ����� 7644 �������� 1159+7644=8803 (F8FFFF0F FFFFFFFF 39680200...������ ����� ������ FAT)
// ������� ��� ���� ������ FAT 7644, ������� ����� �������� ���������� 8803+7644=16447
// �������� ���������� �������� �������� ������
// � ������ 40h (11 ����) - ��� �����
// �� ������ 2Dh ��� �����
// �� ������ 5Ch (4 �����) - ������ �����, 0000 0000 = ������
// �� ������ 56h (2 �����) - ����� �����, ������ 5 ��� (� �����) - ������� (����� �������� �� 2, ������ ��� ������ ���� �� 2 �������)
// � 6 �� 10 ��� - ������,
// �� ������ 58h (2 �����) - ���� �����
#include <stdio.h>
#include <stdlib.h>

#define flash

typedef unsigned char   BYTE;
typedef unsigned short  WORD;
typedef unsigned long	DWORD;

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




#include <Windows.h>


// Work buffer;
BYTE buf[512];


HANDLE handle = INVALID_HANDLE_VALUE;


BYTE sd_init() {//fat16.vhd  //   \\\\.\\G:
  handle = CreateFile(TEXT("\\\\.\\PhysicalDrive1"),    // ����������� ����
                   GENERIC_READ, FILE_SHARE_READ, NULL,
            OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

                   /*GENERIC_ALL,//GENERIC_READ | GENERIC_WRITE,          // ��������� ��� ������
                   FILE_SHARE_READ | FILE_SHARE_WRITE,       // ��� ����������� ������
                   NULL,                  // ������ �� ���������
                   OPEN_EXISTING,         // ������ ������������ ����
                   NULL,//FILE_ATTRIBUTE_NORMAL, // ������� ����
                   NULL);*/
  if(handle==INVALID_HANDLE_VALUE) { lastError = ERR_DISK_ERR; return 1; }
	return 0;
}


BYTE sd_read(BYTE* dest,	DWORD sector,	WORD sofs, WORD count) {
  if(SetFilePointer(handle, sector*512+sofs, 0, FILE_BEGIN)==INVALID_SET_FILE_POINTER) { lastError=ERR_DISK_ERR; return 1; }
  DWORD tmp;
  if(!ReadFile(handle, dest, count, &tmp, 0) || tmp!=count) { lastError=ERR_DISK_ERR; return 1; }
	return 0;
}


BYTE sd_write512(BYTE* dest,	DWORD sect) {
  if(SetFilePointer(handle, sect*512, 0, FILE_BEGIN)==INVALID_SET_FILE_POINTER) { lastError=ERR_DISK_ERR; return 1; }
  DWORD tmp;
  if(!WriteFile(handle, dest, 512, &tmp, 0) || tmp!=512) { lastError=ERR_DISK_ERR; return 1; }
  return 0;
}

void abort2(const char* msg) {
  printf("ABORT! %s\n", msg);
  scanf("%c", &msg);
  abort();
}



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
//BYTE fs_opendir();                               /* ������� ����� */
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









/* ��� ����������� */

#define	LD_WORD(ptr)    (*(WORD*)(ptr))
#define	LD_DWORD(ptr)   (*(DWORD*)(ptr))

/* �������� fs_type */

#define FS_FAT16	0
#define FS_FAT32	1
#define FS_ERROR	2

/* ����������� �������� ��������� */

#define FREE_CLUSTER    0
#define LAST_CLUSTER    0x0FFFFFFF

/* ��������� ����������. ���������� �� boot-������� */

typedef struct {
#ifndef FS_DISABLE_CHECK
  BYTE  opened;              /* ��� �������: OPENED_NONE, OPENED_FILE, OPENED_DIR */
#endif
  BYTE  entry_able;          /* ��������� ���������� ������� fs_dirread */
  WORD  entry_index;         /* ����� ������ � �������� */
  DWORD entry_cluster;       /* ������� ������ � �������� */
  DWORD entry_sector;        /* ������ ������ � �������� */
  DWORD entry_start_cluster; /* ������ ������ ����� ��� �������� (0 - �������� ������� FAT16) */
  DWORD ptr;                 /* ��������� ������/������ �����*/
  DWORD size;                /* ������ ����� / File size */
  DWORD cluster;             /* ������� ������� ����� */
  DWORD sector;              /* ������� ������ ����� */
  BYTE  changed;             /* ������ ����� ���������, ���� ��������� */
} File;

BYTE  fs_type;         /* FS_FAT16, FS_FAT32, FS_ERROR */
DWORD fs_fatbase;      /* ����� ������ FAT */
DWORD fs_fatbase2;     /* ����� ������ FAT */
BYTE  fs_csize;        /* ������ �������� � �������� */
WORD  fs_n_rootdir;    /* ���-�� ������� � �������� �������� (������ FAT16) */
DWORD fs_n_fatent;     /* ���-�� ��������� */
DWORD fs_dirbase;      /* �������� ������� (������ ��� FAT16, ������� ��� FAT32) */
DWORD fs_database;     /* ����� ������� �������� */

/* ��������� ����������. ��������� */

BYTE  lastError;       /* ��������� ������ */
DWORD fs_fatoptim;     /* ������ ��������� ������� */
DWORD fs_tmp;          /* ������������� ��� ������ ����� */
WORD  fs_wtotal;       /* ������������ ��������� fs_write_start, fs_write_end*/

/* �������� �����/����� */

File fs_file;

#ifndef FS_DISABLE_SWAP
File fs_secondFile;
#endif

/* ��������� boot-������� */

#define BPB_SecPerClus    13
#define BPB_RsvdSecCnt    14
#define BPB_NumFATs       16
#define BPB_RootEntCnt    17
#define BPB_TotSec16      19
#define BPB_FATSz16       22
#define BPB_TotSec32      32
#define BS_FilSysType     54
#define BPB_FATSz32       36
#define BPB_RootClus      44
#define BS_FilSysType32   82
#define MBR_Table         446


/**************************************************************************
*  ������ ������� � �����                                                 *
**************************************************************************/

static BYTE sd_readBuf(DWORD sector) {
  return sd_read(buf, sector, 0, 512);
}

/**************************************************************************
*  ������ ������ � ������                                                 *
**************************************************************************/

static BYTE sd_writeBuf(DWORD sector) {
  return sd_write512(buf, sector);
}

/**************************************************************************
*  �������������                                                          *
**************************************************************************/

BYTE fs_init() {
  DWORD bsect, fsize, tsect;

  /* ���������� ����������� */
  fs_fatoptim = 2;

  /* ������������� ������ ������������ */
/*#ifndef FS_DISABLE_CHECK
  fs_file.opened = OPENED_NONE;
#ifndef FS_DISABLE_SWAP
  fs_secondFile.opened = OPENED_NONE;
#endif
  fs_type = FS_ERROR;
#endif*/

  /* ������������� ���������� */
  if(sd_init()) return 1;

/* ���� �������� ������� */
  bsect = 0;
  while(1) {
    if(sd_readBuf(bsect)) return 1;
    if(LD_WORD(buf + 510) == 0xAA55) {
      if(LD_WORD(buf + BS_FilSysType  ) == 0x4146) break;
      if(LD_WORD(buf + BS_FilSysType32) == 0x4146) break;
      /* �������� ��� MBR */
      if(bsect == 0 && buf[MBR_Table+4]) {
        bsect = LD_DWORD(buf + (MBR_Table + 8));
        if(bsect != 0) continue;
      }
    }
abort_noFS:
    printf("NO FS");
    lastError = ERR_NO_FILESYSTEM; return 1;
  }

  /* ������ ������� FAT � �������� */
  fsize = LD_WORD(buf + BPB_FATSz16);
  if(fsize == 0) fsize = LD_DWORD(buf + BPB_FATSz32);

  /* ������ �������� ������� � �������� */
  tsect = LD_WORD( buf + BPB_TotSec16);
  if(tsect == 0) tsect = LD_DWORD(buf + BPB_TotSec32);

  /* ������ ��������� �������� (������ ���� ������ 16 � ��� FAT32 ������ ���� ���� ����) */
  fs_n_rootdir = LD_WORD(buf + BPB_RootEntCnt);

  /* ������ ������ FAT � �������� */
  fs_fatbase  = bsect + LD_WORD(buf + BPB_RsvdSecCnt);
  fs_fatbase2 = 0;
  if(buf[BPB_NumFATs] >= 2) fs_fatbase2 = fs_fatbase + fsize;

  /* ���-�� �������� �� ������� */
  fs_csize = buf[BPB_SecPerClus];

  /* ���-�� ��������� */
  fsize *= buf[BPB_NumFATs];
  fs_n_fatent = (tsect - LD_WORD(buf + BPB_RsvdSecCnt) - fsize - fs_n_rootdir / 16) / fs_csize + 2;

  /* ����� 2-��� �������� */
  fs_database = fs_fatbase + fsize + fs_n_rootdir / 16;

  /* ����������� �������� ������� */

  /* FAT 12 */
  if(fs_n_fatent < 0xFF7) goto abort_noFS;

  /* FAT 16 */
  if(fs_n_fatent < 0xFFF7) {
    printf("FAT 16");
    fs_dirbase = fs_fatbase + fsize;
    fs_type = FS_FAT16;
    return 0;
  }

  /* FAT 32 */
  printf("FAT 32");
  fs_dirbase = LD_DWORD(buf + BPB_RootClus);

  /* ���������� ������� ���������� ����� */
  if(LD_WORD(buf + BPB_RsvdSecCnt)>0) {
    bsect++;
    if(sd_readBuf(bsect)) return 1;
    if(LD_DWORD(buf) == 0x41615252 && LD_DWORD(buf + 0x1E4) == 0x61417272 && LD_DWORD(buf + 0x1FC) == 0xAA550000) {
      LD_DWORD(buf + 0x1E8) = 0xFFFFFFFF;
      LD_DWORD(buf + 0x1EC) = 0xFFFFFFFF;
      if(sd_writeBuf(bsect)) return 1;
    }
  }
  fs_type = FS_FAT32;

  return 0;
}



/**************************************************************************
*  �������� �������                                                       *
*                                                                         *
*  ��������� ������� ����������� � fs_tmp                                 *
**************************************************************************/

/* ���� ������� fs_getfree �������� ��������� ������� fs_allocCluster.
���� ������� �� ������������, �� ����� � ������� ������� FS_DISABLE_GETFREESPACE
��������� ������ ��� */

#ifdef FS_DISABLE_GETFREESPACE
#define DIS(X)
#define ALLOCCLUSTER
#else
#define DIS(X) X
#define ALLOCCLUSTER 0
#endif

static BYTE fs_allocCluster(DIS(BYTE freeSpace)) {
  BYTE i;
  DWORD s;
  BYTE *a;

  /* �������� ����� � ����� �������� */
  fs_tmp = fs_fatoptim;

  /* ��������������� ���������� ������� */
  while(1) {
    /* ������ � �������� */
    s = fs_tmp / 256, i = (BYTE)fs_tmp, a = (BYTE*)((WORD*)buf + i);
    if(fs_type != FS_FAT16) s = fs_tmp / 128, i |= 128, a = (BYTE*)((DWORD*)buf - 128 + i);

    /* ������ ������ */
    if(sd_readBuf(fs_fatbase + s)) goto abort;

    /* ����� 128/256 ����� � ������� ���� 0 */
    /* ���� �������� ������ ����� �� ����� ������� �������, �� ���� ���������� ����� ���������� ���. */
    do {
      /* �������� ��������� */
      if(fs_tmp >= fs_n_fatent) { DIS(if(freeSpace) return 0;) lastError = ERR_NO_FREE_SPACE; goto abort; }

      /* ���� ��������� ������� � �������� ��� ��������� */
      if(fs_type == FS_FAT16) {
        if(LD_WORD(a) == 0) { DIS(if(!freeSpace) {) LD_WORD(a) = (WORD)LAST_CLUSTER; goto founded; DIS(} fs_file.sector++;) }
        a += 2;
      } else {
        if(LD_DWORD(a) == 0) { DIS(if(!freeSpace) {) LD_DWORD(a) = LAST_CLUSTER; goto founded; DIS(} fs_file.sector++;) }
        a += 4;
      }

      /* ������� */
      ++fs_tmp, ++i;
    } while(i != 0);
  }
founded:
  /* ����������� */
  fs_fatoptim = fs_tmp;

  /* ��������� ��������� */
  return fs_saveFatSector(s);
abort:
  return 1;
}

#undef DIS


/**************************************************************************
*  ������ ���������� �����                                                *
*                                                                         *
*  ��������� � ���������� fs_tmp � ����������                             *
*  ������� ��������� ����                                                 *
**************************************************************************/

#ifndef FS_DISABLE_GETFREESPACE
BYTE fs_getfree() {
  /* �� �������� ���������� fs_file.sector, ������� ��������� ���� */
  fs_file.opened = OPENED_NONE;

  /* ���-�� ��������� ��������� ����� � fs_file.sector */
  fs_file.sector = 0;
  if(fs_allocCluster(1)) return 1;

  /* �������� � ��������� */
  fs_tmp = ((fs_file.sector >> 10) + 1) / 2 * fs_csize;

  return 0;
}
#endif

/**************************************************************************
*  ������ ���������� � ����������                                         *
**************************************************************************/

BYTE fs_gettotal() {
  /* �������� ������ ������������ */
#ifndef FS_DISABLE_CHECK
  if(fs_type == FS_ERROR) { lastError = ERR_NO_FILESYSTEM; return 1; }
#endif

  fs_tmp = ((fs_n_fatent >> 10) + 1) / 2 * fs_csize;
  return 0;
}


/**************************************************************************
*  �������/������� ���� ��� �����                                         *
*                                                                         *
*  ��� ����� � buf. ��� �� ������ ��������� FS_MAXFILE ������� ������� 0  *
*                                                                         *
*  what = OPENED_NONE - ������� ���� ��� �����                            *
*  what = OPENED_FILE - ������� ���� (��������� ���� ������)              *
*  what = OPENED_DIR  - ������� ����� (��������� ����� �� �������)        *
*  what | 0x80        - �� ��������� ���� � ����� entry_start_cluster     *
*                                                                         *
*  �� ������                                                              *
*   FS_DIRENTRY                 - ���������                               *
*   fs_file.entry_able          - 0 (���� ������ ������������ ����/�����) *
*   fs_file.entry_sector        - ������ ���������                        *
*   fs_file.entry_cluster       - ������� ���������                       *
*   fs_file.entry_index         - ����� ���������                         *
*   fs_file.entry_start_cluster - ������ ������� ����� ��� �����          *
*   fs_parent_dir_cluster       - ������ ������� ����� ������ (CREATE)    *
*   fs_file.ptr                 - 0 (���� ������ ����)                    *
*   fs_file.size                - ������ ����� (���� ������ ����)         *
*                                                                         *
*  ������� �� ������ buf[0..MAX_FILENAME-1]                               *
**************************************************************************/

static BYTE fs_open0_create(BYTE dir); /* forward */
static CONST BYTE* fs_open0_name(CONST BYTE *p); /* forward */

#define FS_DIRFIND      (buf + 469)           /* 11 ���� �������������� ������ ������� fs_open0 */
#define fs_notrootdir (*(BYTE*)&fs_file.size) /* ������������ fs_open0, � ��� ����� ���������� fs_file. �� ������� ������ �������� */
#define fs_parent_dir_cluster fs_file.sector  /* ��� �� ������������ fs_file.sector ��� �������� ������� �������� ����� ������. */

BYTE fs_open0(BYTE what) {
  CONST BYTE *path;
  BYTE r;

  /* �������� ������ ������������ */
#ifndef FS_DISABLE_CHECK
  if(fs_type == FS_ERROR) { lastError = ERR_NO_FILESYSTEM; goto abort; }
  fs_file.opened = OPENED_NONE;
#endif

  /* �������������� �������� */
  r = what & 0x80; what &= 0x7F;
  fs_parent_dir_cluster = fs_file.entry_start_cluster;

  /* �������� ������� */
  fs_notrootdir = 0;
  fs_file.entry_start_cluster = fs_dirbase;
  if(fs_type == FS_FAT16) fs_file.entry_start_cluster =  0;

  /* �������� ����� */
  if(buf[0] == 0) {
    if(what) goto abort_noPath;
    FS_DIRENTRY[0] = 0;             /* ������� �������� ����� */
    FS_DIRENTRY[DIR_Attr] = AM_DIR; /* ��� ��������� ����������� ����/����� ������� ���� AM_DIR */
  } else {
    path = buf;
    while(1) {
      /* �������� ��������� ��� �� path � FS_DIRFIND */
      path = fs_open0_name(path);
      if(path == (CONST BYTE*)1) goto abort_noPath;
      /* ���� ��� � ����� */
      fs_file.entry_able = 0;
      while(1) {
        if(fs_readdir_nocheck()) return 1;
        if(fs_file.entry_able == 0) break;
        if(!memcmp(FS_DIRENTRY, FS_DIRFIND, 11)) break;
      }
      /* ��������� �������� ���� � ������ �������� */
      if(what && path == 0) {
        fs_parent_dir_cluster = fs_file.entry_start_cluster; /* ��������� � ���� ���������� ��������� ��� ������ fs_move */
        if(fs_type == FS_FAT32 && fs_parent_dir_cluster == fs_dirbase) fs_parent_dir_cluster = 0;
        if(fs_file.entry_able == 0) return fs_open0_create(what-1); /* ��������� ��� */
        lastError = ERR_FILE_EXISTS; goto abort;
      }
      /* ����/����� �� ������� */
      if(fs_file.entry_able == 0) goto abort_noPath;

      /* ��� �� ������� */
      fs_file.entry_start_cluster = fs_getEntryCluster();
      /* ��� ��� ��������� ������� ���� */
      if(path == 0) break;
      /* ��� ������ ���� ����� */
      if((FS_DIRENTRY[DIR_Attr] & AM_DIR) == 0) goto abort_noPath;
      /* ������������� �������� ��� ������� fs_move */
      if(r && fs_file.entry_start_cluster == fs_parent_dir_cluster) goto abort_noPath;
      /* �������� ����� ��� �� ����� �������� */
      fs_notrootdir = 1;
    }
  }
  /* ������������� ���������� */
  fs_file.entry_able = 0;
  fs_file.size  = LD_DWORD(FS_DIRENTRY + DIR_FileSize);
  fs_file.ptr   = 0;
#ifndef FS_DISABLE_CHECK
  fs_file.opened     = OPENED_FILE;
  if(FS_DIRENTRY[DIR_Attr] & AM_DIR) fs_file.opened = OPENED_DIR;
#endif

  /* ������ ������ ��������� ���� */
#ifndef FS_DISABLE_CHECK
#ifndef FS_DISABLE_SWAP
  if(fs_secondFile.opened==OPENED_FILE && fs_file.opened==OPENED_FILE && fs_secondFile.entry_sector == fs_file.entry_sector && fs_secondFile.entry_index==fs_file.entry_index) {
    fs_file.opened  = OPENED_NONE;
    lastError = ERR_ALREADY_OPENED;
    goto abort;
  }
#endif
#endif

  return 0;
abort_noPath:
  lastError = ERR_NO_PATH;
abort:
  return 1;
}

/*static BYTE exists(const flash BYTE* str, BYTE c) {
  while(*str)
    if(*str++ == c)
      return c;
  return 0;
}*/

static CONST BYTE * fs_open0_name(CONST BYTE *p) {
  BYTE c, ni, i;

  memset(FS_DIRFIND, ' ', 11);
  i = 0; ni = 8;
  while(1) {
    c = *p++;
    if(c == 0) {
      if(i == 0) break; /* ������ ��� ����� */
      return 0;
    }
    if(c == '/') return p;
    if(c == '.') {
      if(i == 0) break; /* ������ ��� ����� */
#ifndef FS_DISABLE_CHECK
      if(ni != 8) break; /* ������ ����� */
#endif
      i = 8; ni = 11;
      continue;
    }
    /* ������� ������� ��� */
    if(i == ni) break;
    /* ����������� ������� */
#ifndef FS_DISABLE_CHECK
    if(exists((const flash BYTE* )"+,;=[]*?<:>\\|\"", c)) break;
    if(c <= 0x20) break;
    if(c >= 0x80) break;
#endif
    /* �������� � �������� �������� */
    if(c >= 'a' && c <= 'z') c -= 0x20;
    /* ��������� ��� */
    FS_DIRFIND[i++] = c;
  }
  /* ������ */
  return (CONST BYTE*)1;
}

static BYTE fs_open0_create(BYTE dir) {
  BYTE  new_name[11];
  DWORD allocatedCluster;
  BYTE* allocatedEntry;

  /* ��������� ���, ��� ��� ���� ����� ����� ������ */
  memcpy(new_name, FS_DIRFIND, 11);

  /* �������� ������� ��� ����� */
  if(dir) {
    if(fs_allocCluster(ALLOCCLUSTER)) goto abort; /* fs_file.entry_start_cluster ������� �� �����, ��� ������ ������� ����� � ������� �� �������� ���� */
    allocatedCluster = fs_tmp;
  }

  /* ��������� � ����� ��������� (������ �� ��������) */
  allocatedEntry = fs_allocEntry();
  if(allocatedEntry == 0) {

    /* � ������ ������ ����������� ������� */
    fs_tmp = FREE_CLUSTER;
    fs_setNextCluster(allocatedCluster);
    goto abort;
  }

  /* ������������ ��� � ���������. */
  memset(allocatedEntry, 0, 32);
  memcpy(allocatedEntry, new_name, 11);

  if(!dir) {
    /* ��������� ��������� �� ���� */
    if(sd_writeBuf(fs_file.entry_sector)) goto abort;
    /* fs_file.entry_sector, fs_file.entry_index - �������������� � fs_allocCluster */
    fs_file.entry_start_cluster = 0;
    fs_file.size           = 0;
    fs_file.ptr            = 0;
#ifndef FS_DISABLE_CHECK
    fs_file.opened              = OPENED_FILE;
#endif
    return 0;
  }

  /* ��� ����� */
  allocatedEntry[DIR_Attr] = AM_DIR;
  fs_setEntryCluster(allocatedEntry, allocatedCluster);

  /* ��������� ��������� �� ���� */
  if(sd_writeBuf(fs_file.entry_sector)) goto abort;

  /* ������ ��� ����� ����� */
  fs_tmp = allocatedCluster;
  fs_clust2sect();

  /* ������� fs_tmp � �� ���� buf */
  fs_eraseCluster(1);

  /* ������� ������ ����� */
  memset(buf, ' ', 11); buf[0] = '.'; buf[11] = 0x10;
  fs_setEntryCluster(buf, allocatedCluster);

  memset(buf+32, ' ', 11); buf[32] = '.'; buf[33] = '.'; buf[32+11] = 0x10;
  if(fs_notrootdir) fs_setEntryCluster(buf + 32, fs_file.entry_start_cluster); /* ������ � fs_file.size==0 ������ �������� ������� */

  /* ��������� ����� */
  return sd_writeBuf(fs_tmp);
abort:
  return 1;
}


/**************************************************************************
*  ������� ����                                                           *
**************************************************************************/

BYTE fs_open() {
  if(fs_openany()) goto abort;
#ifndef FS_DISABLE_CHECK
  if(fs_file.opened == OPENED_FILE) return 0;
  fs_file.opened = OPENED_NONE;
#else
  if((FS_DIRENTRY[DIR_Attr] & AM_DIR) == 0) return 0;
#endif
  lastError = ERR_NO_PATH;
abort:
  return 1;
}


/**************************************************************************
*  ������� �����                                                          *
**************************************************************************/

BYTE fs_opendir() {
  if(fs_openany()) goto abort;
#ifndef FS_DISABLE_CHECK
  if(fs_file.opened == OPENED_DIR) return 0;
  fs_file.opened = OPENED_NONE;
#else
  if(FS_DIRENTRY[DIR_Attr] & AM_DIR) return 0;
#endif
  lastError = ERR_NO_PATH;
abort:
  return 1;
}



void testSD(){
    int tmp;

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
}


void testSD2(){

    //fat16.vhd    \\\\.\\G:
    HANDLE hDisk = CreateFile(TEXT("\\\\.\\PhysicalDrive6"),    // ����������� ����
                   GENERIC_READ | GENERIC_WRITE,          // ��������� ��� ������
                   FILE_SHARE_READ | FILE_SHARE_WRITE,       // ��� ����������� ������
                   NULL,                  // ������ �� ���������
                   OPEN_EXISTING,         // ������ ������������ ����
                   FILE_ATTRIBUTE_NORMAL, // ������� ����
                   NULL);
    if (hDisk == INVALID_HANDLE_VALUE) abort2("fs_init2");

    BYTE buf[512];
    DWORD dwRead;
    if (!ReadFile(hDisk, buf, sizeof(buf), &dwRead, NULL) || dwRead != sizeof(buf)) abort2("read");
    CloseHandle(hDisk);



    for(int i = 0; i<512;i++)
    {
        char f[3];
        itoa(buf[i],f,16);
        printf("%d\t%x\n", i, buf[i]);

    }



}
