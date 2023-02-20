// Драйвер Flash-диска
// Размер 4 Гб, байт в секторе 512
// FAT32
// в MBR информация о секторах начало 1BEh
// 8-й байт указывает на первый сектор данного раздела (4 байта с 1C6h до 1C9h)
// перевести Hex2Dec (0000h 003Fh = 63)
// Ctrl+G перейти в сектор 63 LBA - это загрузочная запись раздела
// с 3h-го адреса 8 байт - версия операционной системы (MSDOS5.0)
// по адресу Dh - количество секторов на кластер (08h) 08*512=4096 (4Кб размер кластера)
// по адресу Eh (2 байта) количество секторов перед первой FAT 0448h = 1096, 1096*512=561152
// по адресу 10h количество FAT таблиц (02h)
// количество секторов занимаемые одной FAT по адресу 24h (2 байта) = 1DDCh = 7644 секторов
// первая таблица FAT 63+1096=1159 (Ctrl+G) (F8FFFF0F FFFFFFFF 39680200...)
// следующая таблица FAT через 7644 секторов 1159+7644=8803 (F8FFFF0F FFFFFFFF 39680200...полная копия первой FAT)
// добавим еще один размер FAT 7644, получим адрес корневой директории 8803+7644=16447
// корневая директория содержит файловые записи
// с адреса 40h (11 байт) - имя файла
// по адресу 2Dh чек сумма
// по адресу 5Ch (4 байта) - размер файла, 0000 0000 = пустой
// по адресу 56h (2 байта) - время файла, первые 5 бит (с конца) - секунды (нкжно умножить на 2, потому что отсчет идет по 2 секунды)
// с 6 по 10 бит - минуты,
// по адресу 58h (2 байта) - дата файла
#include <stdio.h>
#include <stdlib.h>

#define flash

typedef unsigned char   BYTE;
typedef unsigned short  WORD;
typedef unsigned long	DWORD;

/* Error */
/* Ошибки файловой системы */

#define ERR_OK              0  // Нет ошибки
#define ERR_NO_FILESYSTEM   1  // Файловая система не обнаружена
#define ERR_DISK_ERR        2  // Ошибка чтения/записи
#define	ERR_NOT_OPENED      3  // Файл/папка не открыта
#define	ERR_NO_PATH         4  // Файл/папка не найдена
#define ERR_DIR_FULL        5  // Папка содержит максимальное кол-во файлов
#define ERR_NO_FREE_SPACE   6  // Нет свободного места
#define ERR_DIR_NOT_EMPTY   7  // Нельзя удалить папку, она не пуста
#define ERR_FILE_EXISTS     8  // Файл/папка с таким именем уже существует
#define ERR_NO_DATA         9  // fs_file_wtotal=0 при вызове функции fs_write_begin

#define ERR_MAX_FILES       10 // Не используется файловой системой, резерв
#define ERR_RECV_STRING     11 // Не используется файловой системой, резерв
#define ERR_INVALID_COMMAND 12 // Не используется файловой системой, резерв

#define ERR_ALREADY_OPENED  13 // Файл уже открыт (fs_swap)


/* Filesystem variables. Can change */
/* Переменные файловой системы. Можно изменять */

extern DWORD fs_tmp;
extern BYTE lastError;       /* Последняя ошибка файловой системы или диска */
extern WORD fs_wtotal;       /* Используется функциями fs_write_start, fs_write_end*/

/* Maximal length of file name */
/* Максимальная длина имени */

#define FS_MAXFILE  469

/* Result of fs_readdir */
/* Описатель прочитанный функцией fs_readdir */

#define FS_DIRENTRY  (buf + 480)




#include <Windows.h>


// Work buffer;
BYTE buf[512];


HANDLE handle = INVALID_HANDLE_VALUE;


BYTE sd_init() {//fat16.vhd  //   \\\\.\\G:
  handle = CreateFile(TEXT("\\\\.\\PhysicalDrive1"),    // открываемый файл
                   GENERIC_READ, FILE_SHARE_READ, NULL,
            OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

                   /*GENERIC_ALL,//GENERIC_READ | GENERIC_WRITE,          // открываем для чтения
                   FILE_SHARE_READ | FILE_SHARE_WRITE,       // для совместного чтения
                   NULL,                  // защита по умолчанию
                   OPEN_EXISTING,         // только существующий файл
                   NULL,//FILE_ATTRIBUTE_NORMAL, // обычный файл
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
void fs_swap();                                  /* Переключится на второй файл / Switch to the second file */
#endif

#ifndef FS_DISABLE_GETFREESPACE
BYTE fs_getfree();                               /* Свободное место на диске в fs_tmp  в мегабайтах / Free disk space in fs_tmp (megabytes) */
#endif


/* Функции ниже позволяют использовать для записи буфер (buf).
*  The functions below can be used to write the buffer (buf).
*
*  wtotal = размер данных для записи;
*  while(wtotal) {
*    fs_write_start(&wtotal);
*    Копируем fs_file_wlen байт в fs_file_wbuf
*    Ни одна функция ФС не должеа выпонятсья между ними
*    fs_write_end(&wtotal);
*  }
*
* See the source of fs_write
* Или смотри исходники fs_write
*/

#define fs_file_wlen   (*(WORD*)&fs_tmp)
#define fs_file_woff   ((WORD*)&fs_tmp)[1]
#define fs_file_wbuf   (buf + fs_file_woff)

BYTE fs_write_start();
BYTE fs_write_end();

/* Derived from the function fs_open0 */
/* Производные от функции fs_open0 */

BYTE fs_open();                                  /* Открыть файл */
//BYTE fs_opendir();                               /* Открыть папку */
#define fs_openany()    fs_open0(OPENED_NONE)    /* Открыть файл или папку */
#define fs_create()     fs_open0(OPENED_FILE)    /* Создать файл */
#define fs_createdir()  fs_open0(OPENED_DIR)     /* Создать папку */


/* Values ??fs_opened */
/* Значения fs_opened */

#define OPENED_NONE    0
#define OPENED_FILE    1
#define OPENED_DIR     2

/* File attributes */
/* Атрибуты файлов */

#define	AM_RDO         0x01  /* Read only */
#define	AM_HID         0x02  /* Hidden */
#define	AM_SYS         0x04  /* System */
#define	AM_VOL         0x08  /* Volume label */
#define AM_DIR         0x10  /* Directory */
#define AM_ARC         0x20  /* Archive */

/* Fodler descriptor (FS_DIRENTRY) */
/* Описатель */

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









/* Для наглядности */

#define	LD_WORD(ptr)    (*(WORD*)(ptr))
#define	LD_DWORD(ptr)   (*(DWORD*)(ptr))

/* Значения fs_type */

#define FS_FAT16	0
#define FS_FAT32	1
#define FS_ERROR	2

/* Специальные значения кластеров */

#define FREE_CLUSTER    0
#define LAST_CLUSTER    0x0FFFFFFF

/* Системные переменные. Информация из boot-сектора */

typedef struct {
#ifndef FS_DISABLE_CHECK
  BYTE  opened;              /* Что открыто: OPENED_NONE, OPENED_FILE, OPENED_DIR */
#endif
  BYTE  entry_able;          /* Результат выполнения функции fs_dirread */
  WORD  entry_index;         /* Номер записи в каталоге */
  DWORD entry_cluster;       /* Кластер записи в каталоге */
  DWORD entry_sector;        /* Сектор записи в каталоге */
  DWORD entry_start_cluster; /* Первый сектор файла или каталога (0 - корневой каталог FAT16) */
  DWORD ptr;                 /* Указатель чтения/записи файла*/
  DWORD size;                /* Размер файла / File size */
  DWORD cluster;             /* Текущий кластер файла */
  DWORD sector;              /* Текущий сектор файла */
  BYTE  changed;             /* Размер файла изменился, надо сохранить */
} File;

BYTE  fs_type;         /* FS_FAT16, FS_FAT32, FS_ERROR */
DWORD fs_fatbase;      /* Адрес первой FAT */
DWORD fs_fatbase2;     /* Адрес второй FAT */
BYTE  fs_csize;        /* Размер кластера в секторах */
WORD  fs_n_rootdir;    /* Кол-во записей в корневом каталоге (только FAT16) */
DWORD fs_n_fatent;     /* Кол-во кластеров */
DWORD fs_dirbase;      /* Корневой каталог (сектор для FAT16, кластер для FAT32) */
DWORD fs_database;     /* Адрес второго кластера */

/* Системные переменные. Остальное */

BYTE  lastError;       /* Последняя ошибка */
DWORD fs_fatoptim;     /* Первый свободный кластер */
DWORD fs_tmp;          /* Используеются для разных целей */
WORD  fs_wtotal;       /* Используется функциями fs_write_start, fs_write_end*/

/* Открытые файлы/папки */

File fs_file;

#ifndef FS_DISABLE_SWAP
File fs_secondFile;
#endif

/* Структура boot-сектора */

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
*  Чтение сектора в буфер                                                 *
**************************************************************************/

static BYTE sd_readBuf(DWORD sector) {
  return sd_read(buf, sector, 0, 512);
}

/**************************************************************************
*  Запись буфера в сектор                                                 *
**************************************************************************/

static BYTE sd_writeBuf(DWORD sector) {
  return sd_write512(buf, sector);
}

/**************************************************************************
*  Инициализация                                                          *
**************************************************************************/

BYTE fs_init() {
  DWORD bsect, fsize, tsect;

  /* Сбрасываем оптимизацию */
  fs_fatoptim = 2;

  /* Предотвращаем ошибки программиста */
/*#ifndef FS_DISABLE_CHECK
  fs_file.opened = OPENED_NONE;
#ifndef FS_DISABLE_SWAP
  fs_secondFile.opened = OPENED_NONE;
#endif
  fs_type = FS_ERROR;
#endif*/

  /* Инициализация накопителя */
  if(sd_init()) return 1;

/* Ищем файловую систему */
  bsect = 0;
  while(1) {
    if(sd_readBuf(bsect)) return 1;
    if(LD_WORD(buf + 510) == 0xAA55) {
      if(LD_WORD(buf + BS_FilSysType  ) == 0x4146) break;
      if(LD_WORD(buf + BS_FilSysType32) == 0x4146) break;
      /* Возможно это MBR */
      if(bsect == 0 && buf[MBR_Table+4]) {
        bsect = LD_DWORD(buf + (MBR_Table + 8));
        if(bsect != 0) continue;
      }
    }
abort_noFS:
    printf("NO FS");
    lastError = ERR_NO_FILESYSTEM; return 1;
  }

  /* Размер таблицы FAT в секторах */
  fsize = LD_WORD(buf + BPB_FATSz16);
  if(fsize == 0) fsize = LD_DWORD(buf + BPB_FATSz32);

  /* Размер файловой системы в секторах */
  tsect = LD_WORD( buf + BPB_TotSec16);
  if(tsect == 0) tsect = LD_DWORD(buf + BPB_TotSec32);

  /* Размер корневого каталога (должно быть кратно 16 и для FAT32 должно быть рано нулю) */
  fs_n_rootdir = LD_WORD(buf + BPB_RootEntCnt);

  /* Адреса таблиц FAT в секторах */
  fs_fatbase  = bsect + LD_WORD(buf + BPB_RsvdSecCnt);
  fs_fatbase2 = 0;
  if(buf[BPB_NumFATs] >= 2) fs_fatbase2 = fs_fatbase + fsize;

  /* Кол-во секторов на кластер */
  fs_csize = buf[BPB_SecPerClus];

  /* Кол-во кластеров */
  fsize *= buf[BPB_NumFATs];
  fs_n_fatent = (tsect - LD_WORD(buf + BPB_RsvdSecCnt) - fsize - fs_n_rootdir / 16) / fs_csize + 2;

  /* Адрес 2-ого кластера */
  fs_database = fs_fatbase + fsize + fs_n_rootdir / 16;

  /* Определение файловой системы */

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

  /* Сбрасываем счетчик свободного места */
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
*  Выделить кластер                                                       *
*                                                                         *
*  Найденный кластер сохраняется в fs_tmp                                 *
**************************************************************************/

/* Ради функции fs_getfree пришлось усложнить функцию fs_allocCluster.
Если функция не используется, то можно с помощью макроса FS_DISABLE_GETFREESPACE
исключить лишний код */

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

  /* Начинаем поиск с этого кластера */
  fs_tmp = fs_fatoptim;

  /* Последовательно перебираем сектора */
  while(1) {
    /* Сектор и смещение */
    s = fs_tmp / 256, i = (BYTE)fs_tmp, a = (BYTE*)((WORD*)buf + i);
    if(fs_type != FS_FAT16) s = fs_tmp / 128, i |= 128, a = (BYTE*)((DWORD*)buf - 128 + i);

    /* Читаем сектор */
    if(sd_readBuf(fs_fatbase + s)) goto abort;

    /* Среди 128/256 чисел в секторе ищем 0 */
    /* Куча проверок внутри цикла не самое быстрое решение, но зато получается очень компактный код. */
    do {
      /* Кластеры кончились */
      if(fs_tmp >= fs_n_fatent) { DIS(if(freeSpace) return 0;) lastError = ERR_NO_FREE_SPACE; goto abort; }

      /* Ищем свободный кластер и помечаем как последний */
      if(fs_type == FS_FAT16) {
        if(LD_WORD(a) == 0) { DIS(if(!freeSpace) {) LD_WORD(a) = (WORD)LAST_CLUSTER; goto founded; DIS(} fs_file.sector++;) }
        a += 2;
      } else {
        if(LD_DWORD(a) == 0) { DIS(if(!freeSpace) {) LD_DWORD(a) = LAST_CLUSTER; goto founded; DIS(} fs_file.sector++;) }
        a += 4;
      }

      /* Счетчик */
      ++fs_tmp, ++i;
    } while(i != 0);
  }
founded:
  /* Оптимизация */
  fs_fatoptim = fs_tmp;

  /* Сохраняем изменения */
  return fs_saveFatSector(s);
abort:
  return 1;
}

#undef DIS


/**************************************************************************
*  Расчет свободного места                                                *
*                                                                         *
*  Результат в переменной fs_tmp в мегабайтах                             *
*  Функция закрывает файл                                                 *
**************************************************************************/

#ifndef FS_DISABLE_GETFREESPACE
BYTE fs_getfree() {
  /* Мы испортим переменную fs_file.sector, поэтому закрываем файл */
  fs_file.opened = OPENED_NONE;

  /* Кол-во свободных кластеров будет в fs_file.sector */
  fs_file.sector = 0;
  if(fs_allocCluster(1)) return 1;

  /* Пересчет в мегабайты */
  fs_tmp = ((fs_file.sector >> 10) + 1) / 2 * fs_csize;

  return 0;
}
#endif

/**************************************************************************
*  Размер накопителя в мегабайтах                                         *
**************************************************************************/

BYTE fs_gettotal() {
  /* Проверка ошибок программиста */
#ifndef FS_DISABLE_CHECK
  if(fs_type == FS_ERROR) { lastError = ERR_NO_FILESYSTEM; return 1; }
#endif

  fs_tmp = ((fs_n_fatent >> 10) + 1) / 2 * fs_csize;
  return 0;
}


/**************************************************************************
*  Открыть/создать файл или папку                                         *
*                                                                         *
*  Имя файла в buf. Оно не должно превышать FS_MAXFILE симолов включая 0  *
*                                                                         *
*  what = OPENED_NONE - Открыть файл или папку                            *
*  what = OPENED_FILE - Создать файл (созданный файл открыт)              *
*  what = OPENED_DIR  - Создать папку (созданная папка не открыта)        *
*  what | 0x80        - Не создавать файл в папке entry_start_cluster     *
*                                                                         *
*  На выходе                                                              *
*   FS_DIRENTRY                 - описатель                               *
*   fs_file.entry_able          - 0 (если открыт существующий файл/папка) *
*   fs_file.entry_sector        - сектор описателя                        *
*   fs_file.entry_cluster       - кластер описателя                       *
*   fs_file.entry_index         - номер описателя                         *
*   fs_file.entry_start_cluster - первый кластер файла или папки          *
*   fs_parent_dir_cluster       - первый кластер папки предка (CREATE)    *
*   fs_file.ptr                 - 0 (если открыт файл)                    *
*   fs_file.size                - размер файла (если открыт файл)         *
*                                                                         *
*  Функция не портит buf[0..MAX_FILENAME-1]                               *
**************************************************************************/

static BYTE fs_open0_create(BYTE dir); /* forward */
static CONST BYTE* fs_open0_name(CONST BYTE *p); /* forward */

#define FS_DIRFIND      (buf + 469)           /* 11 байт использующиеся внутри функции fs_open0 */
#define fs_notrootdir (*(BYTE*)&fs_file.size) /* Используется fs_open0, в это время переменные fs_file. не содежат нужных значения */
#define fs_parent_dir_cluster fs_file.sector  /* Так же используется fs_file.sector для хранения первого кластера папки предка. */

BYTE fs_open0(BYTE what) {
  CONST BYTE *path;
  BYTE r;

  /* Проверка ошибок программиста */
#ifndef FS_DISABLE_CHECK
  if(fs_type == FS_ERROR) { lastError = ERR_NO_FILESYSTEM; goto abort; }
  fs_file.opened = OPENED_NONE;
#endif

  /* Предотвращение рекурсии */
  r = what & 0x80; what &= 0x7F;
  fs_parent_dir_cluster = fs_file.entry_start_cluster;

  /* Корневой каталог */
  fs_notrootdir = 0;
  fs_file.entry_start_cluster = fs_dirbase;
  if(fs_type == FS_FAT16) fs_file.entry_start_cluster =  0;

  /* Корневая папка */
  if(buf[0] == 0) {
    if(what) goto abort_noPath;
    FS_DIRENTRY[0] = 0;             /* Признак корневой папки */
    FS_DIRENTRY[DIR_Attr] = AM_DIR; /* Для упрощения определения файл/папка запишем сюда AM_DIR */
  } else {
    path = buf;
    while(1) {
      /* Получаем очередное имя из path в FS_DIRFIND */
      path = fs_open0_name(path);
      if(path == (CONST BYTE*)1) goto abort_noPath;
      /* Ищем имя в папке */
      fs_file.entry_able = 0;
      while(1) {
        if(fs_readdir_nocheck()) return 1;
        if(fs_file.entry_able == 0) break;
        if(!memcmp(FS_DIRENTRY, FS_DIRFIND, 11)) break;
      }
      /* Последний элементу пути в режиме создания */
      if(what && path == 0) {
        fs_parent_dir_cluster = fs_file.entry_start_cluster; /* Сохраняем в этой переменной результат для фунции fs_move */
        if(fs_type == FS_FAT32 && fs_parent_dir_cluster == fs_dirbase) fs_parent_dir_cluster = 0;
        if(fs_file.entry_able == 0) return fs_open0_create(what-1); /* Продолжим там */
        lastError = ERR_FILE_EXISTS; goto abort;
      }
      /* Файл/папка не найдена */
      if(fs_file.entry_able == 0) goto abort_noPath;

      /* Что то найдено */
      fs_file.entry_start_cluster = fs_getEntryCluster();
      /* Это был последний элемент пути */
      if(path == 0) break;
      /* Это должна быть папка */
      if((FS_DIRENTRY[DIR_Attr] & AM_DIR) == 0) goto abort_noPath;
      /* Предотвращаем рекурсию для функции fs_move */
      if(r && fs_file.entry_start_cluster == fs_parent_dir_cluster) goto abort_noPath;
      /* Наденная папка уже не будет корневой */
      fs_notrootdir = 1;
    }
  }
  /* Устанавливаем переменные */
  fs_file.entry_able = 0;
  fs_file.size  = LD_DWORD(FS_DIRENTRY + DIR_FileSize);
  fs_file.ptr   = 0;
#ifndef FS_DISABLE_CHECK
  fs_file.opened     = OPENED_FILE;
  if(FS_DIRENTRY[DIR_Attr] & AM_DIR) fs_file.opened = OPENED_DIR;
#endif

  /* Нельзя дважды открывать файл */
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
      if(i == 0) break; /* Пустое имя файла */
      return 0;
    }
    if(c == '/') return p;
    if(c == '.') {
      if(i == 0) break; /* Пустое имя файла */
#ifndef FS_DISABLE_CHECK
      if(ni != 8) break; /* Вторая точка */
#endif
      i = 8; ni = 11;
      continue;
    }
    /* Слишком длинное име */
    if(i == ni) break;
    /* Запрещенные символы */
#ifndef FS_DISABLE_CHECK
    if(exists((const flash BYTE* )"+,;=[]*?<:>\\|\"", c)) break;
    if(c <= 0x20) break;
    if(c >= 0x80) break;
#endif
    /* Приводим к верхнему регистру */
    if(c >= 'a' && c <= 'z') c -= 0x20;
    /* Сохраняем имя */
    FS_DIRFIND[i++] = c;
  }
  /* Ошибка */
  return (CONST BYTE*)1;
}

static BYTE fs_open0_create(BYTE dir) {
  BYTE  new_name[11];
  DWORD allocatedCluster;
  BYTE* allocatedEntry;

  /* Сохраняем имя, так как весь буфер будет затерт */
  memcpy(new_name, FS_DIRFIND, 11);

  /* Выделяем кластер для папки */
  if(dir) {
    if(fs_allocCluster(ALLOCCLUSTER)) goto abort; /* fs_file.entry_start_cluster изменен не будет, там первый кластер папки в которой мы создадим файл */
    allocatedCluster = fs_tmp;
  }

  /* Добавляем в папку описатель (сектор не сохранен) */
  allocatedEntry = fs_allocEntry();
  if(allocatedEntry == 0) {

    /* В случае ошибки освобождаем кластер */
    fs_tmp = FREE_CLUSTER;
    fs_setNextCluster(allocatedCluster);
    goto abort;
  }

  /* Устаналиваем имя в описатель. */
  memset(allocatedEntry, 0, 32);
  memcpy(allocatedEntry, new_name, 11);

  if(!dir) {
    /* Сохраняем описатель на диск */
    if(sd_writeBuf(fs_file.entry_sector)) goto abort;
    /* fs_file.entry_sector, fs_file.entry_index - устанавлиается в fs_allocCluster */
    fs_file.entry_start_cluster = 0;
    fs_file.size           = 0;
    fs_file.ptr            = 0;
#ifndef FS_DISABLE_CHECK
    fs_file.opened              = OPENED_FILE;
#endif
    return 0;
  }

  /* Это папка */
  allocatedEntry[DIR_Attr] = AM_DIR;
  fs_setEntryCluster(allocatedEntry, allocatedCluster);

  /* Сохраняем описатель на диск */
  if(sd_writeBuf(fs_file.entry_sector)) goto abort;

  /* Сектор для новой папки */
  fs_tmp = allocatedCluster;
  fs_clust2sect();

  /* Очищаем fs_tmp и за одно buf */
  fs_eraseCluster(1);

  /* Создаем пустую папку */
  memset(buf, ' ', 11); buf[0] = '.'; buf[11] = 0x10;
  fs_setEntryCluster(buf, allocatedCluster);

  memset(buf+32, ' ', 11); buf[32] = '.'; buf[33] = '.'; buf[32+11] = 0x10;
  if(fs_notrootdir) fs_setEntryCluster(buf + 32, fs_file.entry_start_cluster); /* Сейчас в fs_file.size==0 значит корневой каталог */

  /* Сохраняем папку */
  return sd_writeBuf(fs_tmp);
abort:
  return 1;
}


/**************************************************************************
*  Открыть файл                                                           *
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
*  Открыть папку                                                          *
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
    HANDLE hDisk = CreateFile(TEXT("\\\\.\\PhysicalDrive6"),    // открываемый файл
                   GENERIC_READ | GENERIC_WRITE,          // открываем для чтения
                   FILE_SHARE_READ | FILE_SHARE_WRITE,       // для совместного чтения
                   NULL,                  // защита по умолчанию
                   OPEN_EXISTING,         // только существующий файл
                   FILE_ATTRIBUTE_NORMAL, // обычный файл
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
