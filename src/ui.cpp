#include "ui.h"
#include <lvgl.h>

LV_FONT_DECLARE(lv_font_montserrat_16);
LV_FONT_DECLARE(lv_font_montserrat_20);
LV_FONT_DECLARE(lv_font_montserrat_24);
LV_FONT_DECLARE(custom_icon_font); 
LV_FONT_DECLARE(custom_icon_font_60);

static lv_font_t font_16_icons;
static lv_font_t font_20_icons;
static lv_font_t font_24_icons;

lv_obj_t * iaq_arc;                 
lv_obj_t * indoor_center;
lv_obj_t * indoor_temp;
lv_obj_t * indoor_hum;
lv_obj_t * indoor_pres;
lv_obj_t * indoor_voc;

lv_obj_t * slider_temp;
lv_obj_t * slider_hum;
lv_obj_t * slider_pres;
lv_obj_t * slider_voc;

lv_obj_t * outdoor_icon;
lv_obj_t * outdoor_desc;
lv_obj_t * outdoor_temp;
lv_obj_t * outdoor_hum;
lv_obj_t * outdoor_wind;

lv_obj_t * out_bar_temp;
lv_obj_t * out_bar_hum;
lv_obj_t * out_bar_wind;

lv_obj_t * forecast_labels[12]; 
lv_obj_t * tabview;    

String short_weather(int code) {
    switch (code) {
        case 0: return "Clear";
        case 1: case 2: case 3: return "Cloudy";
        case 45: case 48: return "Fog";
        case 51: case 53: case 55: return "Drizzle";
        case 56: case 57: return "F.Driz";
        case 61: case 63: case 65: return "Rain";
        case 66: case 67: return "F.Rain";
        case 71: case 73: case 75: return "Snow";
        case 77: return "Grains";
        case 80: case 81: case 82: return "Shower";
        case 85: case 86: return "S.Shwr";
        case 95: case 96: case 99: return "Storm";
        default: return "Unk";
    }
}

String weather_icon(int code) {
    switch (code) {
        case 0: return "\xef\x86\x85"; 
        case 1: case 2: case 3: return "\xef\x83\x82"; 
        case 45: case 48: return "\xef\x9d\x9f"; 
        case 51: case 53: case 55:
        case 56: case 57:
        case 61: case 63: case 65:
        case 66: case 67:
        case 80: case 81: case 82: return "\xef\x9d\x80"; 
        case 71: case 73: case 75:
        case 77:
        case 85: case 86: return "\xef\x8b\x9c"; 
        case 95: case 96: case 99: return "\xef\x83\xa7"; 
        default: return "\xef\x83\x82"; 
    }
}

// Helper functions to determine colour for indoor readings (green, orange, or red)
lv_color_t get_temp_color(float t) {
    if (t >= 20 && t <= 24) return lv_color_hex(0x00FF00); // Green
    if ((t >= 18 && t < 20) || (t > 24 && t <= 27)) return lv_color_hex(0xFFaa00); // Orange for 
    return lv_color_hex(0xFF0000); // Red
}

lv_color_t get_hum_color(float h) {
    if (h >= 30 && h <= 60) return lv_color_hex(0x00FF00); // Green
    if ((h >= 20 && h < 30) || (h > 60 && h <= 70)) return lv_color_hex(0xFFaa00); // Orange
    return lv_color_hex(0xFF0000); // Red
}

lv_color_t get_voc_color(float v) {
    if (v <= 1.0) return lv_color_hex(0x00FF00); // Green
    if (v > 1.0 && v <= 2.5) return lv_color_hex(0xFFaa00); // Orange
    return lv_color_hex(0xFF0000); // Red
}

// IAQ Arc colour helper function
lv_color_t get_iaq_color(int iaq) {
    if (iaq <= 50)  return lv_color_hex(0x00FF00); // Light Green (Excellent)
    if (iaq <= 100) return lv_color_hex(0x32CD32); // Lime Green (Good)
    if (iaq <= 150) return lv_color_hex(0xFFFF00); // Yellow (Lightly Polluted)
    if (iaq <= 200) return lv_color_hex(0xFFaa00); // Orange (Moderately Polluted)
    if (iaq <= 250) return lv_color_hex(0xFF6666); // Light Red (Heavily Polluted)
    if (iaq <= 350) return lv_color_hex(0xFF0000); // Red (Severely Polluted)
    return lv_color_hex(0x8B0000);                 // Dark Red (Extreme)
}

// Creates the container that holds the color blocks
lv_obj_t* create_bg_track(lv_obj_t* parent, lv_obj_t* align_obj, lv_align_t align_pos) {
    lv_obj_t * track = lv_obj_create(parent);
    lv_obj_set_size(track, 100, 8);
    lv_obj_align_to(track, align_obj, align_pos, 0, 5);
    lv_obj_set_style_pad_all(track, 0, LV_PART_MAIN);
    lv_obj_set_style_border_width(track, 0, LV_PART_MAIN);
    lv_obj_set_style_radius(track, 4, LV_PART_MAIN); 
    lv_obj_set_style_clip_corner(track, true, LV_PART_MAIN); // Rounds the edges of the colors inside
    lv_obj_clear_flag(track, LV_OBJ_FLAG_SCROLLABLE);
    return track;
}

// Drops a color block into the container at an exact X offset
void add_segment(lv_obj_t* track, uint32_t color, int x_offset, int width) {
    lv_obj_t * seg = lv_obj_create(track);
    lv_obj_set_size(seg, width, 8);
    lv_obj_align(seg, LV_ALIGN_LEFT_MID, x_offset, 0);
    lv_obj_set_style_bg_color(seg, lv_color_hex(color), LV_PART_MAIN);
    lv_obj_set_style_radius(seg, 0, LV_PART_MAIN);
    lv_obj_set_style_border_width(seg, 0, LV_PART_MAIN);
    lv_obj_clear_flag(seg, LV_OBJ_FLAG_SCROLLABLE);
}

// Creates the invisible slider with the white ball
lv_obj_t* create_indicator_slider(lv_obj_t* parent, lv_obj_t* track, int min, int max) {
    lv_obj_t * slider = lv_slider_create(parent);
    lv_obj_set_size(slider, 100, 13); // Slightly taller than the track so the ball fits
    lv_obj_align_to(slider, track, LV_ALIGN_CENTER, 0, 0);
    lv_slider_set_range(slider, min, max);
    
    lv_obj_set_style_bg_opa(slider, 0, LV_PART_MAIN);      // Invisible background
    lv_obj_set_style_bg_opa(slider, 0, LV_PART_INDICATOR); // Invisible fill
    
    // Style the Ball (Knob)
    lv_obj_set_style_bg_color(slider, lv_color_hex(0xFFFFFF), LV_PART_KNOB);
    lv_obj_set_style_border_color(slider, lv_color_hex(0x000000), LV_PART_KNOB);
    lv_obj_set_style_border_width(slider, 1, LV_PART_KNOB);
    lv_obj_set_style_pad_all(slider, 0, LV_PART_KNOB); 
    
    lv_obj_clear_flag(slider, LV_OBJ_FLAG_CLICKABLE); // Read-only so user can't drag it
    return slider;
}

// Event callback for info windows of indoor data points
static void info_popup_cb(lv_event_t * e) {
    // Grab the specific text we passed into the event
    const char * info_text = (const char *)lv_event_get_user_data(e);
    
    // Create a pop-up window on the current screen with a Close button (true)
    lv_obj_t * mbox = lv_msgbox_create(lv_scr_act(), "Zone Guide", info_text, NULL, true);
    lv_obj_center(mbox);
    
    // Dim the background behind the pop-up to make it look professional
    lv_obj_t * bg = lv_msgbox_get_title(mbox); // Just grabbing a piece to style
    lv_obj_set_style_bg_color(mbox, lv_color_hex(0x1a1a1a), LV_PART_MAIN);
    lv_obj_set_style_text_color(mbox, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
}

// --- OUTDOOR FLUID COLOR HELPERS ---
lv_color_t get_out_temp_color(int t) {
    if (t < 0)  return lv_color_hex(0x0000FF);      // Deep Blue (Freezing)
    if (t < 10) return lv_color_hex(0x00BFFF);      // Light Blue (Cold)
    if (t < 20) return lv_color_hex(0x00FF00);      // Green (Mild)
    if (t < 26) return lv_color_hex(0xFFaa00);      // Orange (Warm)
    return lv_color_hex(0xFF0000);                  // Red (Hot)
}

lv_color_t get_out_hum_color(int h) {
    if (h < 30) return lv_color_hex(0xFFaa00);      // Orange (Very Dry)
    if (h <= 70) return lv_color_hex(0x00FF00);     // Green (Comfortable)
    return lv_color_hex(0x00BFFF);                  // Blue (High Humidity/Rainy)
}

lv_color_t get_out_wind_color(int w) {
    if (w < 20)  return lv_color_hex(0x00FF00);      // Green (Calm/Light Breeze)
    if (w < 40) return lv_color_hex(0xFFFF00);      // Yellow (Moderate Breeze)
    if (w < 60) return lv_color_hex(0xFFaa00);      // Orange (Strong Wind)
    return lv_color_hex(0xFF0000);                  // Red (Gale Force)
}

void init_ui() {
    
    font_16_icons = lv_font_montserrat_16;
    font_16_icons.fallback = &custom_icon_font;

    font_20_icons = lv_font_montserrat_20;
    font_20_icons.fallback = &custom_icon_font;

    font_24_icons = lv_font_montserrat_24;
    font_24_icons.fallback = &custom_icon_font;

    lv_obj_t * scr = lv_scr_act(); 

    // --- BACKGROUND STYLING ---
    lv_obj_set_style_bg_color(scr, lv_color_hex(0x004A8F), LV_PART_MAIN);      
    lv_obj_set_style_bg_grad_color(scr, lv_color_hex(0x001B3E), LV_PART_MAIN); 
    lv_obj_set_style_bg_grad_dir(scr, LV_GRAD_DIR_VER, LV_PART_MAIN);          

    // --- TABVIEW SETUP ---
    tabview = lv_tabview_create(scr, LV_DIR_TOP, 50);
    lv_obj_set_style_bg_opa(tabview, 0, LV_PART_MAIN); 

    //lv_obj_t * tab_bar = lv_tabview_get_tab_bar(tabview); //V9
    lv_obj_t * tab_bar = lv_tabview_get_tab_btns(tabview); //V8
    lv_obj_set_height(tab_bar, 50); 
    lv_obj_set_style_bg_color(tab_bar, lv_color_hex(0x000000), LV_PART_MAIN); 
    lv_obj_set_style_bg_opa(tab_bar, 100, LV_PART_MAIN); 
    
    lv_obj_set_style_text_font(tab_bar, &lv_font_montserrat_20, LV_PART_ITEMS);
    lv_obj_set_style_text_opa(tab_bar, 255, LV_PART_ITEMS | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(tab_bar, 255, LV_PART_ITEMS | LV_STATE_CHECKED);
    lv_obj_set_style_text_color(tab_bar, lv_color_hex(0x00FFFF), LV_PART_ITEMS | LV_STATE_DEFAULT); 
    lv_obj_set_style_text_color(tab_bar, lv_color_hex(0xFFFFFF), LV_PART_ITEMS | LV_STATE_CHECKED);

    // DISABLING TAB SLIDING ANIMATIONS
    // Grab the invisible container that holds all the tabs
    lv_obj_t * tab_content = lv_tabview_get_content(tabview);
    // 1. Disable finger swiping (but keep button clicks working)
    lv_obj_clear_flag(tab_content, LV_OBJ_FLAG_SCROLLABLE);
    // 2. Set the slide animation time to 0ms (Instant snap)
    lv_obj_set_style_anim_time(tab_content, 0, LV_PART_MAIN);

    lv_obj_t * tab_indoor = lv_tabview_add_tab(tabview, "Indoor");
    lv_obj_t * tab_outdoor = lv_tabview_add_tab(tabview, "Weather");
    lv_obj_t * tab_forecast = lv_tabview_add_tab(tabview, "Forecast");

    lv_obj_set_style_bg_opa(tab_indoor, 0, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(tab_outdoor, 0, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(tab_forecast, 0, LV_PART_MAIN);

    // These are to kill scrollbars
    lv_obj_clear_flag(tab_indoor, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_clear_flag(tab_outdoor, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_clear_flag(tab_forecast, LV_OBJ_FLAG_SCROLLABLE);

    // ==========================================
    //            PAGE 1: INDOOR
    // ==========================================
    
    iaq_arc = lv_arc_create(tab_indoor);
    lv_obj_set_size(iaq_arc, 140, 140); 
    lv_obj_align(iaq_arc, LV_ALIGN_CENTER, 0, -10);
    lv_arc_set_rotation(iaq_arc, 135);                 
    lv_arc_set_bg_angles(iaq_arc, 0, 270);             
    lv_arc_set_value(iaq_arc, 0);                      
    lv_arc_set_range(iaq_arc, 0, 500);                 
    lv_obj_remove_style(iaq_arc, NULL, LV_PART_KNOB);               
    lv_obj_clear_flag(iaq_arc, LV_OBJ_FLAG_CLICKABLE);              
    lv_obj_set_style_arc_color(iaq_arc, lv_color_hex(0x00FFcc), LV_PART_INDICATOR); 
    lv_obj_set_style_arc_color(iaq_arc, lv_color_hex(0x1a1a1a), LV_PART_MAIN);      
    lv_obj_set_style_arc_width(iaq_arc, 12, LV_PART_MAIN);
    lv_obj_set_style_arc_width(iaq_arc, 12, LV_PART_INDICATOR);

    // IAQ INDEX
    indoor_center = lv_label_create(iaq_arc);
    lv_obj_align(indoor_center, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_text_color(indoor_center, lv_color_hex(0x00FFcc), LV_PART_MAIN); 
    lv_obj_set_style_text_font(indoor_center, &font_24_icons, LV_PART_MAIN); 
    lv_obj_set_style_text_align(indoor_center, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);
    lv_obj_add_flag(indoor_center, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(indoor_center, info_popup_cb, LV_EVENT_CLICKED, (void*)"0-50: Excellent\n51-100: Good\n101-150: Lightly Polluted\n151-200: Moderately Polluted\n> 200: Heavily Polluted\n\nLower is better! Open a window if the number exceeds 150.");

    // TEMP
    indoor_temp = lv_label_create(tab_indoor);
    lv_obj_set_style_text_color(indoor_temp, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
    lv_obj_set_style_text_font(indoor_temp, &font_20_icons, LV_PART_MAIN); 
    lv_obj_align(indoor_temp, LV_ALIGN_TOP_LEFT, 15, 15); 
    lv_obj_add_flag(indoor_temp, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(indoor_temp, info_popup_cb, LV_EVENT_CLICKED, (void*)"Green: 20-24 C\nOrange: 18-20 C, 24-27 C\nRed: <18 C, >27 C\n\nIdeal room temperature for focus and sleep is slightly cool.");

    // HUMIDITY
    indoor_hum = lv_label_create(tab_indoor);
    lv_obj_set_style_text_color(indoor_hum, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
    lv_obj_set_style_text_font(indoor_hum, &font_20_icons, LV_PART_MAIN); 
    lv_obj_align(indoor_hum, LV_ALIGN_BOTTOM_LEFT, 15, -30); 
    lv_obj_add_flag(indoor_hum, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(indoor_hum, info_popup_cb, LV_EVENT_CLICKED, (void*)"Green: 30-60 %\nOrange: 20-30 %, 60-70 %\nRed: <20 %, >70 %\n\nHigh humidity invites mold.\nLow humidity dries airways.");

    // PRESSURE
    indoor_pres = lv_label_create(tab_indoor);
    lv_obj_set_style_text_color(indoor_pres, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
    lv_obj_set_style_text_font(indoor_pres, &font_20_icons, LV_PART_MAIN); 
    lv_obj_align(indoor_pres, LV_ALIGN_TOP_RIGHT, -15, 15); 
    lv_obj_add_flag(indoor_pres, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(indoor_pres, info_popup_cb, LV_EVENT_CLICKED, (void*)"Typical: 980 - 1030 hPa\n\nHigh pressure brings clear skies. Dropping pressure indicates an incoming storm front.");

    // VOC
    indoor_voc = lv_label_create(tab_indoor);
    lv_obj_set_style_text_color(indoor_voc, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
    lv_obj_set_style_text_font(indoor_voc, &font_20_icons, LV_PART_MAIN); 
    lv_obj_align(indoor_voc, LV_ALIGN_BOTTOM_RIGHT, -15, -30);
    lv_obj_add_flag(indoor_voc, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(indoor_voc, info_popup_cb, LV_EVENT_CLICKED, (void*)"Green: 0.0 - 1.0 ppm\nOrange: 1.0 - 2.5 ppm\nRed: > 2.5 ppm\n\nVolatile Organic Compounds from breath, cooking, and cleaners. If Orange or Red, open a window!"); 

    // ==========================================
    // --- TEMP GAUGE (10C to 40C) ---
    // ==========================================
    lv_obj_t * track_temp = create_bg_track(tab_indoor, indoor_temp, LV_ALIGN_OUT_BOTTOM_LEFT);
    add_segment(track_temp, 0xFF0000, 0,  26);  // Red (10-18)
    add_segment(track_temp, 0xFFaa00, 26, 7);   // Orange (18-20)
    add_segment(track_temp, 0x00FF00, 33, 13);  // Green (20-24)
    add_segment(track_temp, 0xFFaa00, 46, 10);  // Orange (24-27)
    add_segment(track_temp, 0xFF0000, 56, 44);  // Red (27-40)
    slider_temp = create_indicator_slider(tab_indoor, track_temp, 10, 40);

    // ==========================================
    // --- HUMIDITY GAUGE (0% to 100%) ---
    // ==========================================
    lv_obj_t * track_hum = create_bg_track(tab_indoor, indoor_hum, LV_ALIGN_OUT_BOTTOM_LEFT);
    add_segment(track_hum, 0xFF0000, 0,  20); // Red (0-20)
    add_segment(track_hum, 0xFFaa00, 20, 10); // Orange (20-30)
    add_segment(track_hum, 0x00FF00, 30, 30); // Green (30-60)
    add_segment(track_hum, 0xFFaa00, 60, 10); // Orange (60-70)
    add_segment(track_hum, 0xFF0000, 70, 30); // Red (70-100)
    slider_hum = create_indicator_slider(tab_indoor, track_hum, 0, 100);

    // ==========================================
    // --- VOC GAUGE (0.00 to 5.00 ppm) ---
    // ==========================================
    // Note: Sliders only accept whole numbers, so we scale it 0 to 500!
    lv_obj_t * track_voc = create_bg_track(tab_indoor, indoor_voc, LV_ALIGN_OUT_BOTTOM_RIGHT);
    add_segment(track_voc, 0x00FF00, 0,  20); // Green (0-1.0)
    add_segment(track_voc, 0xFFaa00, 20, 30); // Orange (1.0-2.5)
    add_segment(track_voc, 0xFF0000, 50, 50); // Red (2.5-5.0)
    slider_voc = create_indicator_slider(tab_indoor, track_voc, 0, 500); 

    // ==========================================
    // --- PRESSURE GAUGE (980 to 1030 hPa) ---
    // ==========================================
    lv_obj_t * track_pres = create_bg_track(tab_indoor, indoor_pres, LV_ALIGN_OUT_BOTTOM_RIGHT);
    add_segment(track_pres, 0x00BFFF, 0, 100); // Solid Deep Sky Blue
    slider_pres = create_indicator_slider(tab_indoor, track_pres, 980, 1030);

    // ==========================================
    //            PAGE 2: WEATHER
    // ==========================================
    
    outdoor_icon = lv_label_create(tab_outdoor);
    lv_obj_set_style_text_align(outdoor_icon, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN); 
    lv_obj_set_style_text_color(outdoor_icon, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
    lv_obj_set_style_text_font(outdoor_icon, &custom_icon_font_60, LV_PART_MAIN); 
    lv_obj_align(outdoor_icon, LV_ALIGN_CENTER, 0, -65);

    outdoor_desc = lv_label_create(tab_outdoor);
    lv_obj_set_width(outdoor_desc, 200); // Lock the bounding box width
    lv_obj_set_style_text_align(outdoor_desc, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN); // Center the text inside the box
    lv_obj_set_style_text_color(outdoor_desc, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
    lv_obj_set_style_text_font(outdoor_desc, &font_24_icons, LV_PART_MAIN); 
    lv_obj_align_to(outdoor_desc, outdoor_icon, LV_ALIGN_OUT_BOTTOM_MID, 0, 5); 

    outdoor_temp = lv_label_create(tab_outdoor);
    lv_obj_set_style_text_color(outdoor_temp, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
    lv_obj_set_style_text_font(outdoor_temp, &font_20_icons, LV_PART_MAIN); 
    lv_obj_align(outdoor_temp, LV_ALIGN_CENTER, -135, 25);
    lv_obj_add_flag(outdoor_temp, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(outdoor_temp, info_popup_cb, LV_EVENT_CLICKED, (void*)"Deep Blue: < 0 C\nLight Blue: 0 - 10 C\nGreen: 10 - 20 C\nOrange: 20 - 26 C\nRed: > 26 C\n\nNote: High humidity and high wind speeds can make the actual outdoor temperature feel significantly different.");

    outdoor_hum = lv_label_create(tab_outdoor);
    lv_obj_set_style_text_color(outdoor_hum, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
    lv_obj_set_style_text_font(outdoor_hum, &font_20_icons, LV_PART_MAIN); 
    lv_obj_align(outdoor_hum, LV_ALIGN_CENTER, 0, 60);
    lv_obj_add_flag(outdoor_hum, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(outdoor_hum, info_popup_cb, LV_EVENT_CLICKED, (void*)"Orange: < 30% (Very Dry)\nGreen: 30 - 70% (Comfortable)\nBlue: > 70% (Humid/Rainy)\n\nHigh outdoor humidity traps heat in the summer and makes the cold feel bone-chilling in the winter.");

    outdoor_wind = lv_label_create(tab_outdoor);
    lv_obj_set_style_text_color(outdoor_wind, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
    lv_obj_set_style_text_font(outdoor_wind, &font_20_icons, LV_PART_MAIN); 
    lv_obj_align(outdoor_wind, LV_ALIGN_CENTER, 135, 25);
    lv_obj_add_flag(outdoor_wind, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(outdoor_wind, info_popup_cb, LV_EVENT_CLICKED, (void*)"Green: 0 - 20 km/h (Breeze)\nYellow: 20 - 40 km/h (Moderate)\nOrange: 40 - 60 km/h (Strong)\nRed: > 60 km/h (Gale Force)\n\nSustained winds above 40 km/h can cause minor structural damage and severe wind chill.");

    // ==========================================
    // --- FLUID WEATHER BARS ---
    // ==========================================
    
    // OUTDOOR TEMP BAR
    out_bar_temp = lv_bar_create(tab_outdoor);
    lv_obj_set_size(out_bar_temp, 100, 8);
    lv_obj_align_to(out_bar_temp, outdoor_temp, LV_ALIGN_OUT_BOTTOM_MID, 0, 5);
    // Range: -30C to +40C. 
    // Note: 0 degrees is NOT empty! 0 is visually at 42% of the bar. 
    // The bar will drain to the left in winter, and fill to the right in summer.
    lv_bar_set_range(out_bar_temp, -30, 40); 

    // OUTDOOR HUMIDITY BAR
    out_bar_hum = lv_bar_create(tab_outdoor);
    lv_obj_set_size(out_bar_hum, 100, 8);
    lv_obj_align_to(out_bar_hum, outdoor_hum, LV_ALIGN_OUT_BOTTOM_MID, 0, 5);
    // Range: 0% to 100%. (This scales perfectly by default).
    lv_bar_set_range(out_bar_hum, 0, 100);

    // OUTDOOR WIND BAR
    out_bar_wind = lv_bar_create(tab_outdoor);
    lv_obj_set_size(out_bar_wind, 100, 8);
    lv_obj_align_to(out_bar_wind, outdoor_wind, LV_ALIGN_OUT_BOTTOM_MID, 0, 5);
    // Range: 0 to 60 km/h. 
    // A 15 km/h breeze fills it 25%. A 60 km/h gale fills it 100%. 
    // LVGL automatically caps anything higher than 60 at 100% visually.
    lv_bar_set_range(out_bar_wind, 0, 60);

    // ==========================================
    //            PAGE 3: FORECAST (Grid)
    // ==========================================
    
    for(int i = 0; i < 12; i++) {
        forecast_labels[i] = lv_label_create(tab_forecast);
        lv_obj_set_style_text_align(forecast_labels[i], LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);
        lv_obj_set_style_text_color(forecast_labels[i], lv_color_hex(0xFFFFFF), LV_PART_MAIN);
        lv_obj_set_style_text_font(forecast_labels[i], &font_16_icons, LV_PART_MAIN); 
        lv_obj_set_style_text_line_space(forecast_labels[i], 3, LV_PART_MAIN);
        
        int col = i % 6;
        int row = i / 6;
        int x_offset = -185 + (col * 74); 
        int y_offset = -65 + (row * 115);  
        
        lv_obj_align(forecast_labels[i], LV_ALIGN_CENTER, x_offset, y_offset);
    }
}

void update_ui(IndoorData* indoor, WeatherData* outdoor) {
    
    // --- 1. Update Indoor Data ---
    int current_iaq = (int)indoor->iaq_estimate;
    lv_label_set_text_fmt(indoor_center, "IAQ\n%d", current_iaq);
    lv_arc_set_value(iaq_arc, current_iaq);

    // Colour for the arc of IAQ index
    lv_color_t iaq_color = get_iaq_color(current_iaq);
    lv_obj_set_style_arc_color(iaq_arc, iaq_color, LV_PART_INDICATOR);

    // Get the dynamic colors
    lv_color_t t_color = get_temp_color(indoor->temperature);
    lv_color_t h_color = get_hum_color(indoor->humidity);
    lv_color_t v_color = get_voc_color(indoor->breath_voc);

    // Apply colors to the Labels
    lv_obj_set_style_text_color(indoor_temp, t_color, LV_PART_MAIN);
    lv_obj_set_style_text_color(indoor_hum, h_color, LV_PART_MAIN);
    lv_obj_set_style_text_color(indoor_voc, v_color, LV_PART_MAIN);

    // Apply color the knobs
    lv_obj_set_style_bg_color(slider_temp, t_color, LV_PART_KNOB);
    lv_obj_set_style_bg_color(slider_hum, h_color, LV_PART_KNOB);
    lv_obj_set_style_bg_color(slider_voc, v_color, LV_PART_KNOB);
    
    // Pressure is a static track, so we just lock the knob to Deep Sky Blue
    lv_obj_set_style_bg_color(slider_pres, lv_color_hex(0x00BFFF), LV_PART_KNOB);

    // Move the Balls
    lv_slider_set_value(slider_temp, indoor->temperature, LV_ANIM_ON);
    lv_slider_set_value(slider_hum, indoor->humidity, LV_ANIM_ON);
    lv_slider_set_value(slider_pres, indoor->pressure, LV_ANIM_ON);
    
    // For VOC, we multiply by 100 to map the decimals to our 0-500 scaled slider
    lv_slider_set_value(slider_voc, (int)(indoor->breath_voc * 100), LV_ANIM_ON);

    lv_label_set_text_fmt(indoor_temp, "\xef\x8b\x89 Temp: %d C", (int)indoor->temperature);
    lv_label_set_text_fmt(indoor_hum, "\xef\x81\x83 Hum: %d %%", (int)indoor->humidity);
    lv_label_set_text_fmt(indoor_pres, "Pres: %d hPa \xef\x98\xa4", (int)indoor->pressure);
    int voc_whole = (int)indoor->breath_voc;
    int voc_frac = (int)(indoor->breath_voc * 100) % 100;
    lv_label_set_text_fmt(indoor_voc, "VOC: %d.%02d ppm \xef\x9d\x9f", voc_whole, voc_frac);

    // --- 2. Update Weather Data ---
    lv_label_set_text(outdoor_icon, weather_icon(outdoor->weather_code).c_str()); 
    lv_label_set_text(outdoor_desc, short_weather(outdoor->weather_code).c_str()); 

    lv_label_set_text_fmt(outdoor_temp, "\xef\x8b\x89 Temp: %d C", (int)outdoor->outdoor_temp);
    lv_label_set_text_fmt(outdoor_hum, "\xef\x81\x83 Hum: %d %%", outdoor->outdoor_humidity);
    lv_label_set_text_fmt(outdoor_wind, "Wind: %d km/h \xef\x9c\xae", (int)outdoor->wind_speed);

    // 1. Get the current colors based on the value
    lv_color_t o_t_color = get_out_temp_color((int)outdoor->outdoor_temp);
    lv_color_t o_h_color = get_out_hum_color(outdoor->outdoor_humidity);
    lv_color_t o_w_color = get_out_wind_color((int)outdoor->wind_speed);

    // 2. Apply the color to the filled part of the bar
    lv_obj_set_style_bg_color(out_bar_temp, o_t_color, LV_PART_INDICATOR);
    lv_obj_set_style_bg_color(out_bar_hum, o_h_color, LV_PART_INDICATOR);
    lv_obj_set_style_bg_color(out_bar_wind, o_w_color, LV_PART_INDICATOR);

    // 3. Move the bars to the new value
    lv_bar_set_value(out_bar_temp, (int)outdoor->outdoor_temp, LV_ANIM_ON);
    lv_bar_set_value(out_bar_hum, outdoor->outdoor_humidity, LV_ANIM_ON);
    lv_bar_set_value(out_bar_wind, (int)outdoor->wind_speed, LV_ANIM_ON);

    // --- 3. Build the 12-Hour Forecast Grid ---
    for (int i = 0; i < 12; i++) {
        lv_label_set_text_fmt(forecast_labels[i], "%s\n%s\n%s\n%dC", 
                 outdoor->hourly_times[i].c_str(), 
                 weather_icon(outdoor->hourly_codes[i]).c_str(),     
                 short_weather(outdoor->hourly_codes[i]).c_str(),    
                 (int)outdoor->hourly_temps[i]);
    }
}