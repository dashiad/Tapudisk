#include "SDFile.h"
File g_fil;

FRESULT f_open(File *fil, char *filename,int mode)
{
  Serial.print("ABRIENDO FICHERO ");
  Serial.println(filename);
   (*fil) = SD.open(filename,mode);
   Serial.println("HECHO");
   if(!(*fil))
      return FR_NO_FILE;
   Serial.println("FICHERO ABIERTO OK");
   return FR_OK;
}

FRESULT f_read (
  File* fp,    /* Pointer to the file object */
  void* buff,   /* Pointer to data buffer */
  uint16_t btr,   /* Number of bytes to read */
  uint16_t* br    /* Pointer to number of bytes read */
)
{
  int cPos=fp->position();
  fp->read(buff,btr);
  *br=fp->position()-cPos;  
  return FR_OK;
}

FRESULT f_close(File *fp)
{
  fp->close();
  return FR_OK;
}

FRESULT f_write (
  File* fp,    /* Pointer to the file object */
  void* buff,   /* Pointer to data buffer */
  uint16_t btr,   /* Number of bytes to read */
  uint16_t* br    /* Pointer to number of bytes read */
)
{
  *br= fp->write((unsigned char *)buff,btr);
  return FR_OK;
}

void f_lseek(File *fp,int pos)
{
  fp->seek(pos);
}


