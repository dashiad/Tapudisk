#ifndef SDFILE_H
#define SDFILE_H
#include <SD.h>

#define FAT_BUF_SIZE 256

typedef enum {
  FR_OK = 0,        /* (0) Succeeded */
  FR_DISK_ERR,      /* (1) A hard error occurred in the low level disk I/O layer */
  FR_INT_ERR,       /* (2) Assertion failed */
  FR_NOT_READY,     /* (3) The physical drive cannot work */
  FR_NO_FILE,       /* (4) Could not find the file */
  FR_NO_PATH,       /* (5) Could not find the path */
  FR_INVALID_NAME,    /* (6) The path name format is invalid */
  FR_DENIED,        /* (7) Access denied due to prohibited access or directory full */
  FR_EXIST,       /* (8) Access denied due to prohibited access */
  FR_INVALID_OBJECT,    /* (9) The file/directory object is invalid */
  FR_WRITE_PROTECTED,   /* (10) The physical drive is write protected */
  FR_INVALID_DRIVE,   /* (11) The logical drive number is invalid */
  FR_NOT_ENABLED,     /* (12) The volume has no work area */
  FR_NO_FILESYSTEM,   /* (13) There is no valid FAT volume */
  FR_MKFS_ABORTED,    /* (14) The f_mkfs() aborted due to any parameter error */
  FR_TIMEOUT,       /* (15) Could not get a grant to access the volume within defined period */
  FR_LOCKED,        /* (16) The operation is rejected according to the file sharing policy */
  FR_NOT_ENOUGH_CORE,   /* (17) LFN working buffer could not be allocated */
  FR_TOO_MANY_OPEN_FILES, /* (18) Number of open files > _FS_SHARE */
  FR_INVALID_PARAMETER  /* (19) Given parameter is invalid */
} FRESULT;


#define  FA_READ       FILE_READ
#define FA_OPEN_EXISTING  0x00

#if !_FS_READONLY
#define FA_WRITE      FILE_WRITE
#define FA_CREATE_NEW   0x04
#define FA_CREATE_ALWAYS  0x08
#define FA_OPEN_ALWAYS    0x10
#define FA__WRITTEN     0x20
#define FA__DIRTY     0x40
#endif





FRESULT f_open(File *g_fil, char *filename,int mode);
FRESULT f_read (
  File* fp,    /* Pointer to the file object */
  void* buff,   /* Pointer to data buffer */
  uint16_t btr,   /* Number of bytes to read */
  uint16_t* br    /* Pointer to number of bytes read */
);

FRESULT f_write (
  File* fp,    /* Pointer to the file object */
  void* buff,   /* Pointer to data buffer */
  uint16_t btr,   /* Number of bytes to read */
  uint16_t* br    /* Pointer to number of bytes read */
);


FRESULT f_close (
  File* fp    /* Pointer to the file object */  
);
void f_lseek(File *fp,int pos);

#endif

