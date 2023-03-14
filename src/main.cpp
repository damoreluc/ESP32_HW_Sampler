#include <Arduino.h>
#include <userdatatypes/sampletype.h>
#include <acquisition/sampler.h>

// impiego di un timer hardware per il periodo di campionamento
// periodo di campionamento ts = 200us
// visualizzazione con TelePlot tramite seriale
// due buffer dati in ping-pong

// puntatore alla struttura timer da associare al timer hardware
hw_timer_t *timer = NULL;
// dataReady è posto a true nella ISR dopo aver acquisito tutti i campioni del buffer
volatile bool dataReadyA = false;
volatile bool dataReadyB = false;
// dichiarazione della interrupt handling routine
void IRAM_ATTR onTimer();

// periodo di campionamento in us
uint32_t ts = 1000;

// dimensione del buffer dati
const u_int32_t BUFFER_LENGTH = 10;

// buffer dei campioni
volatile Sample sampleBufferA[BUFFER_LENGTH];
volatile Sample sampleBufferB[BUFFER_LENGTH];

// puntatore all'inizio del bufferA
volatile Sample *pSampleBufferA = &sampleBufferA[0];
// puntatore all'inizio del bufferB
volatile Sample *pSampleBufferB = &sampleBufferB[0];

// puntatore in scrittura alla prossima cella libera del buffer
volatile Sample *pwSampleBuffer = pSampleBufferA;
// puntatore in lettura
volatile Sample *prSampleBuffer = pSampleBufferB;
// variabile di controllo del ping-pong (valori: 0, 1, 0, 1, ...)
volatile int ciclo;

// indice nel buffer dati
volatile uint32_t indice = 0;

void setup()
{
  Serial.begin(921600);
  Serial.printf("Sampler %d campioni con ISR\n", BUFFER_LENGTH);

  ciclo = 0;
  // puntatore in scrittura alla prossima cella libera del buffer
  pwSampleBuffer = pSampleBufferA;
  // puntatore in lettura
  prSampleBuffer = pSampleBufferB;

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

  // quando il bufferA è pieno...
  if (ciclo == 0 && dataReadyA && prSampleBuffer != NULL)
  {
    // stampa dei dati
    for (indice = 0; indice < BUFFER_LENGTH; indice++)
    {
      // push data and perform immediate plotting
      Serial.printf(">Ax:%.4f:%d\n", prSampleBuffer->timestamp / 1000.0, prSampleBuffer->iax);
      Serial.printf(">Ay:%.4f:%d\n", prSampleBuffer->timestamp / 1000.0, prSampleBuffer->iay);
      Serial.printf(">Az:%.4f:%d\n", prSampleBuffer->timestamp / 1000.0, prSampleBuffer->iaz);
      prSampleBuffer++;
    }

    //  scambio dei buffer in ping-pong
    ciclo = 1;
    dataReadyA = false;

    // // puntatore in scrittura alla prossima cella libera del buffer
    // pwSampleBuffer = pSampleBufferA;

    // puntatore in lettura
    prSampleBuffer = pSampleBufferB;
  }

  // quando il bufferB è pieno...
  if (ciclo == 1 && dataReadyB && prSampleBuffer != NULL)
  {
    // stampa dei dati
    for (indice = 0; indice < BUFFER_LENGTH; indice++)
    {
      // push data and perform immediate plotting
      Serial.printf(">Ax:%.4f:%d\n", prSampleBuffer->timestamp / 1000.0, prSampleBuffer->iax);
      Serial.printf(">Ay:%.4f:%d\n", prSampleBuffer->timestamp / 1000.0, prSampleBuffer->iay);
      Serial.printf(">Az:%.4f:%d\n", prSampleBuffer->timestamp / 1000.0, prSampleBuffer->iaz);
      prSampleBuffer++;
    }

    //  scambio dei buffer in ping-pong
    ciclo = 0;
    dataReadyB = false;

    // puntatore in scrittura alla prossima cella libera del buffer
    // pwSampleBuffer = pSampleBufferA;
    // puntatore in lettura
    prSampleBuffer = pSampleBufferA;
  }
}

// definition of the interrupt handling routine
void IRAM_ATTR onTimer()
{
  static uint32_t tc = 0;
  static uint32_t idx = 0;
  Sample dato;

  getSample(&dato, tc);
  tc += ts;

  if (ciclo == 0)
  {
    if (idx < BUFFER_LENGTH && !dataReadyA)
    {
      // getSample(pwSampleBuffer++, tc);
      // tc += ts;
      pwSampleBuffer->iax = dato.iax;
      pwSampleBuffer->iay = dato.iay;
      pwSampleBuffer->iaz = dato.iaz;
      pwSampleBuffer->timestamp = dato.timestamp;
      pwSampleBuffer++;
      idx++;
    }

    if (idx >= BUFFER_LENGTH)
    {
      dataReadyA = true;
      pwSampleBuffer = pSampleBufferB;
      idx = 0;
    }
  }

  if (ciclo == 1)
  {
    if (idx < BUFFER_LENGTH && !dataReadyB)
    {
      // getSample(pwSampleBuffer++, tc);
      // tc += ts;
      pwSampleBuffer->iax = dato.iax;
      pwSampleBuffer->iay = dato.iay;
      pwSampleBuffer->iaz = dato.iaz;
      pwSampleBuffer->timestamp = dato.timestamp;
      pwSampleBuffer++;
      idx++;
    }

    if (idx >= BUFFER_LENGTH)
    {
      dataReadyB = true;
      pwSampleBuffer = pSampleBufferA;
      idx = 0;
    }
  }
}