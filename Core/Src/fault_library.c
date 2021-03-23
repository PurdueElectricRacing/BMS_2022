#include "fault_library.h"


TaskHandle_t Faul_Task_Handle;
uint8_t task_active = 0;

void faultTask();
void callFunction(uint8_t bit_num, void (*handlers[])());
void setFault(uint8_t loc, uint8_t val);
void setHistoric(uint8_t loc, uint8_t val);
void setCriticality(uint8_t loc, uint8_t val);

// GENERATED VALUES --------
const uint16_t rise_init[FAULT_MAX] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
const uint16_t fall_init[FAULT_MAX] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
#define HISTORIC_INIT 0b00000000000000000000000000000000
#define ENABLE_INIT 0b00000000000000000000111111111111
#define CRITICALITY_INIT 0b0000000000000000000000000000000000000000101010101010101010101010
const void (*set_handler_init[FAULT_MAX])() = {overvoltageFaultSet, NULL, overSOCFaultSet, NULL, NULL, NULL, NULL, NULL, tempConnectionFaultSet, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL};
const void (*cont_handler_init[FAULT_MAX])() = {NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL};
const void (*off_handler_init[FAULT_MAX])() = {NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL};
// END GENERATED VALUES -------


//starts the fault task
void faultLibInitialize()
{
  eepromLoadStruct(FAULT_EEPROM_NAME);

  memcpy(faults.rise_threshold, rise_init, sizeof(faults.rise_threshold));
  memcpy(faults.fall_threshold, fall_init, sizeof(faults.fall_threshold));
  // Convert thresholds from ms to ticks
  for(int i = 0; i < FAULT_MAX; i++)
  {
    faults.rise_threshold[i] /= PERIOD_FAULT_TASK;
    faults.fall_threshold[i] /= PERIOD_FAULT_TASK;
  }
  faults.historic_type = HISTORIC_INIT;
  faults.enable_type = ENABLE_INIT;
  faults.stored.criticality = CRITICALITY_INIT;
  memcpy(faults.set_handler, set_handler_init, sizeof(faults.set_handler));
  memcpy(faults.cont_handler, cont_handler_init, sizeof(faults.cont_handler));
  memcpy(faults.off_handler, off_handler_init, sizeof(faults.off_handler));

  faults.stored.signal = 0;
  faults.stored.set = 0;

  // TODO: get rid of RTOS task functions
  task_active = 1;
  xTaskCreate(faultTask, "Faults", 256, NULL, 1, &Faul_Task_Handle);
}

//main loop
void faultTask()
{
  while (task_active)
  {
    TickType_t current_tick_time = xTaskGetTickCount();

    for (int i = 0; i < FAULT_MAX; i++)
    {
      if (getHistoricOverriding(i) && getFaultEnabled(i) != FAULT_DISABLED)
      {
        callFunction(i, faults.cont_handler);
        setFault(i, 1);
      }
      else if (getFaultSet(i))
      {
        //fault is currently active
        callFunction(i, faults.cont_handler);

        if (getFaultSignal(i))
        {
          //reset cooldown counter if set again
          faults.current_time[i] = 0;
        }
        else if (faults.fall_threshold[i] == 0)
        {
          // fall threshold infinite
        }
        else if (faults.current_time[i] >= faults.fall_threshold[i])
        {
          // It fell for the minimum time, turn fault off
          setFault(i, 0);
          // reset counter for rise
          faults.current_time[i] = 0;
          callFunction(i, faults.off_handler);
        }
        else
        {
          //Currently falling, increment timer
          faults.current_time[i]++;
        }
      }
      else if(getFaultSignal(i) && getFaultEnabled(i) != FAULT_DISABLED)
      {
        // fault is not set, use rise counter

        // if rise threshold is 0, instant set
        if (faults.current_time[i] < faults.rise_threshold[i])
        {
          faults.current_time[i]++;
        }
        else
        {
          // fault set, reset timer for cooldown
          setFault(i, 1);
          setHistoric(i, 1);
          faults.current_time[i] = 0;
          callFunction(i, faults.set_handler);

          //Called once
          switch(getCriticality(i))
          {
            case FAULT_WARNING:
              handleWarningFault();
            break;
            case FAULT_ERROR:
              handleErrorFault();
            break;
            case FAULT_CRITICAL:
              handleCriticalFault();
            break;
          }

        }
      }
      else
      {
        //fault and signal off, reset timer
        faults.current_time[i] = 0;
      }

    }// end for loop

    //read signals, reset for next cycle
    faults.stored.signal = 0;

    vTaskDelayUntil(&current_tick_time, pdMS_TO_TICKS(PERIOD_FAULT_TASK));
  }

  vTaskDelete(NULL);
}

void faultLibShutdown()
{
  if(task_active) {
    //vTaskDelete(Faul_Task_Handle);
    task_active = 0; //stops main loop
    eepromSaveStruct(FAULT_EEPROM_NAME);
  }
}

//clears history on controller and in eeprom
void clearHistory()
{
  faults.stored.historic = 0;
  eepromSaveStruct(FAULT_EEPROM_NAME);
}

//calls at function at the specified index
void callFunction(uint8_t bit_num, void (*handlers[])())
{
  if (handlers[bit_num] != NULL)
  {
    (*handlers[bit_num])(); //call corresponding function
  }
}

uint8_t getFaultSet(uint8_t loc)
{
  return ((faults.stored.set >> loc) & 1);
}

uint8_t getFaultSignal(uint8_t loc)
{
  return ((faults.stored.signal >> loc) & 1);
}

uint8_t getHistoricOverriding(uint8_t loc)
{
  if (((faults.historic_type >> loc) & 1) == HISTORIC_OVERRIDE)
  {
    return ((faults.stored.historic >> loc) & 1);
  }
  else
  {
    return 0;
  }
}

fault_criticality_t getCriticality(uint8_t loc)
{
  return ((faults.stored.criticality >> loc * 2) & 0b11);
}

fault_enable_t getFaultEnabled(uint8_t loc)
{
  return ((faults.enable_type >> loc) & 1);
}

//signals that a fault has occurred
//NOT set, set requires signal on for a time threshold
void signalFault(uint8_t loc)
{
  faults.stored.signal |= 1 << loc;
}

void setFault(uint8_t loc, uint8_t val)
{
  faults.stored.set &= (uint32_t) ~(0b1 << loc);
  faults.stored.set |= val << loc;
}

void setHistoric(uint8_t loc, uint8_t val)
{
  faults.stored.historic &= (uint32_t) ~(0b1 << loc);
  faults.stored.historic |= val << loc;
}

void setCriticality(uint8_t loc, uint8_t val)
{
  faults.stored.criticality &= ~(0b11 << loc * 2);
  faults.stored.criticality |= val << loc * 2;
}

__weak void handleCriticalFault()
{
  //Define elsewhere if you want to use it
}

__weak void handleErrorFault()
{
  //Define elsewhere if you want to use it
}

__weak void handleWarningFault()
{
  //Define elsewhere if you want to use it
}
