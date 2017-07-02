#include "arduino.h"
#include "pvStats.h"
#include "pvStatus.h"
#include "tftScreen.h"
#include "SPI.h"
#include "Adafruit_GFX.h"
#include "Adafruit_ILI9341.h"
#include "FS.h"


Adafruit_ILI9341 tft = Adafruit_ILI9341(4,15,2);

uint8_t graphLine[GRAPHWIDTH];
uint8_t graphLine_old[GRAPHWIDTH];
int graphLineOldLength=0;
void setupSPIFFS(void);
void bmpDraw(char *filename, uint8_t x, uint16_t y);
uint16_t read16(File &f);
uint32_t read32(File &f);

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
  if (pvStatus.instantaneousPower<1000)
  {
    tft.print(" ");  
  }
  if (pvStatus.instantaneousPower<100)
  {
    tft.print(" ");  
  }
  if (pvStatus.instantaneousPower<10)
  {
    tft.print(" ");  
  }
    
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
     tft.setCursor(TEXT_START_X+12,TOTALLY_GENERATED_KWH_START);
     tft.setTextSize(2);
     tft.print("kWh");  
   }
}   
  
  

void tftShowLedscircle(void)
{
  tft.fillScreen(ILI9341_WHITE);
  analogWriteRange(100);
    for (int i=0;i<=100;i++)
  {
    analogWrite(0,i);
    delay(15);
  }
  setupSPIFFS();
  tft.setTextColor(ILI9341_BLACK,ILI9341_WHITE);  
  tft.setTextSize(3);
  tft.setCursor(0,25);
  tft.print("   Windmill");
  delay(200); 
  tft.setCursor(0,50);
  tft.print("   Monitor");
  delay(200); 
  tft.setCursor(0,75);
  tft.print("   By:");
  delay(200); 
  bmpDraw("/V2_320width.bmp", 0,108);
}

void tftShowStartUpText(char* text,uint8_t row){
  tft.setTextColor(ILI9341_WHITE,ILI9341_BLACK);  
  tft.setTextSize(3);
  tft.setCursor(0,row*25);
  tft.print(text);
}

void tftShowPVOutput(void)
{
  tft.fillScreen(ILI9341_BLACK);
  bmpDraw("/pvout.bmp", 60,121);
}

void tftSetup(void){
    tft.begin();
    tft.setRotation(1);
}

void tftDrawGraphScreen(void)
{
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
  bmpDraw("/ledscircle105.bmp", TEXT_START_X+12,TOTALLY_GENERATED_KWH_START+22);
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
    return maxy; 
}

void setupSPIFFS(void){
  SPIFFS.begin();
}



//These routines are taken from adafruit example SPITFTBITMAP: 
//https://learn.adafruit.com/adafruit-2-dot-8-color-tft-touchscreen-breakout-v2/bitmaps-spi-mode
#define BUFFPIXEL 20

void bmpDraw(char *filename, uint8_t x, uint16_t y) {

  File     bmpFile;
  int      bmpWidth, bmpHeight;   // W+H in pixels
  uint8_t  bmpDepth;              // Bit depth (currently must be 24)
  uint32_t bmpImageoffset;        // Start of image data in file
  uint32_t rowSize;               // Not always = bmpWidth; may have padding
  uint8_t  sdbuffer[3*BUFFPIXEL]; // pixel buffer (R+G+B per pixel)
  uint8_t  buffidx = sizeof(sdbuffer); // Current position in sdbuffer
  boolean  goodBmp = false;       // Set to true on valid header parse
  boolean  flip    = true;        // BMP is stored bottom-to-top
  int      w, h, row, col;
  uint8_t  r, g, b;
  uint32_t pos = 0, startTime = millis();

  if((x >= tft.width()) || (y >= tft.height())) return;

  Serial.println();
  Serial.print(F("Loading image '"));
  Serial.print(filename);
  Serial.println('\'');

  // Open requested file on SD card
  if ((bmpFile = SPIFFS.open(filename,"r")) == NULL) {
    Serial.print(F("File not found"));
    return;
  }

  // Parse BMP header
  if(read16(bmpFile) == 0x4D42) { // BMP signature
    Serial.print(F("File size: ")); Serial.println(read32(bmpFile));
    (void)read32(bmpFile); // Read & ignore creator bytes
    bmpImageoffset = read32(bmpFile); // Start of image data
    Serial.print(F("Image Offset: ")); Serial.println(bmpImageoffset, DEC);
    // Read DIB header
    Serial.print(F("Header size: ")); Serial.println(read32(bmpFile));
    bmpWidth  = read32(bmpFile);
    bmpHeight = read32(bmpFile);
    if(read16(bmpFile) == 1) { // # planes -- must be '1'
      bmpDepth = read16(bmpFile); // bits per pixel
      Serial.print(F("Bit Depth: ")); Serial.println(bmpDepth);
      if((bmpDepth == 24) && (read32(bmpFile) == 0)) { // 0 = uncompressed

        goodBmp = true; // Supported BMP format -- proceed!
        Serial.print(F("Image size: "));
        Serial.print(bmpWidth);
        Serial.print('x');
        Serial.println(bmpHeight);

        // BMP rows are padded (if needed) to 4-byte boundary
        rowSize = (bmpWidth * 3 + 3) & ~3;

        // If bmpHeight is negative, image is in top-down order.
        // This is not canon but has been observed in the wild.
        if(bmpHeight < 0) {
          bmpHeight = -bmpHeight;
          flip      = false;
        }

        // Crop area to be loaded
        w = bmpWidth;
        h = bmpHeight;
        if((x+w-1) >= tft.width())  w = tft.width()  - x;
        if((y+h-1) >= tft.height()) h = tft.height() - y;

        // Set TFT address window to clipped image bounds
        tft.setAddrWindow(x, y, x+w-1, y+h-1);

        for (row=0; row<h; row++) { // For each scanline...

          // Seek to start of scan line.  It might seem labor-
          // intensive to be doing this on every line, but this
          // method covers a lot of gritty details like cropping
          // and scanline padding.  Also, the seek only takes
          // place if the file position actually needs to change
          // (avoids a lot of cluster math in SD library).
          if(flip) // Bitmap is stored bottom-to-top order (normal BMP)
            pos = bmpImageoffset + (bmpHeight - 1 - row) * rowSize;
          else     // Bitmap is stored top-to-bottom
            pos = bmpImageoffset + row * rowSize;
          if(bmpFile.position() != pos) { // Need seek?
            bmpFile.seek(pos,SeekSet);
            buffidx = sizeof(sdbuffer); // Force buffer reload
          }

          for (col=0; col<w; col++) { // For each pixel...
            // Time to read more pixel data?
            if (buffidx >= sizeof(sdbuffer)) { // Indeed
              bmpFile.read(sdbuffer, sizeof(sdbuffer));
              buffidx = 0; // Set index to beginning
            }

            // Convert pixel from BMP to TFT format, push to display
            b = sdbuffer[buffidx++];
            g = sdbuffer[buffidx++];
            r = sdbuffer[buffidx++];
            tft.pushColor(tft.color565(r,g,b));
          } // end pixel
        } // end scanline
        Serial.print(F("Loaded in "));
        Serial.print(millis() - startTime);
        Serial.println(" ms");
      } // end goodBmp
    }
  }

  bmpFile.close();
  if(!goodBmp) Serial.println(F("BMP format not recognized."));
}

// These read 16- and 32-bit types from the SD card file.
// BMP data is stored little-endian, Arduino is little-endian too.
// May need to reverse subscript order if porting elsewhere.

uint16_t read16(File &f) {
  uint16_t result;
  ((uint8_t *)&result)[0] = f.read(); // LSB
  ((uint8_t *)&result)[1] = f.read(); // MSB
  return result;
}

uint32_t read32(File &f) {
  uint32_t result;
  ((uint8_t *)&result)[0] = f.read(); // LSB
  ((uint8_t *)&result)[1] = f.read();
  ((uint8_t *)&result)[2] = f.read();
  ((uint8_t *)&result)[3] = f.read(); // MSB
  return result;
}

