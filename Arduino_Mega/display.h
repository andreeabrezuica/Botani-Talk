#ifndef DISPLAY_H
#define DISPLAY_H

#define LAN_ICON_H 6
#define LAN_ICON_W 8
#define FACE_ICON_H 16
#define FACE_ICON_W 16

Ucglib_ST7735_18x128x160_SWSPI ucg(/*scl=*/ 3, /*data=*/ 4, /*cd=*/ 5, /*cs=*/7, /*reset=*/ 6);

/* 
  The icons we display are arrays of booleans
  Each digit represents a pixel as follows:
    - 0: off (black) pixel
    - 1: on: (colored) pixel
  We use bools so we don't waste space for each color channel (3 channels * 4 bytes = 12 bytes per pixle)
  We also use a scaled down version and scale it up programatically when rendering it for the same reason as above
*/

// display an icon showing there is no internet connection
const bool noInternet[LAN_ICON_H][LAN_ICON_W] = {
  {1,0,1,0,0,0,0,0},
  {0,1,0,0,0,0,0,0},
  {1,0,1,0,0,0,0,1},
  {0,0,0,0,0,1,0,1},
  {0,0,0,1,0,1,0,1},
  {0,1,0,1,0,1,0,1}
};

// happy face used when soil is moist
const bool happyFace[FACE_ICON_H][FACE_ICON_W] = {
  {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
  {0,0,0,1,1,0,0,0,0,0,0,1,1,0,0,0},
  {0,0,1,0,0,1,0,0,0,0,1,0,0,1,0,0},
  {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
  {0,0,1,1,0,0,0,0,0,0,0,0,1,1,0,0},
  {0,0,1,1,0,0,0,0,0,0,0,0,1,1,0,0},
  {0,0,0,1,1,1,0,0,0,0,1,1,1,0,0,0},
  {0,0,0,0,1,1,1,1,1,1,1,1,0,0,0,0},
  {0,0,0,0,0,0,1,1,1,1,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}
};

// sad face when dry
const bool sadFace[FACE_ICON_H][FACE_ICON_W] = {
  {0,0,0,0,1,0,0,0,0,0,0,1,0,0,0,0},
  {0,0,1,1,0,0,0,0,0,0,0,0,1,1,0,0},
  {0,1,0,0,0,0,0,0,0,0,0,0,0,0,1,0},
  {1,0,0,1,1,1,0,0,0,0,1,1,1,0,0,1},
  {0,0,1,1,0,1,1,0,0,1,1,0,1,1,0,0},
  {0,0,1,1,0,0,1,0,0,1,1,0,0,1,0,0},
  {0,0,1,0,0,1,1,0,0,1,0,0,1,1,0,0},
  {0,0,0,1,1,1,0,0,0,0,1,1,1,0,0,0},
  {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
  {0,0,0,0,0,1,1,1,1,1,1,0,0,0,0,0},
  {0,0,0,0,1,1,1,1,1,1,1,1,0,0,0,0},
  {0,0,0,1,1,0,0,0,0,0,0,1,1,0,0,0},
  {0,0,0,1,0,0,0,0,0,0,0,0,1,0,0,0},
  {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}
};

/**
*@brief display a face on the lower right corner of the display
*@param scale: factor for upscaling the image
*@param happy: display happyFace if true, sadFace otherwise 
**/
void displayFace(int scale, bool happy) {
  for (uint8_t i = 0; i < FACE_ICON_H; i++) {
    for (uint8_t j = 0; j < FACE_ICON_W; j++) {
      if (happy)
        ucg.setColor(0, happyFace[i][j] * 255, 0); // green if happy
      else
        ucg.setColor(0, sadFace[i][j] * 255, sadFace[i][j] * 255); // yellow if sad
      ucg.drawBox(j+j*scale + ucg.getWidth() - 64, i+i*scale + 85, scale + 1, scale + 1); // scale it by the scale factor and shift it to the left
    }
  }
}

/**
*@brief display a small icon in the lower left corner showing that there is no internet connection
*@param scale: factor for upscaling the image
*@param enable: show if true, hide if false
**/
void displayNoInternet(uint8_t scale, bool enable = true) {
  for (uint8_t i = 0; i < LAN_ICON_H; i++) {
    for (uint8_t j = 0; j < LAN_ICON_W; j++) {
      ucg.setColor(0, 0, enable ? noInternet[i][j] * 255 : 0); // red
      ucg.drawBox(j+j*scale + 4, i+i*scale + ucg.getHeight() - 20, scale + 1, scale + 1);
    }
  }
}

/**
*@brief display a numeric sensor entry on the screen
*@param pos: x and y positions
*@param color: [blue, green, red]
*@param title: title for the value entry (will be white and bolded)
*@param value: value of the sensor, its color will be given by the parameter
*@param unit: a character to be displayed after the value (e.g. '%', 'm', 'Hz' etc.); 0 for none
*@param toInt: cast the value to integer if true
**/
void displaySensorValue(uint8_t pos[2], uint8_t color[3], char* title, float value, char* unit = 0, bool toInt = false) {
  ucg.setColor(255, 255, 255);
  ucg.setPrintPos(pos[0], pos[1]);
  ucg.setFont(ucg_font_helvB08_tr);
  ucg.print(title);
  ucg.setFont(ucg_font_7x13_mr);

  ucg.setColor(0, color[0], color[1], color[2]);
  ucg.setPrintPos(ucg.getWidth() - 52, pos[1]);
  if (toInt) {
    ucg.print((int) value);
  }
  else {
    ucg.print(value);
  }
  if (unit) {
    ucg.print(unit);
  }
  // print some empty space so no stray pixels are left if the number of characters lowers after a refresh
  ucg.print("    "); 
}

/**
*@brief display a text-value sensor entry on the screen
*@param pos: x and y positions
*@param color: [blue, green, red]
*@param title: title for the value entry (will be white and bolded)
*@param status: the status obtained by reading the sensor (e.g. "cold", "warm", "ok", "on", "off" etc.), its color will be given by the parameter
**/
void displaySensorValue(uint8_t pos[2], uint8_t color[3], char* title, char* status) {
  ucg.setColor(255, 255, 255);
  ucg.setPrintPos(pos[0], pos[1]);
  ucg.setFont(ucg_font_helvB08_tr);
  ucg.print(title);
  ucg.setFont(ucg_font_7x13_mr);
  
  ucg.setColor(0, color[0], color[1], color[2]);
  ucg.setPrintPos(ucg.getWidth() - 52, pos[1]);
  ucg.print(status);
}

#endif // end display.h