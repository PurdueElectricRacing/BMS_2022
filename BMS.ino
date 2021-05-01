#define CELL_MAX      4.2f
#define CELL_MIN      3.7f
#define ADC_VOLTS     5
#define DEBUG

const uint8_t  cell_pins[6] = {0, 1, 2, 3, 4, 5};
const uint16_t res_high[6] = {0, 1000, 2400, 9100, 6200, 6200};
const uint16_t res_low[6] = {1000, 1000, 1100, 3000, 1600, 1300};
float factor[6];

typedef enum
{
  ERROR,
  VALID
} error_t;

typedef struct
{
  uint16_t volts_raw[6];
  float    volts_scaled[6];
  float    volts_stack;
} cells_t;

cells_t cells;

static void scaleVolts()
{
  // Locals
  uint8_t i;
  uint8_t j;

  cells.volts_stack = 0;

  for (i = 0; i < 6; i++)
  {
      cells.volts_scaled[i] = ADC_VOLTS * ((float) cells.volts_raw[i] / 1024) * (1 / factor[i]);
      for (j = 0; j < i; j++)
      {
          cells.volts_scaled[i] -= cells.volts_scaled[j];
      }
      cells.volts_stack += cells.volts_scaled[i];
  }
}

static error_t detError()
{
   // Locals
   uint8_t i;
   error_t ret = VALID;

   for (i = 0; i < 6; i++)
   {
      if (cells.volts_scaled[i] > CELL_MAX || cells.volts_scaled[i] < CELL_MIN)
      {
          ret = ERROR;
      }
   }

   if (cells.volts_stack < (CELL_MIN * 6) || cells.volts_stack > (CELL_MAX * 6))
   {
      ret = ERROR;
   }

   return ret;
}

static void printCells(error_t err)
{
    // Locals
    uint8_t i;
    char buff[30];

    Serial.flush();

    if (err == ERROR)
    {
      Serial.println("ERROR! Cell voltage exceeded min/max!");
    }
    else
    {
      Serial.println("--- Cell Voltages -------------------");
    }
    
    Serial.print("Stack voltage: ");
    Serial.print(cells.volts_stack);
    Serial.println("V");
    
    for (i = 0; i < 6; i++)
    {
      sprintf(&buff[0], "Battery %d voltage: ", i + 1);
      Serial.print(&buff[0]);
      Serial.print(cells.volts_scaled[i]);
      Serial.println("V");
    }

    digitalWrite(LED_BUILTIN, err == ERROR ? HIGH : LOW);
}

// Run once to setup parameters
void setup()
{
  // Locals
  int i;
  char buff[30];
  
  Serial.begin(9600);
  pinMode(LED_BUILTIN, OUTPUT);

  delay(1000);

  for (i = 0; i < 6; i++)
  {
    factor[i] = ((float) res_low[i] / (res_high[i] + res_low[i]));
    #ifdef DEBUG
    sprintf(&buff[0], "Cell %d Factor: ", i + 1);
    Serial.print(&buff[0]);
    Serial.println(factor[i]);
    #endif
  }
}

// Main loop
void loop()
{
  // Locals
  uint8_t i;

  for (i = 0; i < 6; i++)
  {
    cells.volts_raw[i] = analogRead(cell_pins[i]);
  }

  scaleVolts();
  printCells(detError());

  delay(5000);
}
