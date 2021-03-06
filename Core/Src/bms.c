/*
 * bms.c
 *
 * Created: 9/19/20
 */

#include "bms.h"

/*
 * TODO:
 * Add EEPROM loading parameters
 * Add EEPROM writes
 * Make AFE switch baud on first boot
 * Turn on auto monitor before sleep and use fault interrupts to wake early
 */

void initBms()
{
    // TODO: Check boot type. If boot = 0, wait for GUI response before we do anything
    bms.id = 0xff;
    initAccum();
    initAfe();

    // Initialize queues
    qConstruct(&bms.q_rx_can, sizeof(CanRxMsgTypeDef));
    qConstruct(&bms.q_tx_can, sizeof(CanTxMsgTypeDef));
    qConstruct(&bms.q_rx_can_hlc, sizeof(CanRxMsgTypeDef));
    qConstruct(&bms.q_tx_can_hlc, sizeof(CanTxMsgTypeDef));

    // Initial values
    bms.cells.balance_flags    = 0;
    bms.cells.balance_mask     = 0;
    bms.override.balance_force = 0;
}

void initScheduler()
{
    /*
        Note: This system uses timer 2
        If you want/need to use this timer, it is on you to edit the configuration
        to use a different timer.
        DO NOT ATTEMPT TO CONFIGURE THIS TIMER IN CUBE!
        The configuration for this timer is done manually, right here.

        This implementation is not a true scheduler. It is simply designed to run
        four loops at a stable frequency with each loop being slower than the last.
        If you only need to schedule four functions, and each is slower than the one previous,
        great. If you need to dynamically schedule a bunch of functions, go back to a true RTOS.

        Also, functions need to return to work properly. The scheduler works on a timer interrupt.
        If the functions called in the timer interrupt, you're going to have a stack overflow.

        Frequencies are given in OS ticks.
    */

    // Configure timer 2
    // Targeting an interrupt every 1 ms
    RCC->APB1ENR1 |= RCC_APB1ENR1_TIM2EN;                           // Enable timer clock in RCC
    TIM2->PSC = 3200 - 1;                                           // Set prescalar (clock at 10000 Hz)
    TIM2->ARR = 10;                                                 // Set auto reload value
    TIM2->CR1 &= ~(TIM_CR1_DIR);                                    // Set to count down
    TIM2->DIER |= TIM_DIER_UIE;                                     // Enable update interrupt

    // Default timing values
    memset(&scheduler.core.task_time, 0, sizeof(scheduler.core.task_time));
    memset(&scheduler.core.cpu_use, 0, sizeof(scheduler.core.cpu_use));

    // Setup watchdog
    IWDG->KR |= 0xCCCC;                                 // Enable watchdog
    IWDG->KR |= 0x5555;                                 // Enable register access
    IWDG->PR =  0;                                      // Set the prescalar value

    // TODO: Finish watchdog init and test
}

void startTasks()
{
    TIM2->CR1 |= TIM_CR1_CEN;                           // Enable the timer
    NVIC->ISER[0] |= 1 << TIM2_IRQn;                    // Unmask timer interrupts
}

void pauseTasks()
{
    TIM2->CR1 &= ~TIM_CR1_CEN;                          // Disable the timer
    NVIC->ISER[0] &= ~(1 << TIM2_IRQn);                 // Mask timer interrupts (just in case)
}

void TIM14_IRQHandler()
{
	TIM2->SR &= ~TIM_SR_UIF;                            // Acknowledge the interrupt
    ++scheduler.os_ticks;                               // Increase tick count (since counter will rollover to ARR once per ms)

    // TODO: Find places where I used TIM2->CNT and fix
    // TODO: Add free running counter if we need

    scheduler.run_next = 1;                             // Let the main loop know it should run
}

void mainLoop()
{
    while (PER == GREAT)
    {
        scheduler.core.task_entry_time = scheduler.os_ticks;                                                    // Update task entry time
        scheduler.run_next = 0;                                                                                 // Update run flag to let scheduler know we're working on next task

        // 1 ms functions
        afeProcess();
        // TODO: Ensure that model runs prior to calculating new balance flags
        //calcBalance(); This process has moved to trackAccum();
        acquireTemp();
        // TODO: faultLibUpdate(); // also, if moved to different rate, update the new rate in fault_library.h
        

        // Send CAN frames waiting in queue
        txCan();
        if (bms.id == ID_HLC)
        {
            txCanHLC();
        }

        // Receive CAN frames from HLC/LLC
        void canProcess();

        // 5 ms functions
        if (scheduler.os_ticks % 5 == 0)
        {
            trackAccum();
        }

        // 50 ms functions
        if (scheduler.os_ticks % 50 == 0)
        {
            txData();
        }

        scheduler.core.task_time = scheduler.os_ticks - scheduler.core.task_entry_time;                         // Store task closing time
        scheduler.core.bg_entry_time = scheduler.os_ticks;                                                      // Store background entry time

        // TODO: Tickle watchdog here

        while (scheduler.run_next == 0);                                                                        // Wait until the next task needs to run

        scheduler.core.bg_time = scheduler.os_ticks - scheduler.core.bg_entry_time;                             // Calculate background exit time
        scheduler.core.cpu_use = (float) scheduler.core.task_time / (scheduler.core.task_time + scheduler.core.bg_time);
    }
}
