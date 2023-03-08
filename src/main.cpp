#include <Arduino.h>
#include <userdatatypes/sampletype.h>
#include <acquisition/sampler.h>

// impiego di un timer hardware per il periodo di campionamento
// periodo di campionamento Ts = 200us

// puntatore alla struttura timer da associare al timer hardware
hw_timer_t *timer = NULL;
// dataReady è posto a true nella ISR dopo aver acquisito tutti i campioni del buffer
volatile bool dataReady = false;
// dichiarazione della interrupt handling routine
void IRAM_ATTR onTimer();

// dimensione del buffer dati
const u_int32_t BUFFER_LENGTH = 5000;

// buffer dei campioni
volatile Sample sampleBuffer[BUFFER_LENGTH];
// puntatore alla prossima cella libera del buffer
volatile Sample *pSampleBuffer = &sampleBuffer[0];

// indice nel buffer dati
volatile uint32_t indice = 0;

void setup()
{
  Serial.begin(115200);
  Serial.printf("Sampler %d campioni con ISR\n", BUFFER_LENGTH);

  // impiego il timer hardware numero 0, con prescaler 80 (Tbit = 1us), conteggio in avanti
  timer = timerBegin(0, 80, true);
  // associazione della ISR al timer per gestire l'interrupt periodico
  timerAttachInterrupt(timer, onTimer, true);
  // impostazione del periodo di campionamento: 200us
  timerAlarmWrite(timer, 200, true);
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
      Serial.printf("%4d\t%4d\t%4d\n", sampleBuffer[indice].iax, sampleBuffer[indice].iay, sampleBuffer[indice].iaz);
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
  if (indice < BUFFER_LENGTH && !dataReady)
  {
    getSample(pSampleBuffer++);
    indice++;
  }

  if (indice >= BUFFER_LENGTH)
  {
    dataReady = true;
  }
}