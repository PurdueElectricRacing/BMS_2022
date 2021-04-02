#include "fault_library.h"

void callFunction(uint8_t bit_num, void (*handlers[])());
void setFault(uint8_t loc, uint8_t val);
void setHistoric(uint8_t loc, uint8_t val);
void setCriticality(uint8_t loc, uint8_t val);

// GENERATED VALUES --------
const uint16_t rise_threshold[FAULT_MAX] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
const uint16_t fall_threshold[FAULT_MAX] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
const uint8_t signal_period[FAULT_MAX] = {5, 1, 5, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1};
const void (*set_handler[FAULT_MAX])() = {overvoltageFaultSet, NULL, overSOCFaultSet, NULL, NULL, NULL, NULL, NULL, tempConnectionFaultSet, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL};
const void (*cont_handler[FAULT_MAX])() = {NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL};
const void (*off_handler[FAULT_MAX])() = {NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL};
// END GENERATED VALUES -------

//starts the fault Update
void faultLibInitialize()
{
  eepromLoadStruct(FAULT_EEPROM_NAME);
  faults.historic_type = HISTORIC_INIT;
  faults.enable_type = ENABLE_INIT;
  faults.stored.criticality = CRITICALITY_INIT;
  faults.stored.signal = 0;
  faults.stored.set = 0;
}

//main, call periodically at PERIOD_FAULT_TASK
void faultLibUpdate()
{

  for (int i = 0; i < FAULT_MAX; i++)
  {
    if (getHistoricOverriding(i) && getFaultEnabled(i) != FAULT_DISABLED)
    {
      callFunction(i, cont_handler);
      setFault(i, 1);
    }
    else if (getFaultSet(i))
    {
      //fault is currently active
      callFunction(i, cont_handler);

      if (fall_threshold[i] == 0)
      {
        // fall threshold infinite
      }
      else if (getFaultSignal(i)) 
      {
        //reset cooldown counter if set again
        faults.cycle_count[i] = 0;
      }
      else
      {
        faults.cycle_count[i]++;
      }

      // done falling condition
      if (fall_threshold[i] != 0 &&
          faults.cycle_count[i] >= fall_threshold[i] / FAULT_LIB_PERIOD)
      {
        // It fell for the minimum time, turn fault off
        setFault(i, 0);
        // reset counters for rise
        faults.cycle_count[i] = 0;
        faults.signal_count[i] = 0;
        callFunction(i, off_handler);
      }
    }
    else 
    {
      // rising counter section

      // increment signal and count or just cycle
      if (getFaultSignal(i) && getFaultEnabled(i) == FAULT_ENABLED)
      {
        faults.signal_count[i]++;
        faults.cycle_count[i]++;
      }
      else if (faults.cycle_count[i] != 0)
      {
        faults.cycle_count[i]++;
      }

      // reset condition
      if ( faults.cycle_count[i] >= (faults.signal_count[i] + 2) * 
           signal_period[i] / FAULT_LIB_PERIOD)
      {
        // not enough signaling of the fault, reset counters
        faults.signal_count[i] = 0;
        faults.cycle_count[i] = 0;
      }

      // set condition
      if ( faults.signal_count[i] > 0 &&
           faults.cycle_count[i] >= rise_threshold[i] / FAULT_LIB_PERIOD &&
           faults.signal_count[i] >= rise_threshold[i] / signal_period[i])
      {
        // fault has met the requirements to be set
        setFault(i, 1);
        setHistoric(i, 1);
        faults.signal_count[i] = 0;
        faults.cycle_count[i] = 0;

        callFunction(i, set_handler);

        //Called once
        switch (getCriticality(i))
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

  }// end for loop

  //read signals, reset for next cycle
  faults.stored.signal = 0;

}

void faultLibShutdown()
{
    eepromSaveStruct(FAULT_EEPROM_NAME);
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