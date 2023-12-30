#include "MKL46z4.h"
#include "Seg_LCD.h"
#include "board.h"

#define RED_LED 1u << 29
#define GREEN_LED 1u << 5
int32_t volatile msTicks = 0;
int state = -1;

void init_all(void) {
    SIM->SCGC5 |= SIM_SCGC5_PORTE_MASK | SIM_SCGC5_PORTD_MASK;
    PORTE->PCR[29] = PORT_PCR_MUX(1u);
    PTE->PDDR |= RED_LED;

    PORTD->PCR[5] = PORT_PCR_MUX(1u);
    PTD->PDDR |= GREEN_LED;

    PTD->PSOR = GREEN_LED;
    PTE->PSOR = RED_LED;

    SIM->SCGC5 |= SIM_SCGC5_PORTC_MASK;
    PORTC->PCR[3] = PORT_PCR_MUX(1U) | PORT_PCR_PE_MASK | PORT_PCR_PS_MASK;
    PTC->PDDR &= ~((uint32_t)(1u << 3));
    PORTC->PCR[3] |= PORT_PCR_IRQC(0xA);

    PORTC->PCR[12] = PORT_PCR_MUX(1) | PORT_PCR_PE_MASK | PORT_PCR_PS_MASK;
    PORTC->PCR[12] |= PORT_PCR_IRQC(0xA);

    NVIC_ClearPendingIRQ(PORTC_PORTD_IRQn);
    NVIC_EnableIRQ(PORTC_PORTD_IRQn);
}

void init_SysTick_interrupt() {
    SysTick->LOAD = SystemCoreClock / 1000;
    SysTick->CTRL = SysTick_CTRL_CLKSOURCE_Msk | SysTick_CTRL_TICKINT_Msk | SysTick_CTRL_ENABLE_Msk;
}

void SysTick_Handler(void) {
    msTicks++;
}

void PORTC_PORTD_IRQHandler(void) {
		if ((PTC->PDIR & (1<<12)) == 0){}
		if ((PTC->PDIR & (1<<3)) == 0){
			state *= -1;
		}
		PORTC->PCR[3] |= PORT_PCR_ISF_MASK;
    PORTC->PCR[12] |= PORT_PCR_ISF_MASK;
}

int main(void) {
		BOARD_BootClockRUN();
		SegLCD_Init();
		init_all();
		init_SysTick_interrupt();
		NVIC_SetPriority(PORTC_PORTD_IRQn,1);
		NVIC_SetPriority(SysTick_IRQn,0);
		while (1) {
			msTicks = 0;
			if(state == 1){
				PTD->PCOR = GREEN_LED;
				while((PTC->PDIR & (1 << 3)) != 0) {
					msTicks = 0;
					while((PTC->PDIR & (1 << 3)) != 0 && msTicks < 10000 && (PTC->PDIR & (1 << 12)) != 0){
						segLCD_Time_count(msTicks);
					}
					while ((PTC->PDIR & (1 << 3)) != 0 && (PTC->PDIR & (1 << 12)) != 0) {
						PTE->PCOR = RED_LED;
						SegLCD_SeatBelt();
					}
					PTE->PSOR = RED_LED;
					SegLCD_SeatBeltOff();
				}
			}
			if(state == -1){
				PTD->PSOR = GREEN_LED;
				PTE->PSOR = RED_LED;
				SegLCD_SeatBeltOff();
			}
		}
		return 0;
}
