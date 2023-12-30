/******************************************************************/
//seatBelt.c							  //																				//
//Purpose:  main program of project		         	  //
/******************************************************************/

#include "MKL46z4.h"
#include "Seg_LCD.h"
#include "board.h"

#define RED_LED 1u << 29
#define GREEN_LED 1u << 5
int32_t volatile msTicks = 0;
int state = -1;								

void init_all(void) {					// initialization of 2 LED and 2 Switches
    SIM->SCGC5 |= SIM_SCGC5_PORTE_MASK | SIM_SCGC5_PORTD_MASK;
	// initialize RED LED
    PORTE->PCR[29] = PORT_PCR_MUX(1u);
    PTE->PDDR |= RED_LED;				

	// initialize GREEN LED
    PORTD->PCR[5] = PORT_PCR_MUX(1u);
    PTD->PDDR |= GREEN_LED;				

    PTD->PSOR = GREEN_LED;
    PTE->PSOR = RED_LED;

	// initialize SW1
    SIM->SCGC5 |= SIM_SCGC5_PORTC_MASK;			
    PORTC->PCR[3] = PORT_PCR_MUX(1U) | PORT_PCR_PE_MASK | PORT_PCR_PS_MASK;	// mux = 001(GPIO),PE = enable press, PS = 1 pull-up res
    PTC->PDDR &= ~((uint32_t)(1u << 3));
    PORTC->PCR[3] |= PORT_PCR_IRQC(0xA);		

	// initialize SW3
    PORTC->PCR[12] = PORT_PCR_MUX(1) | PORT_PCR_PE_MASK | PORT_PCR_PS_MASK;
    PTC->PDDR &= ~((uint32_t)(1u << 3));
    PORTC->PCR[12] |= PORT_PCR_IRQC(0xA);		

    NVIC_ClearPendingIRQ(PORTC_PORTD_IRQn);
    NVIC_EnableIRQ(PORTC_PORTD_IRQn);
}

void init_SysTick_interrupt() {				// initialization of systick
    SysTick->LOAD = SystemCoreClock / 1000;		
    SysTick->CTRL = SysTick_CTRL_CLKSOURCE_Msk | SysTick_CTRL_TICKINT_Msk | SysTick_CTRL_ENABLE_Msk;
}

void SysTick_Handler(void) {				// systick interupt handler
    msTicks++;
}

void PORTC_PORTD_IRQHandler(void) {			// switches interupt handler
		if ((PTC->PDIR & (1<<12)) == 0){}
		if ((PTC->PDIR & (1<<3)) == 0){
			state *= -1;
		}
		PORTC->PCR[3] |= PORT_PCR_ISF_MASK;
    PORTC->PCR[12] |= PORT_PCR_ISF_MASK;
}

int main(void) {
		BOARD_BootClockRUN();			// set clock for segment LCD
		SegLCD_Init();				// initization for LCD
		init_all();
		init_SysTick_interrupt();
		NVIC_SetPriority(PORTC_PORTD_IRQn,1);	// set higer interupt priority level for SW1 and SW3
		NVIC_SetPriority(SysTick_IRQn,0);	// set lower interupt priority level for systick
		while (1) {
			msTicks = 0;
			if(state == 1){						// seated	
				PTD->PCOR = GREEN_LED;
				while((PTC->PDIR & (1 << 3)) != 0) {
					msTicks = 0;				// turn on GREEN LED
										// start counter when seatbelt isn't used
					while((PTC->PDIR & (1 << 3)) != 0 && msTicks < 10000 && (PTC->PDIR & (1 << 12)) != 0){	
						segLCD_Time_count(msTicks);	// count the time on LCD
					}
					while ((PTC->PDIR & (1 << 3)) != 0 && (PTC->PDIR & (1 << 12)) != 0) {
						PTE->PCOR = RED_LED;		// turn on RED LED
						SegLCD_SeatBelt();		// turn on LCD warning
					}
					PTE->PSOR = RED_LED;			// turn off RED LED
					SegLCD_SeatBeltOff();			// turn off LCD warning
				}
			}
			if(state == -1){					// idle
				PTD->PSOR = GREEN_LED;				// turn off GREEN LED
				PTE->PSOR = RED_LED;
				SegLCD_SeatBeltOff();
			}
		}
		return 0;
}
