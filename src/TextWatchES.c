#include "pebble.h"
#include "time.h"

#include "num2words-es.h"

#define DEBUG 0
#define BUFFER_SIZE 44

#define DEFAULT_BACKGROUND_COLOR GColorBlack
#define DEFAULT_DATE_DISPLAY 0

static Window* window;

static GFont font_thin;
static GFont font_reduced;
static int currentFont;

static char date_text[] = "aa.dd/mm ";
static char temp_date[] = "aa.dd/mm ";

// enumeracion para recibir AppMessages
enum {
    KEY_CONFIG_COLOR,
    KEY_CONFIG_SHOWDATE,
        };

typedef struct {
	TextLayer* currentLayer;
	TextLayer* nextLayer;	
	PropertyAnimation* currentAnimation;
	PropertyAnimation* nextAnimation;
} Line;


Line line1;
Line line2;
Line line3;

// layer para desplegar fecha
TextLayer* fechaMes;

GColor backgroundColor;
GColor fontColor;
int dateDisplay;

GColor appColor;

struct tm* t;

static char line1Str[2][BUFFER_SIZE];
static char line2Str[2][BUFFER_SIZE];
static char line3Str[2][BUFFER_SIZE];

static bool textInitialized = false;

// Animation handler
void animationStoppedHandler(struct Animation *animation, bool finished, void *context)
{
	TextLayer *current = (TextLayer *)context;
	GRect rect = layer_get_frame(text_layer_get_layer(current));
	rect.origin.x = 144;
	layer_set_frame(text_layer_get_layer(current), rect);
}

// Animate line
void makeAnimationsForLayers(Line *line, TextLayer *current, TextLayer *next)
{
  //APP_LOG(APP_LOG_LEVEL_DEBUG, "Current: %s  Next:%s", text_layer_get_text(current),text_layer_get_text(next));
	
  GRect rect = layer_get_frame(text_layer_get_layer(next));
	rect.origin.x -= 144;
	
	line->nextAnimation = property_animation_create_layer_frame(text_layer_get_layer(next), NULL, &rect);
	animation_set_duration(&line->nextAnimation->animation, 400);
	animation_set_curve(&line->nextAnimation->animation, AnimationCurveEaseOut);
	animation_schedule(&line->nextAnimation->animation);
	
	GRect rect2 = layer_get_frame(text_layer_get_layer(current));
	rect2.origin.x -= 144;
	
	line->currentAnimation = property_animation_create_layer_frame(text_layer_get_layer(current), NULL, &rect2);
	animation_set_duration(&line->currentAnimation->animation, 400);
	animation_set_curve(&line->currentAnimation->animation, AnimationCurveEaseOut);
	
	animation_set_handlers(&line->currentAnimation->animation, (AnimationHandlers) {
		.stopped = (AnimationStoppedHandler)animationStoppedHandler
	}, current);
	
	animation_schedule(&(line->currentAnimation->animation));
}

// Update line
void updateLineTo(Line *line, char lineStr[2][BUFFER_SIZE], char *value)
{
	TextLayer *next, *current;
	
	GRect rect = layer_get_frame(text_layer_get_layer(line->currentLayer));
	current = (rect.origin.x == 0) ? line->currentLayer : line->nextLayer;
	next = (current == line->currentLayer) ? line->nextLayer : line->currentLayer;
	
	// Update correct text only
	if (current == line->currentLayer) {
		memset(lineStr[1], 0, BUFFER_SIZE);
		memcpy(lineStr[1], value, strlen(value));
		text_layer_set_text(next, lineStr[1]);
	} else {
		memset(lineStr[0], 0, BUFFER_SIZE);
		memcpy(lineStr[0], value, strlen(value));
		text_layer_set_text(next, lineStr[0]);
	}
	
	makeAnimationsForLayers(line, current, next);
}

// Check to see if the current line needs to be updated
bool needToUpdateLine(Line *line, char lineStr[2][BUFFER_SIZE], char *nextValue)
{
	char *currentStr;
	GRect rect = layer_get_frame(text_layer_get_layer(line->currentLayer));
	currentStr = (rect.origin.x == 0) ? lineStr[0] : lineStr[1];

	if (memcmp(currentStr, nextValue, strlen(nextValue)) != 0 ||
		(strlen(nextValue) == 0 && strlen(currentStr) != 0)) {
		return true;
	}
	return false;
}

void dateToText(struct tm *t)
{
	char temp1[] = "1234567890123456789";
	char temp2[] = "dd/mm";
	char weekday[] = "aa";

	// dia de semana
	strftime(temp1, sizeof(temp1), "%a", t);
	temp1[0] += 32;
	temp1[2] = 0;

	if(strcmp(temp1,"mo") == 0)
			strcpy(weekday,"lu.");
	else if(strcmp(temp1, "tu") == 0)
			strcpy(weekday,"ma.");
	else if(strcmp(temp1, "we") == 0)
			strcpy(weekday,"mi.");
	else if(strcmp(temp1, "th") == 0)
			strcpy(weekday,"ju.");
	else if(strcmp(temp1, "fr") == 0)
			strcpy(weekday,"vi.");
	else if(strcmp(temp1, "sa") == 0)
			strcpy(weekday,"sa.");
	else if(strcmp(temp1, "su") == 0)
			strcpy(weekday,"do.");

		
	// fecha y mes
	strftime(temp2, sizeof(temp2), "%d.%m", t);
	
  //APP_LOG(APP_LOG_LEVEL_DEBUG, "Temp: %s", temp2);

	/*if (temp2[0] == '0') {
		memmove(&temp2[0], &temp2[1], 4);
		temp2[4] = 0;
	}
  */
  
	// finalmente armar string
	memmove(&temp_date[0],&weekday[0],3);
	memmove(&temp_date[3],&temp2[0],strlen(temp2));

	/*/ validar si se removio un cero en dia
	if(strlen(temp2) == 4){
		temp_date[7] = ' ';
		temp_date[8] = 0;
	}
	else
		temp_date[8] = ' ';
	*/

}

// Update screen based on new time
void display_time(struct tm *t)
{
	// The current time text will be stored in the following 3 strings
	char textLine1[BUFFER_SIZE];
	char textLine2[BUFFER_SIZE];
	char textLine3[BUFFER_SIZE];
	
	
	
	int changeFont = time_to_3words(t->tm_hour, t->tm_min, textLine1, textLine2, textLine3, BUFFER_SIZE);
	
	// verificar si se debe reducir el font
	if(changeFont != currentFont)
	{
		if(changeFont == 1)
		{
			text_layer_set_font(line2.currentLayer, font_reduced);
			text_layer_set_font(line2.nextLayer, font_reduced);
		}
		else
		{
			text_layer_set_font(line2.currentLayer, font_thin);
				text_layer_set_font(line2.nextLayer, font_thin);
		}

		currentFont = changeFont;
	}
		
	if (needToUpdateLine(&line1, line1Str, textLine1)) {
		updateLineTo(&line1, line1Str, textLine1);	
	}
	if (needToUpdateLine(&line2, line2Str, textLine2)) {
		updateLineTo(&line2, line2Str, textLine2);	
	}
	if (needToUpdateLine(&line3, line3Str, textLine3)) {
		updateLineTo(&line3, line3Str, textLine3);	
	}

  /*
	// generar texto de fecha
	dateToText(t);
		
	if(strcmp(date_text,temp_date) != 0) {
		strcpy(date_text,temp_date);
		text_layer_set_text(fechaMes,date_text);
		APP_LOG(APP_LOG_LEVEL_DEBUG, "Fecha Cambio: %s", date_text);
	}
	else
		APP_LOG(APP_LOG_LEVEL_DEBUG, "Misma Fecha: %s", date_text);
	*/
}

// Update screen without animation first time we start the watchface
void display_initial_time(struct tm *t)
{
	time_to_3words(t->tm_hour, t->tm_min, line1Str[0], line2Str[0], line3Str[0], BUFFER_SIZE);
	
	text_layer_set_text(line1.currentLayer, line1Str[0]);
	text_layer_set_text(line2.currentLayer, line2Str[0]);
	text_layer_set_text(line3.currentLayer, line3Str[0]);
}


// Configure the first line of text
void configureBoldLayer(TextLayer *textlayer, GColor txtColor)
{
	text_layer_set_font(textlayer, fonts_get_system_font(FONT_KEY_BITHAM_42_BOLD));
	text_layer_set_text_color(textlayer, txtColor);
	text_layer_set_background_color(textlayer, GColorClear);
	text_layer_set_text_alignment(textlayer, GTextAlignmentLeft);
}

// Configure for the 2nd and 3rd lines
void configureLightLayer(TextLayer *textlayer, GFont txtFont, GColor txtColor)
{
	text_layer_set_font(textlayer, txtFont);
	text_layer_set_text_color(textlayer, txtColor);
	text_layer_set_background_color(textlayer, GColorClear);
	text_layer_set_text_alignment(textlayer, GTextAlignmentLeft);
}


/** 
 * Debug methods. For quickly debugging enable debug macro on top to transform the watchface into
 * a standard app and you will be able to change the time with the up and down buttons
 */ 
#if DEBUG

void up_single_click_handler(ClickRecognizerRef recognizer, Window *window) {
	(void)recognizer;
	(void)window;
	
	t.tm_min += 1;
	if (t.tm_min >= 60) {
		t.tm_min = 0;
		t.tm_hour += 1;
		
		if (t.tm_hour >= 24) {
			t.tm_hour = 0;
		}
	}
	display_time(&t);
}


void down_single_click_handler(ClickRecognizerRef recognizer, Window *window) {
	(void)recognizer;
	(void)window;
	
	t.tm_min -= 1;
	if (t.tm_min < 0) {
		t.tm_min = 59;
		t.tm_hour -= 1;
	}
	display_time(&t);
}

void click_config_provider(ClickConfig **config, Window *window) {
  (void)window;

  config[BUTTON_ID_UP]->click.handler = (ClickHandler) up_single_click_handler;
  config[BUTTON_ID_UP]->click.repeat_interval_ms = 100;

  config[BUTTON_ID_DOWN]->click.handler = (ClickHandler) down_single_click_handler;
  config[BUTTON_ID_DOWN]->click.repeat_interval_ms = 100;
}

#endif


void setWatchFaceColor(GColor newColor) {

	if(newColor == backgroundColor)
		return;
	else
		backgroundColor = newColor;

	if(backgroundColor == GColorBlack)
		fontColor = GColorWhite;
	else
		fontColor = GColorBlack;


	window_set_background_color(window, backgroundColor);

	// 1st line layers
	text_layer_set_text_color(line1.currentLayer,fontColor);
	text_layer_set_text_color(line1.nextLayer,fontColor);

	// 2nd layers
	text_layer_set_text_color(line2.currentLayer, fontColor);
	text_layer_set_text_color(line2.nextLayer,fontColor);

	// 3rd layers
	text_layer_set_text_color(line3.currentLayer, fontColor);
	text_layer_set_text_color(line3.nextLayer, fontColor);

	// layer para fecha 
	text_layer_set_text_color(fechaMes, fontColor);

	// almacenar
	persist_write_int(KEY_CONFIG_COLOR, backgroundColor);
	
}

void setShowDate(int showDate) {

	if(showDate == dateDisplay)
		return;
	else
		dateDisplay = showDate;

	if(dateDisplay > 0)
		layer_add_child(window_get_root_layer(window), text_layer_get_layer(fechaMes));
	else
		layer_remove_from_parent(text_layer_get_layer(fechaMes));

	// almacenar
	persist_write_int(KEY_CONFIG_SHOWDATE,dateDisplay);
}

void out_sent_handler(DictionaryIterator *sent, void *context) {
   // outgoing message was delivered

 }


 void out_failed_handler(DictionaryIterator *failed, AppMessageResult reason, void *context) {
   // outgoing message failed

 }


 void in_received_handler(DictionaryIterator *received, void *context) {
   // incoming message received

 	// Check parametro de config de color
    Tuple *text_tuple = dict_find(received, KEY_CONFIG_COLOR);

    // Act on the found fields received
    if (text_tuple) {
        
        //APP_LOG(APP_LOG_LEVEL_DEBUG, "Config Value: %s", text_tuple->value->cstring);

        if(strcmp(text_tuple->value->cstring, "1") == 0)
        	setWatchFaceColor(GColorWhite);
        else
        	setWatchFaceColor(GColorBlack);
    }

    // Check parametro de config de color
    text_tuple = dict_find(received, KEY_CONFIG_SHOWDATE);

    // Act on the found fields received
    if (text_tuple) {
        
        //APP_LOG(APP_LOG_LEVEL_DEBUG, "Config Value: %s", text_tuple->value->cstring);

        if(strcmp(text_tuple->value->cstring, "1") == 0)
        	setShowDate(1);
        else
        	setShowDate(0);
    }


 }


 void in_dropped_handler(AppMessageResult reason, void *context) {
   // incoming message dropped

 }


// Time handler called every minute by the system
static void handle_minute_tick(struct tm *tick_time, TimeUnits units_changed) {

  display_time(tick_time);
}



void init(void) {

	// levantar valores de configuracion del storage local
	if (persist_exists(KEY_CONFIG_COLOR)) {
    	backgroundColor = persist_read_int(KEY_CONFIG_COLOR);
	}
	else {
		backgroundColor = DEFAULT_BACKGROUND_COLOR;
		persist_write_int(KEY_CONFIG_COLOR, backgroundColor);
	}

	if (persist_exists(KEY_CONFIG_SHOWDATE)) {
    	dateDisplay = persist_read_int(KEY_CONFIG_SHOWDATE);
	}
	else {
		dateDisplay = DEFAULT_DATE_DISPLAY;
		persist_write_int(KEY_CONFIG_SHOWDATE,dateDisplay);
	}

	app_message_register_inbox_received(in_received_handler);
   	app_message_register_inbox_dropped(in_dropped_handler);
   	app_message_register_outbox_sent(out_sent_handler);
   	app_message_register_outbox_failed(out_failed_handler);

 	// create tick handler
 	tick_timer_service_subscribe(MINUTE_UNIT, handle_minute_tick);

	window = window_create();
	window_stack_push(window, true);

	window_set_background_color(window, backgroundColor);

	if(backgroundColor == GColorBlack)
		fontColor = GColorWhite;
	else
		fontColor = GColorBlack;

	currentFont = 0;

	
	// Load font
	font_thin = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_THIN_41));
	font_reduced = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_THIN_38));
	
	
	// 1st line layers
	line1.currentLayer = text_layer_create(GRect(0, 18, 144, 50));
	line1.nextLayer = text_layer_create(GRect(144, 18, 144, 50));
	configureBoldLayer(line1.currentLayer,fontColor);
	configureBoldLayer(line1.nextLayer,fontColor);

	// 2nd layers
	line2.currentLayer = text_layer_create(GRect(0, 55, 144, 50));
	line2.nextLayer = text_layer_create(GRect(144, 55, 144, 50));
	configureLightLayer(line2.currentLayer,font_thin, fontColor);
	configureLightLayer(line2.nextLayer, font_thin, fontColor);

	// 3rd layers
	line3.currentLayer = text_layer_create(GRect(0, 92, 144, 50));
	line3.nextLayer = text_layer_create(GRect(144, 92, 144, 50));
	configureLightLayer(line3.currentLayer,font_thin, fontColor);
	configureLightLayer(line3.nextLayer,font_thin, fontColor);

	// layer para fecha 
	fechaMes = text_layer_create(GRect(0,139,144,50));
	text_layer_set_font(fechaMes, fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_GORDITA_26)));
	text_layer_set_text_color(fechaMes, fontColor);
	text_layer_set_background_color(fechaMes, GColorClear);
	text_layer_set_text_alignment(fechaMes, GTextAlignmentRight);


	// Configure time on init
	time_t hora =  time(NULL);
	t = localtime(&hora);
	display_initial_time(t);
	
	// Load layers
  	layer_add_child(window_get_root_layer(window), text_layer_get_layer(line1.currentLayer));
	layer_add_child(window_get_root_layer(window), text_layer_get_layer(line1.nextLayer));
	layer_add_child(window_get_root_layer(window), text_layer_get_layer(line2.currentLayer));
	layer_add_child(window_get_root_layer(window), text_layer_get_layer(line2.nextLayer));
	layer_add_child(window_get_root_layer(window), text_layer_get_layer(line3.currentLayer));
	layer_add_child(window_get_root_layer(window), text_layer_get_layer(line3.nextLayer));

	if(dateDisplay > 0){
		layer_add_child(window_get_root_layer(window), text_layer_get_layer(fechaMes));
		
	}
	

	const uint32_t inbound_size = 64;
   	const uint32_t outbound_size = 64;
   	app_message_open(inbound_size, outbound_size);
	
#if DEBUG
	// Button functionality
	window_set_click_config_provider(&window, (ClickConfigProvider) click_config_provider);
#endif

}


static void deinit(void){
    
    text_layer_destroy(line1.currentLayer);
	text_layer_destroy(line2.currentLayer);
	text_layer_destroy(line3.currentLayer);

	text_layer_destroy(line1.nextLayer);
	text_layer_destroy(line2.nextLayer);
	text_layer_destroy(line3.nextLayer);
	text_layer_destroy(fechaMes);


    fonts_unload_custom_font(font_thin);
    fonts_unload_custom_font(font_reduced);

    window_destroy(window);
}

// main entrance for program
int main(void) {

	init();

	//APP_LOG(APP_LOG_LEVEL_DEBUG, "Done initializing, pushed window: %p", window);

	app_event_loop();

	deinit();
  
}
