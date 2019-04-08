/*
 * main.c
 *
 * Created: 05/03/2019 18:00:58
 *  Author: eduardo
 */ 

/*
 
	Includes
 
 */ 

#include <asf.h>
#include "tfont.h"
#include "sourcecodepro_28.h"
#include "calibri_36.h"
#include "arial_72.h"

/*
 
	Defines
 
 */ 

#define PI 3.14

#define BUT_PIO_VR			PIOA
#define BUT_PIO_ID_VR		10
#define BUT_PIO_IDX_VR		19u
#define BUT_PIO_IDX_MASK_VR (1u << BUT_PIO_IDX_VR)

/*
 
	variaveis globais
 
 */ 


volatile int x = -1;

volatile Bool f_rtt_alarme = false;

/************************************************************************/
/* prototypes                                                           */
/************************************************************************/
void pin_toggle(Pio *pio, uint32_t mask);
void io_init(void);
static void RTT_init(uint16_t pllPreScale, uint32_t IrqNPulses);


struct ili9488_opt_t g_ili9488_display_opt;

/*
 
	handlers
 
 */ 


void but_callback(void){
	 x += 1;
	 delay_ms(30);
}



/************************************************************************/
/* inits                                                                */
/************************************************************************/

void configure_lcd(void){
	/* Initialize display parameter */
	g_ili9488_display_opt.ul_width = ILI9488_LCD_WIDTH;
	g_ili9488_display_opt.ul_height = ILI9488_LCD_HEIGHT;
	g_ili9488_display_opt.foreground_color = COLOR_CONVERT(COLOR_WHITE);
	g_ili9488_display_opt.background_color = COLOR_CONVERT(COLOR_WHITE);

	/* Initialize LCD */
	ili9488_init(&g_ili9488_display_opt);
	ili9488_draw_filled_rectangle(0, 0, ILI9488_LCD_WIDTH-1, ILI9488_LCD_HEIGHT-1);
	
}


void init_but(void){
	
	pmc_enable_periph_clk(BUT_PIO_ID_VR);

	pio_configure(BUT_PIO_VR, PIO_INPUT, BUT_PIO_IDX_MASK_VR, PIO_PULLUP);
	
	pio_handler_set(BUT_PIO_VR,
	BUT_PIO_ID_VR,
	BUT_PIO_IDX_MASK_VR,
	PIO_IT_FALL_EDGE,
	but_callback);
	
	// Ativa interrupção
	pio_enable_interrupt(BUT_PIO_VR, BUT_PIO_IDX_MASK_VR);

	NVIC_EnableIRQ(BUT_PIO_ID_VR);
	NVIC_SetPriority(BUT_PIO_ID_VR, 0); // Prioridade 4

	
}


/*
 
	funcoes
 
 */ 

void font_draw_text(tFont *font, const char *text, int x, int y, int spacing) {
	char *p = text;
	while(*p != NULL) {
		char letter = *p;
		int letter_offset = letter - font->start_char;
		if(letter <= font->end_char) {
			tChar *current_char = font->chars + letter_offset;
			ili9488_draw_pixmap(x, y, current_char->image->width, current_char->image->height, current_char->image->data);
			x += current_char->image->width + spacing;
		}
		p++;
	}	
}

float velocidade(int pulsos){
	int tempo = 4;
	return pulsos*0.65*PI/tempo;
}

float distancia(int pulsos){
	return 2*PI*0.325*pulsos;
}


int main(void) {
	board_init();
	sysclk_init();	
	configure_lcd();
	WDT->WDT_MR = WDT_MR_WDDIS;
	init_but();
	
	
	char buffer1[32];
	char buffer2[32];
	char buffer3[32];
	
	//font_draw_text(&sourcecodepro_28, "oimundo", 50, 50, 1);
	
	//font_draw_text(&arial_72, sprintf(buffer, "%d", x), 50, 200, 2);
	
	while(1) {
		sprintf(buffer1, "x: %d", x);
		font_draw_text(&calibri_36, buffer1, 50, 100, 1);
		
		sprintf(buffer2, "vel: %f", velocidade(x));
		font_draw_text(&calibri_36, buffer2, 50, 200, 1);
		
		sprintf(buffer3, "dist: %f", distancia(x));
		font_draw_text(&calibri_36, buffer3, 50, 300, 1);
	}
}