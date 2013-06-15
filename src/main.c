#include "pebble_os.h"
#include "pebble_app.h"
#include "pebble_fonts.h"
#include "http.h"


// #define MY_UUID { 0x44, 0x3E, 0xE6, 0x97, 0x0B, 0x39, 0x45, 0xBD, 0xBB, 0x68, 0x10, 0x12, 0x1F, 0x4E, 0x4D, 0x26 }
// from http://kathar.in/httpebble/

#define MY_UUID { 0x91, 0x41, 0xB6, 0x28, 0xBC, 0x89, 0x49, 0x8E, 0xB1, 0x47, 0x04, 0x9F, 0x49, 0xC0, 0x99, 0xAD }
#define VERSION 6
PBL_APP_INFO_SIMPLE(MY_UUID, "Ernie App", "Ernie", VERSION /* App version */);

// POST (SENDING)
#define TESTES_KEY_WHICH 1

#define TESTES_VALUE_UP 1
#define TESTES_VALUE_DOWN 2
#define TESTES_VALUE_SELECT 3
#define TESTES_VALUE_PING 9

// RESPONSE (RECEIVING)
//
#define TESTES_KEY_CURRENT 1
#define TESTES_KEY_PRECIPITATION 3


#define TESTES_HTTP_COOKIE 1949327671
#define TESTES_URL "http://pebble.ernie.org/default"

Window window;

TextLayer textLayer;


void send_button(int which);
char *get_result_string(AppMessageResult result);
static int our_latitude, our_longitude;
static bool located;

// Modify these common button handlers

void up_single_click_handler(ClickRecognizerRef recognizer, Window *window) {
  (void)recognizer;
  (void)window;
  text_layer_set_text(&textLayer, "Up!");
  send_button(TESTES_VALUE_UP);
}


void down_single_click_handler(ClickRecognizerRef recognizer, Window *window) {
  (void)recognizer;
  (void)window;
  text_layer_set_text(&textLayer, "Down!");
  send_button(TESTES_VALUE_DOWN);
}


void select_single_click_handler(ClickRecognizerRef recognizer, Window *window) {
  (void)recognizer;
  (void)window;
  text_layer_set_text(&textLayer, "Select!");
  send_button(TESTES_VALUE_SELECT);
}


void select_long_click_handler(ClickRecognizerRef recognizer, Window *window) {
  (void)recognizer;
  (void)window;
  text_layer_set_text(&textLayer, "Long!");
}


// This usually won't need to be modified

void click_config_provider(ClickConfig **config, Window *window) {
  (void)window;

  config[BUTTON_ID_SELECT]->click.handler = (ClickHandler) select_single_click_handler;

  config[BUTTON_ID_SELECT]->long_click.handler = (ClickHandler) select_long_click_handler;

  config[BUTTON_ID_UP]->click.handler = (ClickHandler) up_single_click_handler;
  config[BUTTON_ID_UP]->click.repeat_interval_ms = 100;

  config[BUTTON_ID_DOWN]->click.handler = (ClickHandler) down_single_click_handler;
  config[BUTTON_ID_DOWN]->click.repeat_interval_ms = 100;
}





void failed(int32_t cookie, int http_status, void* context) {
	if(cookie == 0 || cookie == TESTES_HTTP_COOKIE) {
		 text_layer_set_text(&textLayer, "NO RESPONSE!");
	}
}

void success(int32_t cookie, int http_status, DictionaryIterator* received, void* context) {
	if(cookie != TESTES_HTTP_COOKIE) return;
	Tuple* data_tuple = dict_find(received, TESTES_KEY_CURRENT);
	if(data_tuple) {
		// The below bitwise dance is so we can actually fit our precipitation forecast.
		uint16_t value = data_tuple->value->int16;
		uint8_t icon = value >> 11;
		if(icon < 10) {
		 	text_layer_set_text(&textLayer, "GOT SOME ICON");
		} else {
		 	text_layer_set_text(&textLayer, "NO ICON!");
		}
		int16_t temp = value & 0x3ff;
		if(value & 0x400) temp = -temp;
		text_layer_set_text(&textLayer, "GOT TEMP");
	}
	Tuple* forecast_tuple = dict_find(received, TESTES_KEY_PRECIPITATION);
	if(forecast_tuple) {
		// It's going to rain!
		// memset(precip_forecast, 0, 60);
		// memcpy(precip_forecast, forecast_tuple->value->data, forecast_tuple->length > 60 ? 60 : forecast_tuple->length);
		// precip_forecast_index = 0;
		// weather_layer_set_precipitation_forecast(&weather_layer, precip_forecast, 60);
	} else {
		// weather_layer_clear_precipitation_forecast(&weather_layer);
	}
}

void reconnect(void* context) {
	text_layer_set_text(&textLayer, "RECONNECT");
}


void location(float latitude, float longitude, float altitude, float accuracy, void* context) {
	text_layer_set_text(&textLayer, "LOCATION");
	// Fix the floats
	 our_latitude = latitude * 10000;
	 our_longitude = longitude * 10000;
	 located = true;
	// request_weather();
	// set_timer((AppContextRef)context);
}

void send_button(int which) {
	// Build the HTTP request
	DictionaryIterator *body;
	HTTPResult result = http_out_get(TESTES_URL, TESTES_HTTP_COOKIE, &body);
	// HTTPResult result = http_out_get("http://pwdb.kathar.in/pebble/weather3.php", TESTES_HTTP_COOKIE, &body);
	if(result != HTTP_OK) {
		// text_layer_set_text(&textLayer, "NO SETUP HTTP_OK!");
		text_layer_set_text(&textLayer, get_result_string(result));
		return;
	}
	// dict_write_int32(body, TESTES_KEY_LATITUDE, our_latitude);
	dict_write_int32(body, TESTES_KEY_WHICH, which);
	// dict_write_cstring(body, TESTES_KEY_UNIT_SYSTEM, UNIT_SYSTEM);
	// Send it.
	if(http_out_send() != HTTP_OK) {
		text_layer_set_text(&textLayer, "NO SENT HTTP_OK!");
		return;
	}
}


// Standard app initialisation

void handle_init(AppContextRef ctx) {
  (void)ctx;
  resource_init_current_app(&APP_RESOURCES);
  window_init(&window, "Ernie Button App");
  window_stack_push(&window, true /* Animated */);

  text_layer_init(&textLayer, window.layer.frame);
  text_layer_set_text(&textLayer, "Ernie Hello World 6");
  text_layer_set_font(&textLayer, fonts_get_system_font(FONT_KEY_GOTHAM_30_BLACK));
  layer_add_child(&window.layer, &textLayer.layer);

  // Attach our desired button functionality
  window_set_click_config_provider(&window, (ClickConfigProvider) click_config_provider);
	
  http_register_callbacks((HTTPCallbacks){
		.failure=failed,
		.success=success,
		.reconnect=reconnect,
		.location=location
	}, (void*)ctx);

  send_button(TESTES_VALUE_PING);
}


void pbl_main(void *params) {
  PebbleAppHandlers handlers = {
    .init_handler = &handle_init,
    .messaging_info = {
      .buffer_sizes = {
        .inbound = 124,
        .outbound = 256,
      }
    }
  };
  app_event_loop(params, &handlers);
}


char *get_result_string(AppMessageResult result) {

  switch(result) {
    case APP_MSG_OK:
      return "RESULT IS APP_MSG_OK";
      break;

    case APP_MSG_SEND_TIMEOUT:
      return "RESULT IS APP_MSG_SEND_TIMEOUT";
      break;

    case APP_MSG_SEND_REJECTED:
      return "RESULT IS APP_MSG_SEND_REJECTED";
      break;

    case APP_MSG_NOT_CONNECTED:
      return "RESULT IS APP_MSG_NOT_CONNECTED";
      break;

    case APP_MSG_APP_NOT_RUNNING:
      return "RESULT IS APP_MSG_APP_NOT_RUNNING";
      break;

    case APP_MSG_INVALID_ARGS:
      return "RESULT IS APP_MSG_INVALID_ARGS";
      break;

    case APP_MSG_BUSY:
      return "RESULT IS APP_MSG_BUSY";
      break;

    case APP_MSG_BUFFER_OVERFLOW:
      return "RESULT IS APP_MSG_BUFFER_OVERFLOW";
      break;

    case APP_MSG_ALREADY_RELEASED:
      return "RESULT IS APP_MSG_ALREADY_RELEASED";
      break;

    case APP_MSG_CALLBACK_ALREADY_REGISTERED:
      return "RESULT IS APP_MSG_CALLBACK_ALREADY_REGISTERED";
      break;

    case APP_MSG_CALLBACK_NOT_REGISTERED:
      return "RESULT IS APP_MSG_CALLBACK_NOT_REGISTERED";
      break;
  }
  return "UNKNOWN";

}
