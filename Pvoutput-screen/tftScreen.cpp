#include "arduino.h"
#include "pvStats.h"
#include "pvStatus.h"
#include "tftScreen.h"
#include "SPI.h"
#include "Adafruit_GFX.h"
#include "Adafruit_ILI9341.h"

Adafruit_ILI9341 tft = Adafruit_ILI9341(4,15,2);

uint8_t graphLine[GRAPHWIDTH];
uint8_t graphLine_old[GRAPHWIDTH];
int graphLineOldLength=0;

extern PvStats pvStats;
extern PvStatus pvStatus;
extern PvSystemService pvSystemService;
//--------------------------------------------

void plotLines()
{
  Serial.print("stat Amount:");
  Serial.println(pvStats.len);
  scaleForGraph(GRAPHHEIGHT,GRAPHWIDTH,pvStats.instantaneousPower,graphLine,pvStats.len);
  if (graphLineOldLength==pvStats.len)
  {
    tftDrawOverOldGraph(GRAPHBORDER,GRAPH_Y_START,GRAPHBORDER+GRAPHWIDTH,GRAPHHEIGHT+GRAPH_Y_START,ILI9341_BLACK,graphLine_old,ILI9341_GREEN,graphLine,pvStats.len);      
  }
  else if (graphLineOldLength>0)
  {
    tftDrawGraph(GRAPHBORDER,GRAPH_Y_START,GRAPHBORDER+GRAPHWIDTH,GRAPHHEIGHT+GRAPH_Y_START,ILI9341_BLACK,graphLine_old,graphLineOldLength);
  }
  tftDrawGraph(GRAPHBORDER,GRAPH_Y_START,GRAPHBORDER+GRAPHWIDTH,GRAPHHEIGHT+GRAPH_Y_START,ILI9341_GREEN,graphLine,pvStats.len);  
  graphLineOldLength=pvStats.len;
  memcpy(graphLine_old,graphLine,graphLineOldLength);
} 



void fakeDataForGraph(int samples)
{
  int i;
  pvStats.len=random(0,samples);
  for(i=0;i<REQUEST_SIZE;i++)
  {
    pvStats.instantaneousPower[i] = random(0,20000);
  } 
}

void screenTask(void)
{
  //Clear dataplaces
  //tft.fillRect(GRAPHBORDER,GRAPH_Y_START,GRAPHWIDTH,200-GRAPH_Y_START,ILI9341_BLACK);
  plotLines();
  char shortSystemName[9];

  tft.setTextSize(TIME_TEXTSIZE);
  tft.setCursor(2,SCREENHEIGTH-TIME_TEXTSIZE*8);
  tft.print(pvStats.startTime);

  tft.setCursor(TEXT_START_X-5*11-5,SCREENHEIGTH-TIME_TEXTSIZE*8);
  tft.print(pvStats.endTime);
  
  tft.setCursor(TEXT_START_X+12,SYSTEMNAME_START_Y);
  tft.setTextSize(SYSTEMNAME_TEXTSIZE);
  memcpy(shortSystemName,pvSystemService.systemName,sizeof(shortSystemName)-1);
  shortSystemName[sizeof(shortSystemName)-1]=0;
  tft.print(shortSystemName);
  
 
  if (pvStatus.instantaneousPower<10000)
    tft.setTextSize(4);
  else
    tft.setTextSize(3);
 
  tft.setCursor(TEXT_START_X+12,CURRENTWATT_START_Y+2*8+2*SPACE_BETWEEN_STUFF);
  tft.print(pvStatus.instantaneousPower);


  
  tft.setCursor(TEXT_START_X+12,TOTALLYGENERATED_START_Y+2*8+2*SPACE_BETWEEN_STUFF);
  tft.setTextSize(4);
  if (pvStatus.energyGeneration<1000)
  {
    tft.print(pvStatus.energyGeneration);
    tft.setCursor(TEXT_START_X+12,TOTALLYGENERATED_START_Y+6*8+3*SPACE_BETWEEN_STUFF);
    tft.setTextSize(2);
    tft.print("Wh  ");
  }
  else
  {
     tft.print(pvStatus.energyGeneration/1000);
     if (pvStatus.energyGeneration<100000)
     {
        tft.print(".");
        tft.print((pvStatus.energyGeneration%1000)/100); 
     }
     tft.setCursor(TEXT_START_X+12,TOTALLYGENERATED_START_Y+6*8+3*SPACE_BETWEEN_STUFF);
     tft.setTextSize(2);
     tft.print("kWh");  
   }
}   
  
  


void tftSetup(void){
  tft.begin();
  tft.setRotation(1);
  tft.fillScreen(ILI9341_BLACK);

  tftDrawGraphTimeLegend(GRAPHBORDER,GRAPH_Y_START,GRAPHBORDER+GRAPHWIDTH,220,ILI9341_WHITE,0,24,0,1200); 
  
  tft.drawRect(TEXT_START_X+1,0,SCREENWIDTH-1-TEXT_START_X,SCREENHEIGTH-1,INTERFACECOLOR); //all encompassing rectangle for text.
  
  tft.drawFastHLine(TEXT_START_X+1,SYSTEMNAME_DIVIDER_Y,SCREENWIDTH-1-TEXT_START_X,INTERFACECOLOR); //first divider in text (under name)
  tft.drawFastHLine(TEXT_START_X+1,TOTALLYGENERATED_DIVIDER_Y,SCREENWIDTH-1-TEXT_START_X,INTERFACECOLOR); //second divider in text (current generated)
  
  tft.setTextSize(2);
  tft.setCursor(TEXT_START_X+12,CURRENTWATT_START_Y);
  tft.print("Cur. gen");
  tft.setCursor(TEXT_START_X+12,(CURRENTWATT_START_Y+3*SPACE_BETWEEN_STUFF+6*8));
  tft.setTextSize(2);
  tft.print("Watt");
  
  tft.setCursor(TEXT_START_X+12,TOTALLYGENERATED_START_Y);
  tft.print("Tot. gen");

  delay(100);
}  


 
void tftDrawOverOldGraph(uint16_t x0,uint16_t y0,uint16_t x1,uint16_t y1,uint16_t aOldColor,uint8_t oldData[],uint16_t aColor,uint8_t data[],uint16_t datasize)
{
   int x;
   int xlength=x1-x0-1;
   if (datasize<xlength)
   {
     for (x=0;x<(datasize-1);x=x+2)
     {
      tft.drawLine(x0+map(x,0,datasize,0,xlength),y1-oldData[x],x0+map(x+1,0,datasize,0,xlength),y1-oldData[x+1],aOldColor);
      tft.drawLine(x0+map(x,0,datasize,0,xlength),y1-data[x],x0+map(x+1,0,datasize,0,xlength),y1-data[x+1],aColor);
     }
     for (x=1;x<(datasize-1);x=x+2)
     {
      tft.drawLine(x0+map(x,0,datasize,0,xlength),y1-oldData[x],x0+map(x+1,0,datasize,0,xlength),y1-oldData[x+1],aOldColor);
      tft.drawLine(x0+map(x,0,datasize,0,xlength),y1-data[x],x0+map(x+1,0,datasize,0,xlength),y1-data[x+1],aColor);
     }
  }
}


 
void tftDrawGraph(uint16_t x0,uint16_t y0,uint16_t x1,uint16_t y1,uint16_t aColor,uint8_t data[],uint16_t datasize)
{
   int x;
   int xlength=x1-x0-1;
   if (datasize<xlength)
   {
     for (x=0;x<(datasize-1);x++)
     {
      tft.drawLine(x0+map(x,0,datasize,0,xlength),y1-data[x],x0+map(x+1,0,datasize,0,xlength),y1-data[x+1],aColor);
     }
  }
}

void tftDrawGraphTimeLegend(uint16_t x0, uint16_t y0, uint16_t x1,uint16_t y1, uint16_t aColor, uint8_t minhour, uint8_t maxhour, uint16_t minY, uint16_t maxY)
{
  uint8_t i;
  //X line
  tft.drawLine(x0-2,y0,x0-2,y1,aColor);
  tft.drawLine(x0-1,y0,x0-1,y1,aColor);
  
  //Y Line
  tft.drawLine(x0-2,y1+1,x1,y1+1,aColor);
  tft.drawLine(x0-2,y1+2,x1,y1+2,aColor);

  String text = "Watt";
  tft.setTextSize(2);
  tft.setTextColor(ILI9341_WHITE,ILI9341_BLACK);  
  for(i=0;i<4;i++)
  {
    tft.setCursor(x0-14, y0+i*16);
    tft.print(text[i]);
  }
  text = "Time";
  tft.setCursor(x1-5*11,y1+6);
//  tft.print(text);
}

int scaleForGraph(uint16_t graphHeight,uint16_t graphWidth, const int* data_in,uint8_t* data_out,int aLength)
{
    int maxy=data_in[0];
    int miny=data_in[0];
    int i;
    for(i=0;i<aLength;i++)
    {
     if(data_in[i]>maxy)
     {
        maxy=data_in[i];
     }
     if(data_in[i]<miny)
     {
        miny=data_in[i];
     }
    }
    
    Serial.print("maxy: ");
    Serial.println(maxy);
    Serial.print("miny: ");
    Serial.println(miny);
    
    if (aLength>graphWidth)
    {
      aLength=graphWidth;
    }
    if (maxy<1000)
    {
      maxy = 1000;
    }
      for(i=0;i<aLength;i++)
      {
        data_out[i] = map(data_in[i],0,maxy,0,graphHeight); 
        Serial.print(data_out[i]);
        Serial.print(",");             
      }
      Serial.println("");
/*
    }
    else
    {
      for(i=0;i<aLength;i++)
      {
        data_out[i+(GRAPHWIDTH-aLength)] = map(data_in[i],0,maxy,0,GRAPHHEIGHT);      
      }
      for(i=0;i<(GRAPHWIDTH-aLength);i++)
      {
        data_out[i] = 0;
      }
    }   
*/
    
    return maxy; 
}

