#include <Arduino.h>
#include <userdatatypes/sampletype.h>
#include <acquisition/sampler.h>

// impiego di un timer hardware per il periodo di campionamento
// periodo di campionamento ts = 200us
// visualizzazione con TelePlot tramite seriale

// puntatore alla struttura timer da associare al timer hardware
hw_timer_t *timer = NULL;
// dataReady è posto a true nella ISR dopo aver acquisito tutti i campioni del buffer
volatile bool dataReady = false;
// dichiarazione della interrupt handling routine
void IRAM_ATTR onTimer();

// periodo di campionamento in us
uint32_t ts = 200;

// dimensione del buffer dati
const u_int32_t BUFFER_LENGTH = 50;

// buffer dei campioni
volatile Sample sampleBuffer[BUFFER_LENGTH];
// puntatore alla prossima cella libera del buffer
volatile Sample *pSampleBuffer = &sampleBuffer[0];

// indice nel buffer dati
volatile uint32_t indice = 0;

void setup()
{
  Serial.begin(921600);
  Serial.printf("Sampler %d campioni con ISR\n", BUFFER_LENGTH);

  // impiego il timer hardware numero 0, con prescaler 80 (Tbit = 1us), conteggio in avanti
  timer = timerBegin(0, 80, true);
  // associazione della ISR al timer per gestire l'interrupt periodico
  timerAttachInterrupt(timer, onTimer, true);
  // impostazione del periodo di campionamento: 200us
  timerAlarmWrite(timer, ts, true);
  // abilitazione del timer
  timerAlarmEnable(timer);
}

void loop()
{

  // quando il buffer è pieno...
  if (dataReady)
  {
    // stampa dei dati
    for (indice = 0; indice < BUFFER_LENGTH; indice++)
    {
      // push data but do not perform immediate plotting
      // Serial.printf("%4d\t%4d\t%4d\n", sampleBuffer[indice].iax, sampleBuffer[indice].iay, sampleBuffer[indice].iaz);
      Serial.printf(">Ax:%f:%d\n", sampleBuffer[indice].timestamp/1000.0, sampleBuffer[indice].iax);
      Serial.printf(">Ay:%f:%d\n", sampleBuffer[indice].timestamp/1000.0, sampleBuffer[indice].iay);
      Serial.printf(">Az:%f:%d\n", sampleBuffer[indice].timestamp/1000.0, sampleBuffer[indice].iaz);
    }

    Serial.println();

    delay(10);
    // rewind del puntatore e dell'indice
    pSampleBuffer = &sampleBuffer[0];
    indice = 0;
    dataReady = false;
  }
}

// definition of the interrupt handling routine
void IRAM_ATTR onTimer()
{
  static uint32_t tc = 0;

  if (indice < BUFFER_LENGTH && !dataReady)
  {
    getSample(pSampleBuffer++, tc);
    tc += ts;
    indice++;
  }

  if (indice >= BUFFER_LENGTH)
  {
    dataReady = true;
  }
}