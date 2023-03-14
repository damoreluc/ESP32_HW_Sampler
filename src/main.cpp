#include <Arduino.h>
#include <userdatatypes/sampletype.h>
#include <acquisition/sampler.h>

// ISR su APP_CPU (1)
// trasmissione seriale su PRO_CPU(0)
// coda per il passaggio dei dati tra i due task

// dimensione della coda
static const uint8_t msgQueueLen = 20;
// coda di comunicazione tra task, variabile globale
static QueueHandle_t msg_queue;

// impiego di un timer hardware per il periodo di campionamento
// periodo di campionamento Ts = 200us
uint32_t Ts_us = 200;

// puntatore alla struttura timer da associare al timer hardware
hw_timer_t *timer = NULL;
// dichiarazione della interrupt handling routine
void IRAM_ATTR onTimer();

// indice nel buffer dati
volatile uint32_t indice = 0;

// Implementazione dei task
// task che attende un item nella coda e lo stampa
void printTask(void *parameters)
{
  // variabile locale col dato prelevato dalla coda
  Sample item;

  // stampa sul terminale
  while (1)
  {
    // controlla se è presente un messaggio nella coda
    // e in caso affermativo lo preleva (non bloccante)
    if (xQueueReceive(msg_queue, (void *)&item, 2) == pdTRUE)
    {
      Serial.printf("%4d\t%4d\t%4d\t\t%3d\n", item.iax, item.iay, item.iaz, uxQueueSpacesAvailable(msg_queue));
    }
  }
}

void setup()
{
  Serial.begin(921600);
  Serial.printf("Sampler %d campioni con ISR e coda\n", msgQueueLen);
    vTaskDelay(5000 / portTICK_PERIOD_MS);

  // creazione della coda
  msg_queue = xQueueCreate(msgQueueLen, sizeof(Sample));

  // creazione ed avvio del task di stampa
  xTaskCreatePinnedToCore(
      printTask,    // funzione da richiamare nel task
      "Print Task", // nome del task (etichetta utile per debug)
      2000,         // dimensione in byte dello stack per le variabili locali del task (minimo 768 byte)
      NULL,         // puntatore agli eventuali parametri da passare al task
      1,            // priorità del task
      NULL,         // eventuale task handle per gestire il task da un altro task
      PRO_CPU_NUM   // core su cui far girare il task
  );

  // impiego il timer hardware numero 0, con prescaler 80 (Tbit = 1us), conteggio in avanti
  timer = timerBegin(0, 80, true);
  // associazione della ISR al timer per gestire l'interrupt periodico
  timerAttachInterrupt(timer, onTimer, true);
  // impostazione del periodo di campionamento: 100us
  timerAlarmWrite(timer, Ts_us, true);
  // abilitazione del timer
  timerAlarmEnable(timer);
}

void loop()
{
}

// definition of the interrupt handling routine
void IRAM_ATTR onTimer()
{
  Sample item;
  BaseType_t result;

  getSample(&item);
  result = xQueueSendFromISR(msg_queue, (void*)&item, NULL);

}