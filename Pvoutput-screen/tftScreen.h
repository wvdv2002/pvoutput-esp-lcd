#ifndef tftscreen_h
#define tftscreen_h

#define GRAPHWIDTH 175
#define SCREENHEIGTH 240
#define SCREENWIDTH 320
#define INTERFACECOLOR ILI9341_RED
#define GRAPHBORDER 16
#define GRAPH_Y_START 20
#define GRAPHHEIGHT 200
#define REQUEST_SIZE 288
#define SPACE_BETWEEN_STUFF 3
#define TEXT_START_X (GRAPHWIDTH+GRAPHBORDER+5)
#define CURRENTWATT_START_Y (SYSTEMNAME_DIVIDER_Y + SPACE_BETWEEN_STUFF)
#define TOTALLYGENERATED_START_Y (TOTALLYGENERATED_DIVIDER_Y+SPACE_BETWEEN_STUFF)
#define SYSTEMNAME_START_Y (1+SPACE_BETWEEN_STUFF)
#define SYSTEMNAME_TEXTSIZE 2
#define SYSTEMNAME_DIVIDER_Y (SYSTEMNAME_START_Y+SYSTEMNAME_TEXTSIZE*8+SPACE_BETWEEN_STUFF*2)
#define TOTALLYGENERATED_DIVIDER_Y (CURRENTWATT_START_Y+2*8+2*8+4*8+SPACE_BETWEEN_STUFF*4)
#define TOTALLY_GENERATED_KWH_START (TOTALLYGENERATED_START_Y+6*8+3*SPACE_BETWEEN_STUFF)
#define TIME_TEXTSIZE 2



void tftSetup(void);
void tftShowLedscircle(void);
void tftShowEAZWind(void);
void tftShowPVOutput(void);
void tftDrawGraphScreen(void);
void tftShowStartUpText(char*,uint8_t);
void screenTask(void);
int scaleForGraph(uint16_t graphWidth, uint16_t graphHeight,const int*,uint8_t*,int);
void tftDrawGraph(uint16_t,uint16_t,uint16_t,uint16_t,uint16_t,uint8_t*,uint16_t); //returns maximum y (from old data)
void tftDrawOverOldGraph(uint16_t,uint16_t,uint16_t,uint16_t,uint16_t,uint8_t*,uint16_t,uint8_t*,uint16_t); //returns maximum y (from old data)
void tftDrawGraphTimeLegend(uint16_t, uint16_t, uint16_t,uint16_t, uint16_t, uint8_t, uint8_t, uint16_t, uint16_t);
void fakeDataForGraph(int samples);

#endif
