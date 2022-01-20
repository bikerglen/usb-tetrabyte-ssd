//-----------------------------------------------------------------------------------------------
// EFM8UB2 USB Tetrabyte DIP Switch Memory
//

//-----------------------------------------------------------------------------------------------
// includes
//

#include "SI_EFM8UB2_Register_Enums.h"
#include "efm8_usb.h"
#include "descriptors.h"
#include "idle.h"
#include "InitDevice.h"
#include "config.h"


//-----------------------------------------------------------------------------------------------
// defines
//

// device pins
SI_SBIT(USER_LED, SFR_P0, 6);

#define SW1_1 ((P1 & 0x80) ? 0 : 1) // P1.7
#define SW1_2 ((P2 & 0x01) ? 0 : 1) // P2.0
#define SW1_3 ((P2 & 0x02) ? 0 : 1) // P2.1
#define SW1_4 ((P2 & 0x04) ? 0 : 1) // P2.2
#define SW1_5 ((P2 & 0x08) ? 0 : 1) // P2.3
#define SW1_6 ((P2 & 0x10) ? 0 : 1) // P2.4
#define SW1_7 ((P2 & 0x20) ? 0 : 1) // P2.5
#define SW1_8 ((P2 & 0x40) ? 0 : 1) // P2.6

#define SW2_1 ((P1 & 0x40) ? 0 : 1) // P1.6
#define SW2_2 ((P1 & 0x20) ? 0 : 1) // P1.5
#define SW2_3 ((P1 & 0x10) ? 0 : 1) // P1.4
#define SW2_4 ((P1 & 0x08) ? 0 : 1) // P1.3
#define SW2_5 ((P1 & 0x04) ? 0 : 1) // P1.2
#define SW2_6 ((P1 & 0x02) ? 0 : 1) // P1.1
#define SW2_7 ((P1 & 0x01) ? 0 : 1) // P1.0
#define SW2_8 ((P0 & 0x80) ? 0 : 1) // P0.7

#define SW3_1 ((P3 & 0x40) ? 0 : 1) // P3.6
#define SW3_2 ((P3 & 0x20) ? 0 : 1) // P3.5
#define SW3_3 ((P3 & 0x10) ? 0 : 1) // P3.4
#define SW3_4 ((P3 & 0x08) ? 0 : 1) // P3.3
#define SW3_5 ((P3 & 0x04) ? 0 : 1) // P3.2
#define SW3_6 ((P3 & 0x02) ? 0 : 1) // P3.1
#define SW3_7 ((P3 & 0x01) ? 0 : 1) // P3.0
#define SW3_8 ((P2 & 0x80) ? 0 : 1) // P2.7

#define SW4_1 ((P4 & 0x01) ? 0 : 1) // P4.0
#define SW4_2 ((P4 & 0x02) ? 0 : 1) // P4.1
#define SW4_3 ((P4 & 0x04) ? 0 : 1) // P4.2
#define SW4_4 ((P4 & 0x08) ? 0 : 1) // P4.3
#define SW4_5 ((P4 & 0x10) ? 0 : 1) // P4.4
#define SW4_6 ((P4 & 0x20) ? 0 : 1) // P4.5
#define SW4_7 ((P4 & 0x40) ? 0 : 1) // P4.6
#define SW4_8 ((P4 & 0x80) ? 0 : 1) // P4.7

#define LED_ON  0
#define LED_OFF 1

#define SYSCLK       48000000      // SYSCLK frequency in Hz


//-----------------------------------------------------------------------------------------------
// typedefs
//


//-----------------------------------------------------------------------------------------------
// prototypes
//

void Timer2_Init (int counts);
uint8_t ProcessButton (uint8_t whichState, whichBit, uint8_t sw);


//-----------------------------------------------------------------------------------------------
// globals
//

// signals from ISRs to main loop
volatile uint8_t flag250 = 0;

// local variables for deciding to make a USB report or not
uint8_t xdata reportNeeded;
uint8_t xdata thisUsbReportData[4];
uint8_t xdata lastUsbReportData[4];

// variables used to communicate report to USB ISR
volatile uint8_t xdata usbReportNeeded = false;
volatile uint8_t xdata usbReportData[4];
volatile uint8_t xdata usbReportRequested = false;

uint8_t xdata buttonStates[32];


//-----------------------------------------------------------------------------------------------
// SiLabs_Startup() Routine
//

void SiLabs_Startup (void)
{
  // Disable the watchdog here
}
 

//-----------------------------------------------------------------------------------------------
// main() Routine
//

int16_t main(void)
{
	uint8_t SFRPAGE_save;
	uint8_t led_timer = 0;
    uint8_t newUsbState = 0;
    uint8_t oldUsbState = 0;
    uint8_t i;

    // set default pin and peripheral configuration
    enter_DefaultMode_from_RESET();

	// Init Timer2 to generate interrupts at 250 Hz
	Timer2_Init (16000); // SYSCLK / 12 / 250

	// zero button states
	for (i = 0; i < 32; i++) {
		buttonStates[i] = 0;
	}

	// loop forever
	while (1)
	{
		// check if 100 Hz timer expired
		if (flag250) {
			flag250 = false;

			// new USB state
			newUsbState = USBD_GetUsbState ();

			// blink LED based on USB state
			if (newUsbState == USBD_STATE_CONFIGURED) {
				// run one led pattern for configured usb state
				if (led_timer < 25) {
					USER_LED = LED_ON; // LED ON
				} else if (led_timer < 50) {
					USER_LED = LED_OFF; // LED OFF
				} else if (led_timer < 75) {
					USER_LED = LED_ON; // LED ON
				} else if (led_timer < 100) {
					USER_LED = LED_OFF; // LED OFF
				}

				// tasks to run on entering configured state
				if (newUsbState != oldUsbState) {
					oldUsbState = newUsbState;
					// TODO
				}
			} else {
				// and another for all other usb states
				if (led_timer < 50) {
					USER_LED = LED_ON; // LED ON
				} else {
					USER_LED = LED_OFF; // LED OFF
				}

				// tasks to run on leaving configured state
				if ((newUsbState != oldUsbState) && (oldUsbState == USBD_STATE_CONFIGURED)) {
					oldUsbState = newUsbState;
					// TODO
				}
			}

			// update led timer
			if (++led_timer == 100) {
				led_timer = 0;
			}

			// clear USB report data
			for (i = 0; i < 4; i++) {
				thisUsbReportData[i] = 0;
			}

			// sample and process buttons
			thisUsbReportData[0] |= ProcessButton ( 0, 7, SW1_1);
			thisUsbReportData[0] |= ProcessButton ( 1, 6, SW1_2);
			thisUsbReportData[0] |= ProcessButton ( 2, 5, SW1_3);
			thisUsbReportData[0] |= ProcessButton ( 3, 4, SW1_4);
			thisUsbReportData[0] |= ProcessButton ( 4, 3, SW1_5);
			thisUsbReportData[0] |= ProcessButton ( 5, 2, SW1_6);
			thisUsbReportData[0] |= ProcessButton ( 6, 1, SW1_7);
			thisUsbReportData[0] |= ProcessButton ( 7, 0, SW1_8);

			thisUsbReportData[1] |= ProcessButton ( 8, 7, SW2_1);
			thisUsbReportData[1] |= ProcessButton ( 9, 6, SW2_2);
			thisUsbReportData[1] |= ProcessButton (10, 5, SW2_3);
			thisUsbReportData[1] |= ProcessButton (11, 4, SW2_4);
			thisUsbReportData[1] |= ProcessButton (12, 3, SW2_5);
			thisUsbReportData[1] |= ProcessButton (13, 2, SW2_6);
			thisUsbReportData[1] |= ProcessButton (14, 1, SW2_7);
			thisUsbReportData[1] |= ProcessButton (15, 0, SW2_8);

			thisUsbReportData[2] |= ProcessButton (16, 7, SW3_1);
			thisUsbReportData[2] |= ProcessButton (17, 6, SW3_2);
			thisUsbReportData[2] |= ProcessButton (18, 5, SW3_3);
			thisUsbReportData[2] |= ProcessButton (19, 4, SW3_4);
			thisUsbReportData[2] |= ProcessButton (20, 3, SW3_5);
			thisUsbReportData[2] |= ProcessButton (21, 2, SW3_6);
			thisUsbReportData[2] |= ProcessButton (22, 1, SW3_7);
			thisUsbReportData[2] |= ProcessButton (23, 0, SW3_8);

			thisUsbReportData[3] |= ProcessButton (24, 7, SW4_1);
			thisUsbReportData[3] |= ProcessButton (25, 6, SW4_2);
			thisUsbReportData[3] |= ProcessButton (26, 5, SW4_3);
			thisUsbReportData[3] |= ProcessButton (27, 4, SW4_4);
			thisUsbReportData[3] |= ProcessButton (28, 3, SW4_5);
			thisUsbReportData[3] |= ProcessButton (29, 2, SW4_6);
			thisUsbReportData[3] |= ProcessButton (30, 1, SW4_7);
			thisUsbReportData[3] |= ProcessButton (31, 0, SW4_8);

			// check if report needed
			reportNeeded = (thisUsbReportData[0] != lastUsbReportData[0]) ||
					       (thisUsbReportData[1] != lastUsbReportData[1]) ||
						   (thisUsbReportData[2] != lastUsbReportData[2]) ||
						   (thisUsbReportData[3] != lastUsbReportData[3]);

			// save key presses for the next time around
			lastUsbReportData[0] = thisUsbReportData[0];
			lastUsbReportData[1] = thisUsbReportData[1];
			lastUsbReportData[2] = thisUsbReportData[2];
			lastUsbReportData[3] = thisUsbReportData[3];

			// send report to IRQ-based USB code safely
			// TODO -- add a way for PC to send OUT report requesting the current switch state
			SFRPAGE_save = SFRPAGE;
			USB_DisableInts ();
			SFRPAGE = SFRPAGE_save;
			if (reportNeeded || usbReportRequested) {
				usbReportNeeded = true;
				usbReportRequested = false;
				for (i = 0; i < 4; i++) {
					usbReportData[i] = thisUsbReportData[i];
				}
			}
			SFRPAGE_save = SFRPAGE;
			USB_EnableInts ();
			SFRPAGE = SFRPAGE_save;

		} // end if (flag250)
	}
}


void Timer2_Init (int counts)
{
   TMR2CN0 = 0x00;                     // Stop Timer2; Clear TF2;
                                       // use SYSCLK/12 as timebase
   CKCON0 &= ~0x60;                    // Timer2 clocked based on T2XCLK;

   TMR2RL = -counts;                   // Init reload values
   TMR2 = 0xffff;                      // Set to reload immediately
   IE_ET2 = 1;                         // Enable Timer2 interrupts
   TMR2CN0_TR2 = 1;                    // Start Timer2
}


SI_INTERRUPT(TIMER2_ISR, TIMER2_IRQn)
{
    TMR2CN0_TF2H = 0;                   // Clear Timer2 interrupt flag
	flag250 = 1;
}


uint8_t ProcessButton (uint8_t whichState, whichBit, uint8_t sw)
{
	uint8_t state;

	state = buttonStates[whichState];

	switch (state) {
		case 0: state = sw ? 1 : 0; break;
		case 1: state = sw ? 2 : 0; break;
		case 2: state = sw ? 2 : 3; break;
		case 3: state = sw ? 2 : 0; break;
	}

	buttonStates[whichState] = state;

	return (state & 2) ? (1 << whichBit) : 0;
}
