/*
 * main.c
 *
 * Created: 05/03/2019 18:00:58
 *  Author: eduardo
 */ 

#include <asf.h>
#include "tfont.h"
#include "sourcecodepro_28.h"
#include "calibri_36.h"
#include "arial_72.h"




#define BUT_PIO_VR			PIOA
#define BUT_PIO_ID_VR		19
#define BUT_PIO_IDX_VR		2u
#define BUT_PIO_IDX_MASK_VR (1u << BUT_PIO_IDX_VR)

struct ili9488_opt_t g_ili9488_display_opt;

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

void init(void){
	
	pmc_enable_periph_clk(BUT_PIO_ID_VR);

	pio_configure(BUT_PIO_VR, PIO_INPUT, BUT_PIO_IDX_MASK_VR, PIO_PULLUP);
	
	pio_handler_set(BUT_PIO_VR,
	BUT_PIO_ID_VR,
	BUT_PIO_IDX_MASK_VR,
	PIO_IT_FALL_EDGE,
	but_callback);
	
	// Ativa interrupção
	pio_enable_interrupt(BUT_PIO_VR, BUT_PIO_IDX_MASK_VR);

	// Configura NVIC para receber interrupcoes do PIO do botao
	// com prioridade 4 (quanto mais próximo de 0 maior)
	NVIC_EnableIRQ(BUT_PIO_ID_VR);
	NVIC_SetPriority(BUT_PIO_ID_VR, 0); // Prioridade 4

	
}


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

//float velocidade(int pulsos, float tempo){
//	return angular*0,65;
//}

//float distancia(int pulsos){
//	return 2*PI*angular*tempo;
//}


int main(void) {
	board_init();
	sysclk_init();	
	configure_lcd();
	
	char buffer[32];
	int x = 12;
	sprintf(buffer, "pessoal!! %d", x);
	
	font_draw_text(&sourcecodepro_28, "oimundo", 50, 50, 1);
	font_draw_text(&calibri_36, buffer, 50, 100, 1);
	//font_draw_text(&arial_72, sprintf(buffer, "%d", x), 50, 200, 2);
	while(1) {
		
	}
}