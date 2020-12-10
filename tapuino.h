#ifndef TAPUINO_H
#define TAPUINO_H



#include "integer.h"  /* Basic integer types */
#define TCHAR char
#define BYTE unsigned int
#include "SDFile.h"

typedef struct {
  long fsize;      /* File size */
  WORD  fdate;      /* Last modified date */
  WORD  ftime;      /* Last modified time */
  BYTE  fattrib;    /* Attribute */
  TCHAR fname[255];    /* Short file name (8.3 format) */
#if _USE_LFN
  TCHAR*  lfname;     /* Pointer to the LFN buffer */
  UINT  lfsize;     /* Size of LFN buffer in TCHAR */
#endif
} FILINFO;


void record_file(char* pfile_name);
int play_file(FILINFO* pfile_info);
uint32_t get_timer_tick();
void save_eeprom_data();
extern uint8_t g_fat_buffer[FAT_BUF_SIZE];
extern File g_fil;
extern volatile uint8_t g_invert_signal;
extern volatile uint16_t g_ticker_rate;
extern volatile uint16_t g_ticker_hold_rate;
extern volatile uint16_t g_key_repeat_next;
extern volatile uint16_t g_rec_finalize_time;
extern volatile uint8_t g_rec_auto_finalize;
extern uint8_t g_machine_type;
extern uint8_t g_video_mode;
extern volatile uint8_t g_is_paused;

#endif

