#include <TouchScreen.h>


// IMPORTANT: Adafruit_TFTLCD LIBRARY MUST BE SPECIFICALLY
// CONFIGURED FOR EITHER THE TFT SHIELD OR THE BREAKOUT BOARD.
// SEE RELEVANT COMMENTS IN Adafruit_TFTLCD.h FOR SETUP.
////Technical support:goodtft@163.com
#include <Adafruit_GFX.h>    // Core graphics library
//#include <Adafruit_TFTLCD.h> // Hardware-specific library
#include <MCUFRIEND_kbv.h>
#include <SPI.h>

#include "tapuino.h"
#include "IEC.h"
#include "SerialCommand.h"
#include <SPI.h>
#include <SD.h>
#include "d64driver.h"
// The control pins for the LCD can be assigned to any digital or
// analog pins...but we'll use the analog pins as this allows us to
// double up the pins with the touch screen (see the TFT paint example).
#define LCD_CS A3 // Chip Select goes to Analog 3
#define LCD_CD A2 // Command/Data goes to Analog 2
#define LCD_WR A1 // LCD Write goes to Analog 1
#define LCD_RD A0 // LCD Read goes to Analog 0

#define LCD_RESET A4 // Can alternately just connect to Arduino's reset pin

// Assign human-readable names to some common 16-bit color values:
#define  BLACK   0x0000
#define BLUE    0x001F
#define RED     0xF800
#define GREEN   0x07E0
#define CYAN    0x07FF
#define MAGENTA 0xF81F
#define YELLOW  0xFFE0
#define WHITE   0xFFFF
#define SPI_PIN 53

//Adafruit_TFTLCD tft(LCD_CS, LCD_CD, LCD_WR, LCD_RD, LCD_RESET);
MCUFRIEND_kbv tft;

File myFile;

uint8_t g_fat_buffer[FAT_BUF_SIZE];

/*************************************************************************************************************************
 *  VARIABLES ASOCIADAS AL GESTOR DE FICHEROS
 */
#define X_START 0
#define X_END 1
#define Y_START 2
#define Y_END 3 
#define FILELIST_BOUNDARY 0
#define BUTTONUP_BOUNDARY 1
#define BUTTONDOWN_BOUNDARY 2
int16_t boundaries[2][5][5]={
  {
    {0,180,20,280,BLUE}, // Listado de ficheros
    {185,30,20,30,BLUE},
    {185,30,270,30,BLUE}
  },
 {
  {60,105,100,30,BLUE}, // Play
  {60,105,140,30,BLUE}, // Stop
  {60,105,180,30,BLUE}, // None
  {60,105,220,30,BLUE}, // Rec
  {45,150,280,30,BLUE} // Back
 }
  };


#define FILELIST_FILE_HEIGHT 17
#define FILELIST_NFILES 15
char message[100];
char currentDir[255];
int viewOffset=0;
int selectedIndex=0;
int firstFileIndex=0;
int nFiles=0;
int nDirFiles=0;
char currentFiles[FILELIST_NFILES][100];
int currentTypes[FILELIST_NFILES];
char currentPath[255];
char selectedGame[15];
int status=0;
#define YP A1  // must be an analog pin, use "An" notation!
#define XM A2  // must be an analog pin, use "An" notation!
#define YM 7   // can be a digital pin
#define XP 6   // can be a digital pin

#define TS_MINX 150
#define TS_MINY 120
#define TS_MAXX 920
#define TS_MAXY 940

// For better pressure precision, we need to know the resistance
// between X+ and X- Use any multimeter to read it
// For the one we're using, its 300 ohms across the X plate
TouchScreen ts = TouchScreen(XP, YP, XM, YM, 300);

// Predeclaracion
void drawState0UI();
extern void tapuino_run();

void setup(void) {    
  Serial.begin(115200);
  
  uint16_t identifier = tft.readID();
  Serial.print("TFT ID IS:");Serial.println(identifier);
  if(identifier==0x0101)
      identifier=0x9341;
  
  tft.begin(identifier);
  tft.setRotation(4);
  //tft.reset();
  tft.fillScreen(BLACK);
  
  pinMode(SS, OUTPUT);
  if (!SD.begin(SPI_PIN)) {
    Serial.println("initialization failed!");
    return;
  }
  Serial.println("initialization done.");
  strcpy(currentPath,"/");
  tft.setTextSize(2);
  tft.setTextColor(WHITE);
  tft.setTextWrap(false);
  tapuino_run();
  drawState0UI();
}

void lcd_title_P(char *str)
{
  return;
  tft.fillRect(0,0,200,20,BLACK);
  tft.setCursor(0,0);
  tft.println(str);
}
void drawByBoundary(int boundary)
{
  int16_t *bounds;
   bounds=&(boundaries[status][boundary][0]);  
   tft.fillRect((*bounds),*(bounds+2),*(bounds+1),*(bounds+3),*(bounds+4));
}

void drawState1UI()
{
  tft.fillRect(0,0,tft.width(),tft.height(),BLACK);
  tft.setCursor(50,55);
    tft.println(selectedGame);
    
  switch(status)
  {
    case 1:{
    
    for(int i=0;i<5;i++)
      drawByBoundary(i);
    
    tft.setCursor(90,110);
    tft.print("Play");
    tft.setCursor(90,150);
    tft.print("Stop");
    tft.setCursor(85,190);
    tft.print("Eject");
    tft.setCursor(95,230);
    tft.print("Rec");
    tft.setCursor(90,290);
    tft.print("Volver");
    }break;
    case 10:
    {
        CommandFactory c(8,selectedGame);
        c.run();
    }
  }
};
    
void refreshList()
{
  int16_t *bounds=&(boundaries[0][FILELIST_BOUNDARY][0]);  
  drawByBoundary(FILELIST_BOUNDARY);
  
  tft.setCursor(0,0);
  tft.fillRect(0,0,tft.width(),20,BLACK);
  tft.setTextColor(WHITE);
  tft.println(currentPath);
  File dir=SD.open(currentPath);
  dir.rewindDirectory();
  nFiles=0;
  nDirFiles=0;
  char fileName[25];  
  Serial.print("Iniciando listado en ");
  Serial.print(firstFileIndex);
  File entry;
  while(true) {    
    if(nDirFiles < 2)
    {
      if(nDirFiles >=firstFileIndex)
      {
        tft.setTextColor(YELLOW);
        tft.setCursor(0,*(bounds+2)+nFiles*FILELIST_FILE_HEIGHT);
        tft.println(nDirFiles==0?".":"..");
        strncpy(currentFiles[nFiles],nDirFiles==0?".":"..",100);
        currentTypes[nFiles]=1;
        nFiles++;     
      }      
      nDirFiles++;
      continue;
    }
    entry =  dir.openNextFile();    
    
    if (!entry) {              
       break;
    }
    if(nDirFiles < firstFileIndex || nFiles>=FILELIST_NFILES)
    {
      entry.close();
      nDirFiles++;      
      continue;  
    }
    
         
     strncpy(currentFiles[nFiles],entry.name(),100);
     strncpy(fileName,entry.name(),25);         
     if(entry.isDirectory())   
     {  
       tft.setTextColor(YELLOW);
       currentTypes[nFiles]=1;
     }
     else
     {
       tft.setTextColor(WHITE);
       currentTypes[nFiles]=0;
     }
                 
     tft.setCursor(0,25+nFiles*17);
     tft.println(fileName);     
     entry.close();     
     nDirFiles++;
     nFiles++;
   }
   dir.close();
   
}

void drawState0UI()
{
    refreshList();
    drawByBoundary(BUTTONUP_BOUNDARY);
    drawByBoundary(BUTTONDOWN_BOUNDARY);
}


#define MINPRESSURE 10
#define MAXPRESSURE 600

int touchStart=0;

bool isContained(int x,int y,int id)
{
  int16_t *bounds=boundaries[status][id];  
  x=x-(*bounds);
  y=y-*(bounds+2);
 
  if((x >= 0 && x<=*(bounds+1)) &&
     (y >= 0 && y<=*(bounds+3)))
      return true;
  return false;
}
int playing=0;
void processClick(int x, int y)
{
   x = map(x, TS_MAXX, TS_MINX, tft.width(), 0);
   y = map(y, TS_MINY, TS_MAXY, tft.height(), 0);

 
   
  switch(status)
  {
    case 0: 
    {      
      if(isContained(x,y,FILELIST_BOUNDARY))
      {
          int oy=y-boundaries[status][FILELIST_BOUNDARY][Y_START];
              
          int ofY=(oy/FILELIST_FILE_HEIGHT);
         
          
          if(ofY >= nFiles)
            return;  
            
          selectedIndex=firstFileIndex+ofY;
          if(selectedIndex < 0)
            return;

          if(currentTypes[ofY]==1)
          {
             // Es un directorio
             if(!strcmp(".",currentFiles[ofY]))
                return;
             firstFileIndex=0;
             if(!strcmp("..",currentFiles[ofY]))
             {
                  if(!strcmp("/",currentPath))
                    return;
                  String s(currentPath);
                  int idx=s.lastIndexOf("/");
                  if(idx==0)
                  {
                     currentPath[0]='/';
                     currentPath[1]=0;
                  }
                  else
                  {
                    s[s.lastIndexOf("/")]=0;                    
                    s.toCharArray(currentPath,255);
                  }
                  Serial.print("NUEVO DIRECTORIO:");
                  Serial.println(currentPath);
             }
             else
             {
              Serial.print("CURRENTPATH:");
              Serial.println(currentPath);
              
                if(strcmp(currentPath,"/"))
                {
                   strcat(currentPath,"/"); 
                }
                strcat(currentPath,currentFiles[ofY]);
             }
          }
          else
            {
              // Seleccionar fichero para modo 1
              strcpy(selectedGame,currentFiles[ofY]);
              int len=strlen(selectedGame);
              if(!strcmp("D64",selectedGame+len-3))
                status=10;
              if(!strcmp("TAP",selectedGame+len-3))
                status=1;
              drawState1UI();
              return;
            }
          
          drawState0UI();
          return;
      }
      
      if(isContained(x,y,BUTTONUP_BOUNDARY))
      {
        Serial.println("PULSADO ARRIBA");
          if(firstFileIndex==0)
            return;
          firstFileIndex-=FILELIST_NFILES;
          if(firstFileIndex < 0)
            firstFileIndex=0;
         drawState0UI();
      }
      if(isContained(x,y,BUTTONDOWN_BOUNDARY))
      {
        Serial.println("PULSADO ABAJO");
          if(firstFileIndex+FILELIST_NFILES < nDirFiles)
          {
            firstFileIndex+=FILELIST_NFILES;
            
            drawState0UI();
          }
          
      }                 
    }break;
    case 1:
    {
        for(int i=0;i<5;i++)
        {
          if(isContained(x,y,i))
          {
            switch(i)
            {
              case 0:
              {          
                if(playing==1)
                  return;
                playing=1;
                FILINFO info;
                Serial.println("PULSADO PLAY");  
                if(strcmp("/",currentPath))
                  sprintf(info.fname,"%s/%s",currentPath,selectedGame);                
                else
                  sprintf(info.fname,"/%s",selectedGame);
                      
                File f=SD.open(info.fname);                
                // Se crea un objeto como el que quiere tapuino..
                
                
                  Serial.print("FISIZE:::::");
                  Serial.println((long)f.size());
                  long realLong=2*f.size();
                  Serial.println(realLong);
                info.fsize=f.size();
                f.close();
                Serial.println("LLAMO A PLAYFILE");
                play_file(&info);
                
              }break;
              case 4:
              {
                status=0;
                tft.fillRect(0,0,tft.width(),tft.height(),BLACK);
                drawState0UI();
              }break;
            }
          }
        }
        
    }break;
    case 10:
    {
      
    }break;
  }
}

int tt=0;
void loop(void) {
    
 TSPoint p = ts.getPoint();  
 pinMode(XM, OUTPUT);
  pinMode(YP, OUTPUT);
  
  // we have some minimum pressure we consider 'valid'
  // pressure of 0 means no pressing!
  if(touchStart>5)
    touchStart=0;
    
  if (p.z > MINPRESSURE && p.z < MAXPRESSURE) {
             
    if(touchStart<5)
        touchStart++;
  }
  else
  {
    if(touchStart==5)
    {
      processClick(p.x,p.y);
      touchStart=0;
    }
  }
  
}

