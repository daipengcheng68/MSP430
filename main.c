#include "LCD.h"
#include "driverlib.h"

void initGPIO(void);
void buttonPress1(void);
void buttonPress2(void);
//void wiperSpeed(void);

volatile unsigned char * S1buttonDebounce = &BAKMEM2_L;       // S1 button debounce flag
volatile unsigned char * S2buttonDebounce = &BAKMEM2_H;       // S2 button debounce flag
//volatile unsigned int holdCount = 0;
unsigned long count1 = 90;

/*#define stepOne 500000
#define stepTwo 250000
#define stepThree 100000*/

// TimerA0 UpMode Configuration Parameter
Timer_A_initUpModeParam initUpParam_A0 =
{
        TIMER_A_CLOCKSOURCE_SMCLK,              // SMCLK Clock Source
        TIMER_A_CLOCKSOURCE_DIVIDER_1,          // SMCLK/1 = 2MHz
        30500,                                  // debounce period
        TIMER_A_TAIE_INTERRUPT_DISABLE,         // Disable Timer interrupt
        TIMER_A_CCIE_CCR0_INTERRUPT_ENABLE ,    // Enable CCR0 interrupt
        TIMER_A_DO_CLEAR,                       // Clear value
        true                                    // Start Timer
};


int main(void)
{

    WDT_A_hold(WDT_A_BASE); // Stop Watchdog timer

    WDT_A_hold(__MSP430_BASEADDRESS_WDT_A__);
    initGPIO();//initialize gpio pins


    TA1CCR0 = 1800-1;                         // PWM Period


    TA1CTL = TASSEL__SMCLK | MC__UP | TACLR;  // SMCLK, up mode, clear TAR
    PM5CTL0 &= ~LOCKLPM5;

 //   *S1buttonDebounce = *S2buttonDebounce =  0; //s1 s2 debounce

    LCD_init(); // Initialize LCD
    LCD_displayNumber(count1);

    while (1)
    {
    __bis_SR_register( GIE + LPM3_bits);  // Enable interrupts globally and low power mode3

    __no_operation();           // Placeholder for while loop (not required)
    }

    return 0;
}

#pragma vector=PORT1_VECTOR
__interrupt void pushbutton_ISR(void)
{
    switch (__even_in_range(P1IV, P1IV_P1IFG7))
    {
    case P1IV_P1IFG2:       // Pin 1 (button 1)
        if ((*S1buttonDebounce) == 0)
        {
            *S1buttonDebounce = 1;                        // First high to low transition
            //holdCount = 0;
        }
        buttonPress1();
        __bic_SR_register_on_exit(LPM3_bits);      // exit the low power mode 3 and back to next commend after lpm intrinsic
        break;
    default:
        _never_executed();
    }
}

#pragma vector=PORT2_VECTOR
__interrupt void pushbutton_ISR2(void)
{
    switch (__even_in_range(P2IV, P2IV_P2IFG7))
    {
    case P2IV_P2IFG6:       // Pin 2 (button 2)
        if ((*S2buttonDebounce) == 0)
        {
            *S2buttonDebounce = 1;                        // First high to low transition
            //holdCount = 0;
        }

        buttonPress2();
        __bic_SR_register_on_exit(LPM3_bits);
        break;
    default:
        _never_executed();
    }
//    GPIO_toggleOutputOnPin( GPIO_PORT_P4, GPIO_PIN0);
//    GPIO_clearInterrupt( GPIO_PORT_P2, GPIO_PIN6);
}

void initGPIO(void)
{

    // Set pin P1.0 to output direction and turn LED off
    GPIO_setAsOutputPin( GPIO_PORT_P1, GPIO_PIN0);    // Red LED (LED1)
    GPIO_setOutputLowOnPin( GPIO_PORT_P1, GPIO_PIN0);
    GPIO_setAsOutputPin( GPIO_PORT_P4, GPIO_PIN0);    // Red LED (LED1)
    GPIO_setOutputLowOnPin( GPIO_PORT_P4, GPIO_PIN0);

    // Unlock pins (required for FRAM devices)
    // Unless waking from LPMx.5, this should be done before clearing
    // and enabling GPIO port interrupts
    PMM_unlockLPM5();

    // Set P1.2 as input with pull-up resistor (for push button S1)
    //  configure interrupt on low-to-high transition
    //  and then clear flag and enable the interrupt
    GPIO_setAsInputPinWithPullUpResistor( GPIO_PORT_P1, GPIO_PIN2);
    GPIO_selectInterruptEdge( GPIO_PORT_P1, GPIO_PIN2, GPIO_LOW_TO_HIGH_TRANSITION);
    GPIO_clearInterrupt( GPIO_PORT_P1, GPIO_PIN2);
    GPIO_enableInterrupt( GPIO_PORT_P1, GPIO_PIN2);

    GPIO_setAsInputPinWithPullUpResistor( GPIO_PORT_P2, GPIO_PIN6);
    GPIO_selectInterruptEdge( GPIO_PORT_P2, GPIO_PIN6, GPIO_LOW_TO_HIGH_TRANSITION);
    GPIO_clearInterrupt( GPIO_PORT_P2, GPIO_PIN6);
    GPIO_enableInterrupt( GPIO_PORT_P2, GPIO_PIN6);
    displayScrollText("PWM PROJECT START WITH 90");
}
void buttonPress1(void)
{
    LCD_displayNumber(count1);
    GPIO_toggleOutputOnPin( GPIO_PORT_P1, GPIO_PIN0);
    /*if (count1 == 90)
    {
        displayScrollText("IN OFF MODE PRESS SWITCH 2 TO START");
        return;
    }
    else */if (count1 == 0){
        displayScrollText("ALREADY IN BOTTOM");
        return;
    }
    count1 = count1 - 5;

    P1DIR |= BIT7;
    P1SEL0 |= BIT7;//P1.7 output PWM

    TA1CCTL1 = OUTMOD_7;                      // CCR1 reset/set
    TA1CCR1 = count1 * 10;                    // CCR1 PWM duty cycle

    displayScrollText("5 DEGREE DOWN");
    LCD_displayNumber(TA1CCR1/10);

    GPIO_clearInterrupt( GPIO_PORT_P1, GPIO_PIN0);

}

void buttonPress2(void)
{
    LCD_displayNumber(count1);
    GPIO_toggleOutputOnPin( GPIO_PORT_P4, GPIO_PIN0);
    if (count1 == 180)
    {
        displayScrollText("ALREADY IN TOP LEVEL");
        return;
    }
    count1 = count1 + 5;

    P1DIR |= BIT7;
    P1SEL0 |= BIT7;//P1.7 output PWM


    TA1CCTL1 = OUTMOD_7;                      // CCR1 reset/set
    TA1CCR1 = count1 * 10;                    // CCR1 PWM duty cycle

    displayScrollText("5 DEGREE UP");

    LCD_displayNumber(TA1CCR1/10);

    GPIO_clearInterrupt( GPIO_PORT_P2, GPIO_PIN6);

}

/*void wiperSpeed(void)
{
   // unsigned int j = 0 ;
    if (count1 == 45)
    {
        for (;; )
        {
            volatile unsigned int i;         // volatile to prevent optimization

            // P1OUT ^= 0x01;                      // Toggle P1.0 using exclusive-OR
            GPIO_toggleOutputOnPin( GPIO_PORT_P1, GPIO_PIN0);

            __delay_cycles(stepOne);
        }

    }
    else if (count1 == 135)
    {
        for (;;)
        {
            volatile unsigned int i;         // volatile to prevent optimization

            // P1OUT ^= 0x01;                      // Toggle P1.0 using exclusive-OR
            GPIO_toggleOutputOnPin( GPIO_PORT_P1, GPIO_PIN0);

            __delay_cycles(stepOne);
        }

    }
    else if (count1 == 180)
    {
        for (;;)
        {
            volatile unsigned int i;         // volatile to prevent optimization

            // P1OUT ^= 0x01;                      // Toggle P1.0 using exclusive-OR
            GPIO_toggleOutputOnPin( GPIO_PORT_P1, GPIO_PIN0);

            __delay_cycles(stepTwo);
        }

    }
}
*/
