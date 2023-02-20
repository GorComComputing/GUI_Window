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

Version 0.99 30-05-2013

P.S. goto allows you to save memory! Like other horrors bellow.

Program size: 3070 words (6140 bytes), 75% of FLASH at ATMega8 !!!
*/

/*
� �� ���� ��������� �������� �� ����������� �����
CON,PRN,AUX,CLOCK$,NUL,COM1,COM2,COM3,COM4,LPT1,LPT2,LPT3
��� �� �� �������� ���������������. ����� � ������ �������
�������� �� ������� ������������.
*/

//#include <stdafx.h>
#include "fs.h"
#include "sd.h"
#include <string.h>

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
#ifndef FS_DISABLE_CHECK
  fs_file.opened = OPENED_NONE;
#ifndef FS_DISABLE_SWAP
  fs_secondFile.opened = OPENED_NONE;
#endif
  fs_type = FS_ERROR;
#endif

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
    fs_dirbase = fs_fatbase + fsize;
    fs_type = FS_FAT16;
    return 0;
  }

  /* FAT 32 */
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
*  �������� ������� ����� � ���� �����, �� ��� �������������              *
**************************************************************************/

BYTE fs_check() {
  if(!sd_check()) return 0;
  return fs_init();
}

/**************************************************************************
*  �������� ������� �� FS_DIRENTRY                                        *
*  ������� �� ������ buf (�������, � ������� ����� �� ��������, ������)   *
**************************************************************************/

static DWORD fs_getEntryCluster() {
  DWORD c = LD_WORD(FS_DIRENTRY + DIR_FstClusLO);
  if(fs_type != FS_FAT16) c |= ((DWORD)LD_WORD(FS_DIRENTRY + DIR_FstClusHI)) << 16;
  return c;
}

/**************************************************************************
*  �������� ��������� �������.                                            *
*  �������� � ��������� ��������� � fs_tmp.                               *
**************************************************************************/

static BYTE fs_nextCluster() {
  if(fs_type == FS_FAT16) {
    if(sd_read((BYTE*)&fs_tmp, fs_fatbase + fs_tmp / 256, (WORD)(BYTE)fs_tmp * 2, 2)) goto abort;
    fs_tmp &= 0xFFFF;
  } else {
    if(sd_read((BYTE*)&fs_tmp, fs_fatbase + fs_tmp / 128, (WORD)((BYTE)fs_tmp % 128) * 4, 4)) goto abort;
    fs_tmp &= 0x0FFFFFFF;
  }
  /* ��� �������� ���������� �������� ��������� ������� �� ����. */
  if(fs_tmp < 2 || fs_tmp >= fs_n_fatent)
    fs_tmp = 0;
  return 0;
abort:
  return 1;
}

/**************************************************************************
*  ������������� ����� �������� � ����� �������                           *
*  �������� � ��������� ��������� � fs_tmp.                               *
*  ������� �� ������ buf                                                  *
***************************************************************************/

static void fs_clust2sect() {
  fs_tmp = (fs_tmp - 2) * fs_csize + fs_database;
}

/**************************************************************************
*  �������� ��������� ���� ��� �����                                      *
*  ��������� �����, ����� ����, ��������� �������, LFN ������������       *
*                                                                         *
*  �������� ������ ��������� ��������� ������� ����, ������� ����� ��     *
*  ���������                                                              *
*                                                                         *
*  ������� �� ������ buf[0..MAX_FILENAME-1]                               *
***************************************************************************/

static BYTE fs_readdirInt() {
  if(fs_file.entry_able) {
    fs_file.entry_index++;

    /* � ����� �� ����� ���� ������ 65536 ������, � � ����� FAT16 �� ������  fs_n_rootdir */
    if(fs_file.entry_index == 0 || (fs_file.entry_cluster == 0 && fs_file.entry_index == fs_n_rootdir)) {
      fs_file.entry_index = 0;
retEnd:
      FS_DIRENTRY[DIR_Name] = 0; /* ������� ���������� ����� ��� ������������ ����������� fs_dirread */
      fs_file.entry_able = 0;
      return 0;
    }

    /* ������� ������� */
    if(fs_file.entry_index % 16 == 0) {
      fs_file.entry_sector++;

      /* ������� �������� */
      if(fs_file.entry_cluster != 0 && ((fs_file.entry_index / 16) % fs_csize) == 0) {

        /* ��������� ������� */
        fs_tmp = fs_file.entry_cluster;
        if(fs_nextCluster()) return 1;
        if(fs_tmp == 0) goto retEnd; /* ��������� �������, ������������ fs_file.entry_able = 0 */

        /* ��������� */
        fs_file.entry_cluster = fs_tmp;
        fs_clust2sect();
        fs_file.entry_sector = fs_tmp;
      }
    }
  } else {
    fs_file.entry_index = 0;
    fs_file.entry_able  = 1;
    fs_tmp = fs_file.entry_start_cluster;

    /* ������ ������� � ������ �����. ���� ��� �� ����� ������ ���������
    ��� FAT16, �� ���� ��� ������ ���������. �.�. ���� ����� ���� ����������
    ����� ����. */
    fs_file.entry_cluster = fs_tmp;
    fs_clust2sect();
    fs_file.entry_sector = fs_tmp;

    /* �������� ����� FS_FAT16 */
    if(fs_file.entry_cluster == 0) fs_file.entry_sector = fs_dirbase;
  }

  return sd_read(FS_DIRENTRY, fs_file.entry_sector, (WORD)((fs_file.entry_index % 16) * 32), 32);
}

/**************************************************************************
*  �������� ��������� ���� ��� ����� (��������� ����� ������������)       *
*                                                                         *
*  ��� ������� ���������� ����� fs_opendir, ������� ����������� ���       *
*  ������ ����������, ������� ��� ������ ������� �� ����. ����������      *
*  ���� ��������� ��� ������� ��������� ������                            *
*                                                                         *
*  ���� �� ����� entry_able=0,  �� ���������� ����� ����� � �����         *
*  �� ������ entry_start_cluster. ��� ���� ���������������� ����������:   *
*  entry_able, entry_index, entry_sector, entry_cluster.                  *
*                                                                         *
*  ���� �� ����� fs_file.entry_able=1, �� ������������ ��� 4 ����������.  *
*                                                                         *
*  �� ������                                                              *
*    entry_able     - ���� 0 ������ ��������� ����� ��������              *
*    entry_sector   - ������ ���������                                    *
*    entry_cluster  - ������� ���������                                   *
*    entry_index    - ����� ���������                                     *
*    FS_DIRENTRY    - ���������                                           *
*                                                                         *
*  ������� �� ������ buf[0..MAX_FILENAME-1]                               *
**************************************************************************/

BYTE fs_readdir_nocheck() {
  while(!fs_readdirInt()) {
    if(FS_DIRENTRY[DIR_Name] == 0) fs_file.entry_able = 0;
    if(fs_file.entry_able == 0) return 0;
    if(FS_DIRENTRY[DIR_Name] == 0xE5) continue; /*  ����� ���� ��� 0x05 */
    if(FS_DIRENTRY[DIR_Name] == '.') continue;
    if((FS_DIRENTRY[DIR_Attr] & AM_VOL) == 0) return 0;
  }
  return 1;
}

BYTE fs_readdir() {
#ifndef FS_DISABLE_CHECK
  /* ����� ������ ���� ������� */
  if(fs_file.opened != OPENED_DIR) { lastError = ERR_NOT_OPENED; return 1; }
#endif
  return fs_readdir_nocheck();
}

/**************************************************************************
*  ��������� ��������� � ��� ������� FAT                                  *
**************************************************************************/

static BYTE fs_saveFatSector(DWORD sector) {
  if(fs_fatbase2) if(sd_writeBuf(fs_fatbase2+sector)) return 1;
  return sd_writeBuf(fs_fatbase+sector);
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
*  ��������� ������� FAT                                                  *
*                                                                         *
*  ���� fs_tmp!=0, �� FAT[cluster] = fs_tmp                               *
*  ���� fs_tmp==0, �� swap(FAT[cluster], fs_tmp)                          *
**************************************************************************/

static BYTE fs_setNextCluster(DWORD cluster) {
  DWORD s, prev;
  void* a;

  /* ��������� ������ */
  s = cluster / 128;
  if(fs_type == FS_FAT16) s = cluster / 256;

  /* ������ ������ */
  if(sd_readBuf(fs_fatbase + s)) return 1;

  /* �������� ��������� ������� */
  if(fs_type == FS_FAT16) {
    a = (WORD*)buf + (BYTE)cluster;
    prev = LD_WORD(a);
    LD_WORD(a) = (WORD)fs_tmp;
  } else {
    a = (DWORD*)buf + (BYTE)cluster % 128;
    prev = LD_DWORD(a);
    LD_DWORD(a) = fs_tmp;
  }

  /* ����������� ������ ���������� ��������. �������� if() if() �������� ������ ���, ��� && */
  if(fs_tmp == FREE_CLUSTER) if(cluster < fs_fatoptim) fs_fatoptim = cluster;

  /* ��������� */
  if(fs_tmp == LAST_CLUSTER || fs_tmp == FREE_CLUSTER)
    fs_tmp = prev;

  /* ��������� ������ */
  return fs_saveFatSector(s);
}

/**************************************************************************
*  ���������� � ��������� �������                                         *
**************************************************************************/

static void fs_setEntryCluster(BYTE* entry, DWORD cluster) {
  LD_WORD(entry + DIR_FstClusLO) = (WORD)(cluster);
  LD_WORD(entry + DIR_FstClusHI) = (WORD)(cluster >> 16);
}

/**************************************************************************
*  �������� ������� � �����                                               *
**************************************************************************/

static BYTE fs_eraseCluster(BYTE i) {
  memset(buf, 0, 512);
  for(; i < fs_csize; ++i)
    if(sd_writeBuf(fs_tmp + i)) return 1;
  return 0;
}

/**************************************************************************
*  �������� ��������� � ��������                                          *
*                                                                         *
*  entry_able ������ ���� ����� 0                                         *
*  entry_start_cluster ������ ��������� ������ ������� �����              *
**************************************************************************/

static BYTE* fs_allocEntry() {
  /* ���� � ����� ������ ���������. */
  while(1) {
    if(fs_readdirInt()) return 0;

    /* �������� ��������� */
    if(!fs_file.entry_able) break;

    /* ��� ��������� ��������� */
    if(FS_DIRENTRY[0] == 0xE5 || FS_DIRENTRY[0] == 0) { /* ����� ���� ��� 0x05 */

      /* ������ ������ */
      if(sd_readBuf(fs_file.entry_sector)) return 0;

      /* ��������� ��������� */
      return buf + (fs_file.entry_index % 16) * 32;
    }
  }

  /* ����������� �� ���-�� ������ */
  /* �������� ������� FAT16 ��� �� ������ fs_file.entry_index == 0 */
  if(fs_file.entry_index == 0) { lastError = ERR_DIR_FULL; return 0; }

  /* �������� �������. ��������� � fs_tmp */
  if(fs_allocCluster(ALLOCCLUSTER)) return 0;

  /* �������� ��� ���� ������� � �����. */
  if(fs_setNextCluster(fs_file.entry_cluster)) return 0; /* fs_tmp ����������, ��� ��� �� �� LAST � �� FREE */

  /* ��������� ����� */
  fs_file.entry_cluster = fs_tmp;
  fs_clust2sect();
  fs_file.entry_sector  = fs_tmp;

  /* ������� ������� � �� ���� BUF (������������ ���������� fs_tmp) */
  fs_eraseCluster(0);

  /* ��������� ��������� */
  return buf;
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

static BYTE exists(const flash BYTE* str, BYTE c) {
  while(*str)
    if(*str++ == c)
      return c;
  return 0;
}

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

/**************************************************************************
*  ��������� ����� ���������� ������� ��� ������/������                   *
*  ���������� ������ �� fs_read0, fs_write_start                          *
**************************************************************************/

static BYTE fs_nextRWSector() {
  if(fs_file.ptr == 0) {
    /* ������ ����� ��� �� ������ */
    fs_tmp = fs_file.entry_start_cluster;
  } else {
    /* ��� �� ����� ������� */
    if((WORD)fs_file.ptr % 512) return 0;

    /* ��������� ������ */
    fs_file.sector++;

    /* ��� �� ����� �������� */
    if(((fs_file.sector - fs_database) % fs_csize) != 0) return 0;

    /* ��������� ������� */
    fs_tmp = fs_file.cluster;
    if(fs_nextCluster()) return 1;
  }

  /* ���� ��� ��� ��������� �������, ��������� ����� */
  if(fs_tmp == 0) {
    if(fs_allocCluster(ALLOCCLUSTER)) return 1;
    if(fs_file.ptr == 0) fs_file.entry_start_cluster = fs_tmp;
                    else fs_setNextCluster(fs_file.cluster); /* fs_tmp ����������, ��� ��� �� �� LAST � �� FREE */
  }

  /* �� */
  fs_file.cluster = fs_tmp;
  fs_clust2sect();
  fs_file.sector  = fs_tmp;
  return 0;
}

/**************************************************************************
*  ��������� �� ����� ��������� ���� � buf                                *
*                                                                         *
*  ������������ �� ������ �������� �� ������� ����� ��� ������, �����     *
*  ��������� ������ ���������� ����� �� �����.                            *
*                                                                         *
*  ���������:                                                             *
*    ptr      - ����� ��� ������, ����� ���� buf                          *
*    len      - ���-�� ����, ������� ���������� ���������                 *
**************************************************************************/

BYTE fs_read0(BYTE* ptr, WORD len) {
  WORD sectorLen;

  /* �������� ������ ������������ */
#ifndef FS_DISABLE_CHECK
  if(fs_file.opened != OPENED_FILE) { lastError = ERR_NOT_OPENED; goto abort; }
#endif

  while(len) {
    /* ���� ��������� ��������� �� ������� ������� */
    if(fs_nextRWSector()) goto abort;

    /* ���-�� ���� �� ����� ������� */
    sectorLen = 512 - ((WORD)fs_file.ptr % 512);
    if(len < sectorLen) sectorLen = len;

    /* ������ ������ */
    if(ptr) {
      if(sd_read(ptr, fs_file.sector, (WORD)fs_file.ptr % 512, sectorLen)) goto abort;
      ptr += sectorLen;
    }

    /* ����������� �������� */
    fs_file.ptr += sectorLen;
    len -= sectorLen;
  }

  /* ���������� ������� ����� */
  if(fs_file.ptr > fs_file.size) fs_file.size = fs_file.ptr, fs_file.changed = 1;

  return 0;
abort:
#ifndef FS_DISABLE_CHECK
  fs_file.opened = OPENED_NONE;
#endif
  return 1;
}

/**************************************************************************
*  ��������� �� ����� ��������� ���� � buf                                *
*                                                                         *
*  ���������:                                                             *
*    ptr      - ����� ��� ������, ����� ���� buf                          *
*    len      - ���-�� ����, ������� ���������� ���������                 *
*    readed   - ���������, ��� ���������� ���-�� ������� ����������� ���� *
**************************************************************************/

BYTE fs_read(BYTE* ptr, WORD len, WORD* readed) {
  /* ������������ ���-�� ���� ��� ������ */
  if(len > fs_file.size - fs_file.ptr) len = (WORD)(fs_file.size - fs_file.ptr);
  *readed = len;

  /* �������� �� ������ ���������� ��� */
  return fs_read0(ptr, len);
}

/**************************************************************************
*  ��������� ����� ����� � ������ ������� � ���������                     *
*  ���������� �� fs_lseek, fs_write_start, fs_write_end, fs_write_eof     *
**************************************************************************/

static char fs_saveFileLength() {
  BYTE* entry;

  if(fs_file.changed == 0) return 0;
  fs_file.changed = 0;

  /* ��������� ��������� ����� */
  if(sd_readBuf(fs_file.entry_sector)) return 1;

  entry = buf + (fs_file.entry_index % 16) * 32;
  LD_DWORD(entry + DIR_FileSize) = fs_file.size;
  fs_setEntryCluster(entry, fs_file.entry_start_cluster);

  return sd_writeBuf(fs_file.entry_sector);
}

/**************************************************************************
*  ���������� �������� ������/������ �����                                *
**************************************************************************/

#define LSEEK_STEP 32768

BYTE fs_lseek(DWORD off, BYTE mode) {
  DWORD l;

  /* ����� */
  if(mode==1) off += fs_file.ptr; else
  if(mode==2) off += fs_file.size;

  /* ����� �������� �� fs_file.ptr = 0 ��� ���������� ����*/
  if(off >= fs_file.ptr) off -= fs_file.ptr; else fs_file.ptr = 0;

  do { /* ��������� ���� ���� ���� ��� off=0, ��� ��� ������ ���������� �������� �� ������ */
    l = off;
    if(l > LSEEK_STEP) l = LSEEK_STEP;
    if(fs_read0(0, (WORD)l)) return 1;
    off -= l;
  } while(off);

  /* ������ ����� ��� ��������� */
  fs_saveFileLength();

  /* ��������� */
  fs_tmp = fs_file.ptr;

  return 0;
}

/**************************************************************************
*  �������� � ���� (��� 1)                                                *
***************************************************************************/

BYTE fs_write_start() {
  WORD len;

  /* �������� ������ ������������ */
#ifndef FS_DISABLE_CHECK
  if(fs_file.opened != OPENED_FILE) { lastError = ERR_NOT_OPENED; goto abort; }
  if(fs_wtotal == 0) { lastError = ERR_NO_DATA; goto abort; }
#endif

  /* ������� ����� ��� �������� � ���� ������? */
  len = 512 - (WORD)fs_file.ptr % 512;

  /* ������������ �������� ������ � ����� */
  if(len > fs_wtotal) len = (WORD)fs_wtotal;

  /* ���������� fs_file.sector, ��������� ��������� */
  if(fs_nextRWSector()) goto abort; /* ������ �������� ������ �� ������� ERR_NO_FREE_SPACE, ERR_DISK_ERR */

  /* ������������ ����� ����� */
  if(fs_file.size < fs_file.ptr + len) {
    fs_file.size = fs_file.ptr + len;
    fs_file.changed = 1;
  }

  /* ������ ������ �������, ���� �� ���� ������ ����� �������� */
  if(len != 512) {
    if(sd_readBuf(fs_file.sector)) goto abort;
  }

  fs_file_wlen = len;
  fs_file_woff = (WORD)fs_file.ptr % 512;
  return 0;
abort:
  /* ������ ����� ��� ������ ERR_NO_FREE_SPACE */
  /* ���� ������ ����� ��� �������, �� ���� �� ��������� ��������� � ��������� �����. */
  fs_saveFileLength();
  /* ��������� ���� */
#ifndef FS_DISABLE_CHECK
  fs_file.opened = OPENED_NONE;
#endif
  return 1;
}

/**************************************************************************
*  �������� � ���� (��� 2)                                                *
***************************************************************************/

BYTE fs_write_end() {
#ifndef FS_DISABLE_CHECK
  if(fs_file.opened != OPENED_FILE) { lastError = ERR_NOT_OPENED; goto abort; }
#endif

  /* ���������� ��������� �� ���� */
  if(sd_writeBuf(fs_file.sector)) goto abort;

  /* � ������ ������ ���� ����� ��������� ������ ���������, ��� ��������� �� ��� �������. */
  /* �� ��� �� �����, ������ �� ����������. � ��� ������ ��������� ���� � ���������� */
  /* ����� �� �����. */

  /* �������� */
  fs_file.ptr += fs_file_wlen;
  fs_wtotal   -= fs_file_wlen;

  /* ���� ������ ���������, ��������� ������� ����� � ������ ������� */
  if(fs_wtotal == 0) {
    if(fs_saveFileLength()) goto abort;
  }

  /* �� */
  return 0;
abort:
#ifndef FS_DISABLE_CHECK
    fs_file.opened = OPENED_NONE;
#endif
  return 1;
}

/**************************************************************************
*  ���������� ������� ��������� ������� � fs_tmp                          *
**************************************************************************/

static BYTE fs_freeChain() {
  DWORD c;
  while(1) {
    if(fs_tmp < 2 || fs_tmp >= fs_n_fatent) return 0;
    /* ���������� ������� fs_tmp � ������� � fs_tmp ��������� �� ��� ������� */
    c = fs_tmp, fs_tmp = FREE_CLUSTER;
    if(fs_setNextCluster(c)) break; /* fs_tmp ����� ��������� ��������� �������, ��� ��� ������������ FREE_CLUSTER */
  }
  return 1;
}

/**************************************************************************
*  ����������� ����/�����                                                 *
**************************************************************************/

BYTE fs_move0() {
  BYTE* entry;
  BYTE tmp[21];
  WORD old_index;
  DWORD old_sector, old_start_cluster;

#ifndef FS_DISABLE_CHECK
  if(fs_file.opened == OPENED_NONE) { lastError = ERR_NOT_OPENED; goto abort; }
#endif

  /* ���������� ������ ��������� */
  old_index         = fs_file.entry_index;
  old_sector        = fs_file.entry_sector;
  old_start_cluster = fs_file.entry_start_cluster;

  /* ������� ����� ����. � ����� �� ����������� �����. 0x80 - ��� ������������� ��������. */
  if(fs_open0(OPENED_FILE | 0x80)) goto abort;

  /* ������������� ������ ������������ */
#ifndef FS_DISABLE_CHECK
  fs_file.opened = OPENED_NONE;
#ifndef FS_DISABLE_SWAP
  fs_secondFile.opened = OPENED_NONE;
#endif
#endif
  /* fs_file.sector ������� ������ ������� �����, � ������� ��������� ��������� ����. */

  /* �������� ������� �����/����� � ������� ���� ������� */
  if(sd_readBuf(old_sector)) goto abort;
  entry = buf + (old_index % 16) * 32;
  memcpy(tmp, entry+11, 21);
  entry[0] = 0xE5;
  if(sd_writeBuf(old_sector)) goto abort;

  /* �������� ��� �������� ������ �����, ��� ����� ��������� ��� � ����� */
  if(sd_readBuf(fs_file.entry_sector)) goto abort;
  entry = buf + (fs_file.entry_index % 16) * 32;
  memcpy(entry+11, tmp, 21);
  if(sd_writeBuf(fs_file.entry_sector)) goto abort;

  /* � ����� ���� ��� �������������� ��������� .. */
  if(entry[DIR_Attr] & AM_DIR) {
    fs_tmp = old_start_cluster; /* ������ ������� ����� ����� */
    fs_clust2sect();
    if(sd_readBuf(fs_tmp)) goto abort;
    fs_setEntryCluster(buf+32, fs_parent_dir_cluster); /* ������ ������� ����� ������.*/
    if(sd_writeBuf(fs_tmp)) goto abort;
  }

  return 0;
abort:
  return 1;
}

BYTE fs_move(const char* from, const char* to) {
  strcpy((char*)buf, from);
  if(fs_openany()) return 1;
  strcpy((char*)buf, to);
  return fs_move0();
}

/**************************************************************************
*  ������� ���� ��� ������ �����                                          *
*  ��� ����� ������ ����������� � buf � �� ��������� FS_MAXFILE �������   *
*  ������� ����������                                                     *
**************************************************************************/

BYTE fs_delete() {
  DWORD entrySector;
  BYTE* entry;

  /* ��� ����� �������� fs_type == FS_ERROR */
  if(fs_openany()) goto abort;

  /* ������������� ������ ������������ */
  fs_file.opened = OPENED_NONE;
#ifndef FS_DISABLE_SWAP
  fs_secondFile.opened = OPENED_NONE;
#endif

  /* �������� ����� ������� ������ */
  if(FS_DIRENTRY[0] == 0) { lastError = ERR_NO_PATH; goto abort; }

  /* ��������� ������������� � ��������� �����, ��� ��� fs_readdir ���� �� ������� */
  entrySector = fs_file.entry_sector;
  entry = buf + (fs_file.entry_index % 16) * 32;

  /* � ����� �� ������ ���� ������ */
  if(FS_DIRENTRY[DIR_Attr] & AM_DIR) {
    /* ����������� ����� �� ������ */
    fs_file.entry_able = 0;
    /* ���� ������ ���� ��� ����� */
    /* fs_file.entry_start_cluster ����������� (�������� ������ ������� ����� ��� �����) */
    if(fs_readdir_nocheck()) goto abort;
    /* ���� �����, �� ������ */
    if(fs_file.entry_able) { lastError = ERR_DIR_NOT_EMPTY; goto abort; }
  }

  /* ������� ��������� */
  if(sd_readBuf(entrySector)) goto abort;
  entry[0] = 0xE5;
  if(sd_writeBuf(entrySector)) goto abort;

  /* ����������� ������� ��������� */
  fs_tmp = fs_file.entry_start_cluster;
  return fs_freeChain();
abort:
  return 1;
}

/**************************************************************************
*  ���������� ����� �����                                                 *
**************************************************************************/

BYTE fs_write_eof() {
  /* �������� ������ ������������ */
#ifndef FS_DISABLE_CHECK
  if(fs_file.opened != OPENED_FILE) { lastError = ERR_NOT_OPENED; goto abort; }
#endif

  /* ������������ ���� FAT, ���� ��������� �����. */
  if(fs_file.ptr == 0) {
    /* ������ ��� �������� ����� */
    fs_tmp = fs_file.entry_start_cluster;
    fs_file.entry_start_cluster = 0;
  } else {
    /* ���� ������� ����� ���������. */
    fs_tmp = LAST_CLUSTER;
    if(fs_setNextCluster(fs_file.cluster)) goto abort; /* fs_tmp ����� ��������� ��������� �������, ��� ��� ������������ LAST_CLUSTER */
  }

  /* ������ ��� �������� ����� ����� �����. (��� ���������� � fs_tmp); */
  if(fs_freeChain()) goto abort;

  /* ��������� ���� � ������ ������� */
  fs_file.size    = fs_file.ptr;
  fs_file.changed = 1;
  if(!fs_saveFileLength()) return 0;

abort:
#ifndef FS_DISABLE_CHECK
  fs_file.opened = OPENED_NONE;
#endif
  return 1;
}

/**************************************************************************
*  �������� � ����                                                        *
**************************************************************************/

BYTE fs_write(CONST BYTE* ptr, WORD len) {
  /* �������� �� ������ ���������� � ���������� �������� */

  /* ����� ����� */
  if(len == 0) return fs_write_eof();

  fs_wtotal = len;
  do {
    if(fs_write_start()) goto abort;
    memcpy(fs_file_wbuf, ptr, fs_file_wlen);
    ptr += fs_file_wlen;
    if(fs_write_end()) goto abort;
  } while(fs_wtotal);

  return 0;
abort:
  return 1;
}

/**************************************************************************
*  ����������� �����                                                      *
**************************************************************************/

#ifndef FS_DISABLE_SWAP
void fs_swap() {
  /* ��� �������� ������ ���, ��� ��� ������� memcpy */
  BYTE t, *a = (BYTE*)&fs_file, *b = (BYTE*)&fs_secondFile, n = sizeof(File);
  do {
    t=*a, *a=*b, *b=t; ++a; ++b;
  } while(--n);
}
#endif

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
*  ������ �����                                                           *
**************************************************************************/

BYTE fs_getfilesize() {
#ifndef FS_DISABLE_CHECK
  if(fs_file.opened != OPENED_FILE) {
    lastError = ERR_NOT_OPENED;
    return 1;
  }
#endif
  fs_tmp = fs_file.size;
  return 0;
}

/**************************************************************************
*  ��������� ������ ������ �����                                          *
**************************************************************************/

BYTE fs_tell() {
#ifndef FS_DISABLE_CHECK
  if(fs_file.opened != OPENED_FILE) {
    lastError = ERR_NOT_OPENED;
    return 1;
  }
#endif
  fs_tmp = fs_file.ptr;
  return 0;
}
