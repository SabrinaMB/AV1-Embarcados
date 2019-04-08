/*
 * main.c
 *
 * Created: 05/03/2019 18:00:58
 *  Author: eduardo
 */ 

/*
 
	Includes
 
 */ 


#include "asf.h"
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
volatile int x1 = -1;

volatile int segundos = 0;
volatile int minutos = 0;
volatile int horas = 0;

volatile Bool flag = false;

volatile Bool f_rtt_alarme = false;

/************************************************************************/
/* prototypes                                                           */
/************************************************************************/
void pin_toggle(Pio *pio, uint32_t mask);
static void RTT_init(uint16_t pllPreScale, uint32_t IrqNPulses);


struct ili9488_opt_t g_ili9488_display_opt;

/*
 
	handlers
 
 */ 


void but_callback(void){
	 x += 1;
	 x1 += 1;
	 delay_ms(30);
}


void RTT_Handler(void)
{
	uint32_t ul_status;

	/* Get RTT status */
	ul_status = rtt_get_status(RTT);

	/* IRQ due to Time has changed */
	if ((ul_status & RTT_SR_RTTINC) == RTT_SR_RTTINC) {  }

	/* IRQ due to Alarm */
	if ((ul_status & RTT_SR_ALMS) == RTT_SR_ALMS) {
		f_rtt_alarme = true;                  // flag RTT alarme
	}
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

float velocidade(int pulsos1){
	int tempo = 4;
	return pulsos1*0.65*PI/tempo;
}

float distancia(int pulsos){
	return 2*PI*0.325*pulsos;
}

void tempo(int t){
	if ((segundos % 60 == 0)&& (segundos > 0)){
		minutos += 1;
		if (minutos % 60 == 0){
			minutos = 0; 
			horas += 1;
		}
	}
	segundos = t - 60*minutos - 360*horas;
}



void pin_toggle(Pio *pio, uint32_t mask){
	if(pio_get_output_data_status(pio, mask))
	pio_clear(pio, mask);
	else
	pio_set(pio,mask);
}

static float get_time_rtt(){
	uint ul_previous_time = rtt_read_timer_value(RTT);
}

static void RTT_init(uint16_t pllPreScale, uint32_t IrqNPulses)
{
	uint32_t ul_previous_time;

	/* Configure RTT for a 1 second tick interrupt */
	rtt_sel_source(RTT, false);
	rtt_init(RTT, pllPreScale);
	
	ul_previous_time = rtt_read_timer_value(RTT);
	while (ul_previous_time == rtt_read_timer_value(RTT));
	
	rtt_write_alarm_time(RTT, IrqNPulses+ul_previous_time);

	/* Enable RTT interrupt */
	NVIC_DisableIRQ(RTT_IRQn);
	NVIC_ClearPendingIRQ(RTT_IRQn);
	NVIC_SetPriority(RTT_IRQn, 0);
	NVIC_EnableIRQ(RTT_IRQn);
	rtt_enable_interrupt(RTT, RTT_MR_ALMIEN);
}


int main(void) {
	board_init();
	
	configure_lcd();
	WDT->WDT_MR = WDT_MR_WDDIS;
	init_but();
	sysclk_init();	
	
	
	char buffer1[32];
	char buffer2[32];
	char buffer3[32];
	
	//font_draw_text(&sourcecodepro_28, "oimundo", 50, 50, 1);
	
	//font_draw_text(&arial_72, sprintf(buffer, "%d", x), 50, 200, 2);
	
	  uint16_t pllPreScale = (int) (((float) 32768));
      uint32_t irqRTTvalue = 8;
      
      // reinicia RTT para gerar um novo IRQ
      RTT_init(pllPreScale, irqRTTvalue);         
      
     /*
      * caso queira ler o valor atual do RTT, basta usar a funcao
      *   rtt_read_timer_value()
      */

	while(1) {
		
		if ((segundos%4 == 0)&&flag){
			sprintf(buffer2, "%fkm/h", velocidade(x1));
			sprintf(buffer3, "%fm", distancia(x));
			x1 = 0;
			flag = false;
		}
		if (segundos%4 != 0){
			flag = true;
		}
		tempo(rtt_read_timer_value(RTT));
		
		sprintf(buffer1, "tempo: %02d:%02d:%02d", horas, minutos, segundos);
		font_draw_text(&calibri_36, buffer1, 50, 100, 1);
		font_draw_text(&calibri_36, buffer2, 50, 200, 1);
		font_draw_text(&calibri_36, buffer3, 50, 300, 1);
		
	}
}