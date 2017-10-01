/*

Copyright (C) 2017 Mark Reed

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), 
to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, 
and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, 
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, 
WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

-------------------------------------------------------------------

*/

#include <pebble.h>
#include "main.h"
#include "effect_layer.h"

EffectLayer* effect_layer_inv;

static Window *window;
static Layer *window_layer;

BitmapLayer *layer_conn_img;
GBitmap *img_bt_connect;
GBitmap *img_bt_disconnect;

static GBitmap *time_format_image;
static BitmapLayer *time_format_layer;

static GBitmap* background;
static BitmapLayer *background_layer; 

BitmapLayer *layer_batt_img;
GBitmap *img_battery_100;
GBitmap *img_battery_90;
GBitmap *img_battery_80;
GBitmap *img_battery_70;
GBitmap *img_battery_60;
GBitmap *img_battery_50;
GBitmap *img_battery_40;
GBitmap *img_battery_30;
GBitmap *img_battery_20;
GBitmap *img_battery_10;
GBitmap *img_battery_charge;
TextLayer *layer_batt_text;
int charge_percent = 0;

static GBitmap *bt_text_image;
static BitmapLayer *bt_text_layer;

static GBitmap *day_name_image;
static BitmapLayer *day_name_layer;

const int DAY_NAME_IMAGE_RESOURCE_IDS[] = {
  RESOURCE_ID_IMAGE_DAY_NAME_SUN,
  RESOURCE_ID_IMAGE_DAY_NAME_MON,
  RESOURCE_ID_IMAGE_DAY_NAME_TUE,
  RESOURCE_ID_IMAGE_DAY_NAME_WED,
  RESOURCE_ID_IMAGE_DAY_NAME_THU,
  RESOURCE_ID_IMAGE_DAY_NAME_FRI,
  RESOURCE_ID_IMAGE_DAY_NAME_SAT
};

#define TOTAL_DATE_DIGITS 2	
static GBitmap *date_digits_images[TOTAL_DATE_DIGITS];
static BitmapLayer *date_digits_layers[TOTAL_DATE_DIGITS];

#define TOTAL_TIME_DIGITS 4
static GBitmap *time_digits_images[TOTAL_TIME_DIGITS];
static BitmapLayer *time_digits_layers[TOTAL_TIME_DIGITS];

const int BIG_DIGIT_IMAGE_RESOURCE_IDS[] = {
  RESOURCE_ID_IMAGE_NUM_0,
  RESOURCE_ID_IMAGE_NUM_1,
  RESOURCE_ID_IMAGE_NUM_2,
  RESOURCE_ID_IMAGE_NUM_3,
  RESOURCE_ID_IMAGE_NUM_4,
  RESOURCE_ID_IMAGE_NUM_5,
  RESOURCE_ID_IMAGE_NUM_6,
  RESOURCE_ID_IMAGE_NUM_7,
  RESOURCE_ID_IMAGE_NUM_8,
  RESOURCE_ID_IMAGE_NUM_9
};

#define TOTAL_SECONDS_DIGITS 2
static GBitmap *seconds_digits_images[TOTAL_SECONDS_DIGITS];
static BitmapLayer *seconds_digits_layers[TOTAL_SECONDS_DIGITS];


ClaySettings settings;

// Initialize the default settings
static void prv_default_settings() {	
  settings.secs = false;
  settings.invert = false;
  settings.bluetoothvibe = false;
  settings.hourlyvibe = false;
}

// Read settings from persistent storage
static void prv_load_settings() {
  // Load the default settings
  prv_default_settings();
  // Read settings from persistent storage, if they exist
  persist_read_data(SETTINGS_KEY, &settings, sizeof(settings));
}

// Save the settings to persistent storage
static void prv_save_settings() {
  persist_write_data(SETTINGS_KEY, &settings, sizeof(settings));
  // Update the display based on new settings
  prv_update_display();
}

static void handle_tick(struct tm *tick_time, TimeUnits units_changed);

// Update the display elements
static void prv_update_display() {
	

if (settings.secs) {
	
	tick_timer_service_subscribe(SECOND_UNIT, handle_tick);
	for (int i = 0; i < TOTAL_SECONDS_DIGITS; ++i) {
    layer_set_hidden(bitmap_layer_get_layer(seconds_digits_layers[i]), false);
	}	
	layer_set_hidden(bitmap_layer_get_layer(bt_text_layer), true); 
	
	} else {
	
	tick_timer_service_subscribe(MINUTE_UNIT, handle_tick);
	for (int i = 0; i < TOTAL_SECONDS_DIGITS; ++i) {
    layer_set_hidden(bitmap_layer_get_layer(seconds_digits_layers[i]), true);
	}	
	layer_set_hidden(bitmap_layer_get_layer(bt_text_layer), false); 
	}	
	
// invert
  if (settings.invert && effect_layer_inv == NULL) {
    // Add inverter layer
    Layer *window_layer = window_get_root_layer(window);

#ifdef PBL_PLATFORM_CHALK
	  effect_layer_inv = effect_layer_create(GRect(0, 0, 180, 180));  
#else
	  effect_layer_inv = effect_layer_create(GRect(0, 0, 144, 168));
#endif
    effect_layer_add_effect(effect_layer_inv, effect_invert_bw_only, NULL);
    layer_add_child(window_layer, effect_layer_get_layer(effect_layer_inv));
  
  } else if (!settings.invert && effect_layer_inv != NULL) {
    // Remove Inverter layer
   layer_remove_from_parent(effect_layer_get_layer(effect_layer_inv));
   effect_layer_destroy(effect_layer_inv);
   effect_layer_inv = NULL;
  }	

}


// Handle the response from AppMessage
static void prv_inbox_received_handler(DictionaryIterator *iter, void *context) {

// show seconds
  Tuple *secs_t = dict_find(iter, MESSAGE_KEY_secs);
  if (secs_t) {
    settings.secs = secs_t->value->int32 == 1;
  }
	
	// invert
  Tuple *inv_t = dict_find(iter, MESSAGE_KEY_invert);
  if (inv_t) {
    settings.invert = inv_t->value->int32 == 1;
  }
  // Bluetoothvibe
  Tuple *animations_t = dict_find(iter, MESSAGE_KEY_bluetoothvibe);
  if (animations_t) {
    settings.bluetoothvibe = animations_t->value->int32 == 1;
  }

  // hourlyvibe
  Tuple *hourlyvibe_t = dict_find(iter, MESSAGE_KEY_hourlyvibe);
  if (hourlyvibe_t) {
    settings.hourlyvibe = hourlyvibe_t->value->int32 == 1;
  }
	
// Save the new settings to persistent storage
  prv_save_settings();
}


static void set_container_image(GBitmap **bmp_image, BitmapLayer *bmp_layer, const int resource_id, GPoint origin) {
  GBitmap *old_image = *bmp_image;

  *bmp_image = gbitmap_create_with_resource(resource_id);
	
  GRect bounds = gbitmap_get_bounds(*bmp_image);

  GRect main_frame = GRect(origin.x, origin.y, bounds.size.w, bounds.size.h);
  bitmap_layer_set_bitmap(bmp_layer, *bmp_image);
  layer_set_frame(bitmap_layer_get_layer(bmp_layer), main_frame);

  if (old_image != NULL) {
  	gbitmap_destroy(old_image);
  }
}

void handle_battery(BatteryChargeState charge_state) {

    if (charge_state.is_charging) {
        bitmap_layer_set_bitmap(layer_batt_img, img_battery_charge);

    } else {
        if (charge_state.charge_percent <= 10) {
            bitmap_layer_set_bitmap(layer_batt_img, img_battery_10);
        } else if (charge_state.charge_percent <= 20) {
            bitmap_layer_set_bitmap(layer_batt_img, img_battery_20);
        } else if (charge_state.charge_percent <= 30) {
            bitmap_layer_set_bitmap(layer_batt_img, img_battery_30);
		} else if (charge_state.charge_percent <= 40) {
            bitmap_layer_set_bitmap(layer_batt_img, img_battery_40);
		} else if (charge_state.charge_percent <= 50) {
            bitmap_layer_set_bitmap(layer_batt_img, img_battery_50);
    	} else if (charge_state.charge_percent <= 60) {
            bitmap_layer_set_bitmap(layer_batt_img, img_battery_60);	
        } else if (charge_state.charge_percent <= 70) {
            bitmap_layer_set_bitmap(layer_batt_img, img_battery_70);
		} else if (charge_state.charge_percent <= 80) {
            bitmap_layer_set_bitmap(layer_batt_img, img_battery_80);
		} else if (charge_state.charge_percent <= 90) {
            bitmap_layer_set_bitmap(layer_batt_img, img_battery_90);
		} else if (charge_state.charge_percent <= 100) {
            bitmap_layer_set_bitmap(layer_batt_img, img_battery_100);			
			
        if (charge_state.charge_percent < charge_percent) {
            if (charge_state.charge_percent==20){
                vibes_double_pulse();
            } else if(charge_state.charge_percent==10){
                vibes_long_pulse();
            }
        }
    }
    charge_percent = charge_state.charge_percent;
    
  }
}

void handle_bluetooth(bool connected) {
    if (connected) {
        bitmap_layer_set_bitmap(layer_conn_img, img_bt_connect);
    } else {
        bitmap_layer_set_bitmap(layer_conn_img, img_bt_disconnect);
    }
	
	    if (settings.bluetoothvibe) {
        vibes_long_pulse();
	}
}

void force_update(void) {
    handle_battery(battery_state_service_peek());
    handle_bluetooth(bluetooth_connection_service_peek());
}

unsigned short get_display_hour(unsigned short hour) {
  if (clock_is_24h_style()) {
    return hour;
  }
  unsigned short display_hour = hour % 12;
  // Converts "0" to "12"
  return display_hour ? display_hour : 12;
}

static void update_days(struct tm *tick_time) {
#ifdef PBL_PLATFORM_CHALK
//  set_container_image(&day_name_image, day_name_layer, DAY_NAME_IMAGE_RESOURCE_IDS[tick_time->tm_wday], GPoint( 15, 13));
  set_container_image(&date_digits_images[0], date_digits_layers[0], BIG_DIGIT_IMAGE_RESOURCE_IDS[tick_time->tm_mday/10], GPoint(60, 13));
  set_container_image(&date_digits_images[1], date_digits_layers[1], BIG_DIGIT_IMAGE_RESOURCE_IDS[tick_time->tm_mday%10], GPoint(90, 13));
#else	
  set_container_image(&day_name_image, day_name_layer, DAY_NAME_IMAGE_RESOURCE_IDS[tick_time->tm_wday], GPoint( 5, 13));
  set_container_image(&date_digits_images[0], date_digits_layers[0], BIG_DIGIT_IMAGE_RESOURCE_IDS[tick_time->tm_mday/10], GPoint(88, 13));
  set_container_image(&date_digits_images[1], date_digits_layers[1], BIG_DIGIT_IMAGE_RESOURCE_IDS[tick_time->tm_mday%10], GPoint(117, 13));
#endif	
}

static void update_hours(struct tm *tick_time) {
  
   unsigned short display_hour = get_display_hour(tick_time->tm_hour);

  if(settings.hourlyvibe) {
    //vibe!
    vibes_short_pulse();
  }
#ifdef PBL_PLATFORM_CHALK	
  set_container_image(&time_digits_images[0], time_digits_layers[0], BIG_DIGIT_IMAGE_RESOURCE_IDS[display_hour/10], GPoint(23, 67));
  set_container_image(&time_digits_images[1], time_digits_layers[1], BIG_DIGIT_IMAGE_RESOURCE_IDS[display_hour%10], GPoint(53, 67));
#else	
  set_container_image(&time_digits_images[0], time_digits_layers[0], BIG_DIGIT_IMAGE_RESOURCE_IDS[display_hour/10], GPoint(24, 67));
  set_container_image(&time_digits_images[1], time_digits_layers[1], BIG_DIGIT_IMAGE_RESOURCE_IDS[display_hour%10], GPoint(54, 67));
#endif
	
if (!clock_is_24h_style()) {
	
	
	if (tick_time->tm_hour >= 12) {
#ifdef PBL_PLATFORM_CHALK
      set_container_image(&time_format_image, time_format_layer, RESOURCE_ID_IMAGE_PM_MODE, GPoint(16, 67));
      layer_set_hidden(bitmap_layer_get_layer(time_format_layer), false);
#else
      set_container_image(&time_format_image, time_format_layer, RESOURCE_ID_IMAGE_PM_MODE, GPoint(8, 67));
      layer_set_hidden(bitmap_layer_get_layer(time_format_layer), false);
#endif
	}  else {
#ifdef PBL_PLATFORM_CHALK
	set_container_image(&time_format_image, time_format_layer, RESOURCE_ID_IMAGE_AM_MODE, GPoint(16, 103));
	layer_set_hidden(bitmap_layer_get_layer(time_format_layer), false);
#else
	set_container_image(&time_format_image, time_format_layer, RESOURCE_ID_IMAGE_AM_MODE, GPoint(8, 103));
	layer_set_hidden(bitmap_layer_get_layer(time_format_layer), false);
#endif
	}	  
		  
  //  if (tick_time->tm_hour >= 12) {
   //   set_container_image(&time_format_image, time_format_layer, RESOURCE_ID_IMAGE_PM_MODE, GPoint(51, 31));
   //   layer_set_hidden(bitmap_layer_get_layer(time_format_layer), false);
  //  }  else {
   //   layer_set_hidden(bitmap_layer_get_layer(time_format_layer), true);
   // }
    
    if (display_hour/10 == 0) {
      layer_set_hidden(bitmap_layer_get_layer(time_digits_layers[0]), true);
	}   else {
      layer_set_hidden(bitmap_layer_get_layer(time_digits_layers[0]), false);
    }	
   }
}

static void update_minutes(struct tm *tick_time) {
#ifdef PBL_PLATFORM_CHALK
  set_container_image(&time_digits_images[2], time_digits_layers[2], BIG_DIGIT_IMAGE_RESOURCE_IDS[tick_time->tm_min/10], GPoint(88, 67));
  set_container_image(&time_digits_images[3], time_digits_layers[3], BIG_DIGIT_IMAGE_RESOURCE_IDS[tick_time->tm_min%10], GPoint(118, 67));
#else
  set_container_image(&time_digits_images[2], time_digits_layers[2], BIG_DIGIT_IMAGE_RESOURCE_IDS[tick_time->tm_min/10], GPoint(88, 67));
  set_container_image(&time_digits_images[3], time_digits_layers[3], BIG_DIGIT_IMAGE_RESOURCE_IDS[tick_time->tm_min%10], GPoint(117, 67));
#endif

}

static void update_seconds(struct tm *tick_time) {
#ifdef PBL_PLATFORM_CHALK
  set_container_image(&seconds_digits_images[0], seconds_digits_layers[0], BIG_DIGIT_IMAGE_RESOURCE_IDS[tick_time->tm_sec/10], GPoint(78, 121));
  set_container_image(&seconds_digits_images[1], seconds_digits_layers[1], BIG_DIGIT_IMAGE_RESOURCE_IDS[tick_time->tm_sec%10], GPoint(107, 121));
#else
  set_container_image(&seconds_digits_images[0], seconds_digits_layers[0], BIG_DIGIT_IMAGE_RESOURCE_IDS[tick_time->tm_sec/10], GPoint(88, 121));
  set_container_image(&seconds_digits_images[1], seconds_digits_layers[1], BIG_DIGIT_IMAGE_RESOURCE_IDS[tick_time->tm_sec%10], GPoint(117, 121));
#endif

}

static void handle_tick(struct tm *tick_time, TimeUnits units_changed) {

  if (units_changed & DAY_UNIT) {
    update_days(tick_time);
  }
  if (units_changed & HOUR_UNIT) {
    update_hours(tick_time);
  }
  if (units_changed & MINUTE_UNIT) {
    update_minutes(tick_time);
  }	
  if (units_changed & SECOND_UNIT) {
    update_seconds(tick_time);
  }

}

static void init(void) {
	
  prv_load_settings();

// Listen for AppMessages
  app_message_register_inbox_received(prv_inbox_received_handler);
  app_message_open(128, 128);

  memset(&time_digits_layers, 0, sizeof(time_digits_layers));
  memset(&time_digits_images, 0, sizeof(time_digits_images));
  memset(&seconds_digits_layers, 0, sizeof(seconds_digits_layers));
  memset(&seconds_digits_images, 0, sizeof(seconds_digits_images));
  memset(&date_digits_layers, 0, sizeof(date_digits_layers));
  memset(&date_digits_images, 0, sizeof(date_digits_images));

	
  window = window_create();
  if (window == NULL) {
      //APP_LOG(APP_LOG_LEVEL_DEBUG, "OOM: couldn't allocate window");
      return;
  }

  window_stack_push(window, true /* Animated */);
  window_layer = window_get_root_layer(window);
  window_set_background_color(window, GColorBlack);
  GRect bounds = layer_get_bounds(window_layer);

//background
  background = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BACKGROUND);
  background_layer = bitmap_layer_create(bounds);
  bitmap_layer_set_bitmap(background_layer, background);
  layer_add_child(window_layer, bitmap_layer_get_layer(background_layer));
	
	
// resources
	img_bt_connect     = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BLUETOOTHON);
    img_bt_disconnect  = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BLUETOOTHOFF);
	
    img_battery_100   = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BATT_090_100);
    img_battery_90   = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BATT_080_090);
    img_battery_80   = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BATT_070_080);
    img_battery_70   = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BATT_060_070);
    img_battery_60   = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BATT_050_060);
    img_battery_50   = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BATT_040_050);
    img_battery_40   = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BATT_030_040);
    img_battery_30    = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BATT_020_030);
    img_battery_20    = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BATT_010_020);
    img_battery_10    = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BATT_000_010);
    img_battery_charge = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BATT_CHARGING);

// status layers
	
#ifdef PBL_PLATFORM_CHALK
	layer_batt_img  = bitmap_layer_create(GRect(155, 40, 5, 91));
    bitmap_layer_set_bitmap(layer_batt_img, img_battery_100);
	layer_add_child(window_layer, bitmap_layer_get_layer(layer_batt_img));
#else
	layer_batt_img  = bitmap_layer_create(GRect(-1, 2, 144, 5));
    bitmap_layer_set_bitmap(layer_batt_img, img_battery_100);
	layer_add_child(window_layer, bitmap_layer_get_layer(layer_batt_img));	
#endif
	
	
#ifdef PBL_PLATFORM_CHALK
    layer_conn_img  = bitmap_layer_create(GRect(20, 121, 75, 46));
    bitmap_layer_set_bitmap(layer_conn_img, img_bt_connect);
	bitmap_layer_set_compositing_mode(layer_conn_img, GCompOpSet);
	layer_add_child(window_layer, bitmap_layer_get_layer(layer_conn_img)); 
#else
    layer_conn_img  = bitmap_layer_create(GRect(2, 121, 75, 46));
    bitmap_layer_set_bitmap(layer_conn_img, img_bt_connect);
	bitmap_layer_set_compositing_mode(layer_conn_img, GCompOpSet);
	layer_add_child(window_layer, bitmap_layer_get_layer(layer_conn_img)); 	
#endif
	
	
// Create time and date layers

   GRect dummy_frame = { {0, 0}, {0, 0} };
   day_name_layer = bitmap_layer_create(dummy_frame);
	bitmap_layer_set_compositing_mode(day_name_layer, GCompOpSet);
   layer_add_child(window_layer, bitmap_layer_get_layer(day_name_layer));		
 	
	for (int i = 0; i < TOTAL_SECONDS_DIGITS; ++i) {
    seconds_digits_layers[i] = bitmap_layer_create(dummy_frame);
  bitmap_layer_set_compositing_mode((seconds_digits_layers[i]), GCompOpSet);
    layer_add_child(window_layer, bitmap_layer_get_layer(seconds_digits_layers[i]));
  }
	
    for (int i = 0; i < TOTAL_TIME_DIGITS; ++i) {
    time_digits_layers[i] = bitmap_layer_create(dummy_frame);
  bitmap_layer_set_compositing_mode((time_digits_layers[i]), GCompOpSet);
    layer_add_child(window_layer, bitmap_layer_get_layer(time_digits_layers[i]));
  }
	
    for (int i = 0; i < TOTAL_DATE_DIGITS; ++i) {
    date_digits_layers[i] = bitmap_layer_create(dummy_frame);
  bitmap_layer_set_compositing_mode((date_digits_layers[i]), GCompOpSet);
    layer_add_child(window_layer, bitmap_layer_get_layer(date_digits_layers[i]));
  }

  GRect frame5 = GRect(0, 0, 8, 8);
  time_format_layer = bitmap_layer_create(frame5);
  layer_add_child(window_layer, bitmap_layer_get_layer(time_format_layer));
	
#ifdef PBL_PLATFORM_CHALK
  bt_text_image = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BT);
  GRect bitmap_bounds = gbitmap_get_bounds(bt_text_image);
  GRect frame = GRect(78, 121, bitmap_bounds.size.w, bitmap_bounds.size.h);
  bt_text_layer = bitmap_layer_create(frame);
  bitmap_layer_set_bitmap(bt_text_layer, bt_text_image);
  bitmap_layer_set_compositing_mode(bt_text_layer, GCompOpSet);
  layer_add_child(window_layer, bitmap_layer_get_layer(bt_text_layer));   
#else
  bt_text_image = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BT);
  GRect bitmap_bounds = gbitmap_get_bounds(bt_text_image);
  GRect frame = GRect(86, 121, bitmap_bounds.size.w, bitmap_bounds.size.h);
  bt_text_layer = bitmap_layer_create(frame);
  bitmap_layer_set_bitmap(bt_text_layer, bt_text_image);
  bitmap_layer_set_compositing_mode(bt_text_layer, GCompOpSet);
  layer_add_child(window_layer, bitmap_layer_get_layer(bt_text_layer));   	
#endif
	
	
  prv_update_display();

  // Avoids a blank screen on watch start.
  time_t now = time(NULL);
  struct tm *tick_time = localtime(&now);  
  handle_tick(tick_time, MONTH_UNIT + DAY_UNIT + HOUR_UNIT + MINUTE_UNIT + SECOND_UNIT);

  tick_timer_service_subscribe(SECOND_UNIT, handle_tick);	


	// handlers
    battery_state_service_subscribe(&handle_battery);
    bluetooth_connection_service_subscribe(&handle_bluetooth);

	
	// draw first frame
    force_update();

}

static void deinit(void) {
  
  tick_timer_service_unsubscribe();
  battery_state_service_unsubscribe();
  bluetooth_connection_service_unsubscribe();

  layer_remove_from_parent(bitmap_layer_get_layer(background_layer));
  bitmap_layer_destroy(background_layer);

	if (background != NULL) {
		gbitmap_destroy(background);
    }
  
  layer_remove_from_parent(bitmap_layer_get_layer(bt_text_layer));
  bitmap_layer_destroy(bt_text_layer);
  gbitmap_destroy(bt_text_image);
    
  layer_remove_from_parent(bitmap_layer_get_layer(layer_batt_img));
  bitmap_layer_destroy(layer_batt_img);
  gbitmap_destroy(img_battery_100);
  gbitmap_destroy(img_battery_90);
  gbitmap_destroy(img_battery_80);
  gbitmap_destroy(img_battery_70);
  gbitmap_destroy(img_battery_60);
  gbitmap_destroy(img_battery_50);
  gbitmap_destroy(img_battery_40);
  gbitmap_destroy(img_battery_30);
  gbitmap_destroy(img_battery_20);
  gbitmap_destroy(img_battery_10);
  gbitmap_destroy(img_battery_charge);	
	
  layer_remove_from_parent(bitmap_layer_get_layer(layer_conn_img));
  bitmap_layer_destroy(layer_conn_img);
  gbitmap_destroy(img_bt_connect);
  gbitmap_destroy(img_bt_disconnect);
	
  layer_remove_from_parent(bitmap_layer_get_layer(time_format_layer));
  bitmap_layer_destroy(time_format_layer);
  gbitmap_destroy(time_format_image);
  time_format_image = NULL;	
	
  layer_remove_from_parent(bitmap_layer_get_layer(day_name_layer));
  bitmap_layer_destroy(day_name_layer);
  gbitmap_destroy(day_name_image);
		
	for (int i = 0; i < TOTAL_DATE_DIGITS; i++) {
    layer_remove_from_parent(bitmap_layer_get_layer(date_digits_layers[i]));
    gbitmap_destroy(date_digits_images[i]);
    bitmap_layer_destroy(date_digits_layers[i]);
    }

	for (int i = 0; i < TOTAL_SECONDS_DIGITS; i++) {
    layer_remove_from_parent(bitmap_layer_get_layer(seconds_digits_layers[i]));
    gbitmap_destroy(seconds_digits_images[i]);
    bitmap_layer_destroy(seconds_digits_layers[i]);
    }
	
   for (int i = 0; i < TOTAL_TIME_DIGITS; i++) {
    layer_remove_from_parent(bitmap_layer_get_layer(time_digits_layers[i]));
    gbitmap_destroy(time_digits_images[i]);
    bitmap_layer_destroy(time_digits_layers[i]);
    } 

if ( effect_layer_inv != NULL) {
    // Remove Inverter layer
   layer_remove_from_parent(effect_layer_get_layer(effect_layer_inv));
   effect_layer_destroy(effect_layer_inv);
   effect_layer_inv = NULL;
}
	
	window_destroy(window);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}