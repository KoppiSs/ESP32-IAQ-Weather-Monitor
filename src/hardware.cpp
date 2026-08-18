#include "hardware.h"
#include <TFT_eSPI.h>
#include <lvgl.h>

// Instantiate the TFT display driver
TFT_eSPI tft = TFT_eSPI();

// This function reads the physical screen and translates it for LVGL
void my_touch_read(lv_indev_drv_t * indev_driver, lv_indev_data_t * data) {
    uint16_t touchX, touchY;
    
    // Ask TFT_eSPI if the screen is currently being pressed
    bool touched = tft.getTouch(&touchX, &touchY, 100);

    if(!touched) {
        data->state = LV_INDEV_STATE_REL; // Released
    } else {
        data->state = LV_INDEV_STATE_PR;  // Pressed
        data->point.x = touchX;
        data->point.y = touchY;

        // Debugging print
        Serial.printf("TOUCH DETECTED: X=%d, Y=%d\n", touchX, touchY);
    }
}

// The flush callback: LVGL gives us a rendered block of pixels, we push it to the screen.
void my_disp_flush(lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p) {
    uint32_t w = (area->x2 - area->x1 + 1);
    uint32_t h = (area->y2 - area->y1 + 1);

    tft.startWrite();
    tft.setAddrWindow(area->x1, area->y1, w, h);
    // Push the 16-bit color pixels to the screen
    tft.pushColors((uint16_t *)&color_p->full, w * h, true);
    tft.endWrite();

    // Tell LVGL we are done and it can render the next chunk
    lv_disp_flush_ready(disp);
}

void init_hardware() {
    // SET UP THE PHYSICAL BUTTON PIN
    pinMode(27, INPUT_PULLUP);

    // --- TURN ON THE BACKLIGHT ---
    pinMode(32, OUTPUT);
    digitalWrite(32, HIGH); 

    // 1. Boot up the physical screen
    tft.begin();
    tft.setRotation(1); // 1 = Landscape mode (480x320)
    
    // ================================================
    // --- TEMPORARY TOUCH SCREEN CALIBRATION BLOCK ---
    // ================================================
    //tft.fillScreen(TFT_BLACK);
    //tft.setCursor(20, 150);
    //tft.setTextFont(2);
    //tft.setTextSize(1);
    //tft.setTextColor(TFT_WHITE, TFT_BLACK);
    //tft.println("Touch the corners to calibrate!");

    //uint16_t calData[5];
    //tft.calibrateTouch(calData, TFT_RED, TFT_BLACK, 15);

    //Serial.println("\n--- COPY THIS NEW CALIBRATION DATA ---");
    //Serial.printf("uint16_t calData[5] = { %d, %d, %d, %d, %d };\n", 
    //              calData[0], calData[1], calData[2], calData[3], calData[4]);
    //Serial.println("--------------------------------------\n");

    // 2. Boot up the LVGL graphics engine
    lv_init();

    // 3. Create the LVGL display object
    static lv_disp_draw_buf_t draw_buf;
    static lv_color_t buf[480 * 10]; // Buffer for 10 lines
    lv_disp_draw_buf_init(&draw_buf, buf, NULL, 480 * 10);

    // 4. Initialize the display driver
    static lv_disp_drv_t disp_drv;
    lv_disp_drv_init(&disp_drv);
    disp_drv.hor_res = 480;
    disp_drv.ver_res = 320;
    disp_drv.flush_cb = my_disp_flush;
    disp_drv.draw_buf = &draw_buf;
    lv_disp_drv_register(&disp_drv);

    // 5. Initialize the Touch Input Device
    static lv_indev_drv_t indev_drv;
    lv_indev_drv_init(&indev_drv);
    indev_drv.type = LV_INDEV_TYPE_POINTER;
    indev_drv.read_cb = my_touch_read;
    lv_indev_drv_register(&indev_drv);
    
    // Tell TFT_eSPI to use default touch calibration
    uint16_t calData[5] = { 274, 3572, 292, 3492, 7 }; 
    tft.setTouch(calData);
}