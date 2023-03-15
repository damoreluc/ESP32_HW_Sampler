#include <Arduino.h>
#include <userdatatypes/sampletype.h>
#include <acquisition/sampler.h>
#include <wifi/parameters.h>
#include <WiFi.h>
#include <AsyncUDP.h>

// ISR su APP_CPU (1)
// trasmissione UDP su PRO_CPU(0)
AsyncUDP udp;

// coda per il passaggio dei dati tra i due task
// dimensione della coda
static const uint8_t msgQueueLen = 130;
// coda di comunicazione tra task, variabile globale
static QueueHandle_t msg_queue;

// impiego di un timer hardware per il periodo di campionamento
// periodo di campionamento Ts = 250us
uint32_t Ts_us = 250;

// puntatore alla struttura timer da associare al timer hardware
hw_timer_t *timer = NULL;
// dichiarazione della interrupt handling routine
void IRAM_ATTR onTimer();

// indice nel buffer dati
volatile uint32_t indice = 0;

// buffer payload udp
//char payload[512];
char payloadx[1500];
char payloady[1500];
char payloadz[1500];

// variabile locale col dato prelevato dalla coda
const uint8_t itemSize = 75;
char st_ms[itemSize][12];
char sax[itemSize][5];
char say[itemSize][5];
char saz[itemSize][5];



// Implementazione dei task
// task che attende un item nella coda e lo invia tramite UDP
void printTask(void *parameters)
{

  // Sample item1, item2, item3, item4, item5, item6;
  Sample items[itemSize];

  // invio pacchetto UDP
  while (1)
  {
    // controlla se sono presenti almeno due messaggi nella coda
    // e in caso affermativo li preleva (non bloccante)
    if (uxQueueMessagesWaiting(msg_queue) >= itemSize)
    {
      for (uint16_t i = 0; i < itemSize; i++)
      {
        xQueueReceive(msg_queue, (void *)&items[i], 1);
      }
      // xQueueReceive(msg_queue, (void *)&item1, 0);
      // xQueueReceive(msg_queue, (void *)&item2, 1);
      // xQueueReceive(msg_queue, (void *)&item3, 1);
      // xQueueReceive(msg_queue, (void *)&item4, 1);
      // xQueueReceive(msg_queue, (void *)&item5, 1);
      // xQueueReceive(msg_queue, (void *)&item6, 1);
      //}
      // if (xQueueReceive(msg_queue, (void *)&item, 2) == pdTRUE)
      //{
      Serial.printf("  q: %3d\n", uxQueueSpacesAvailable(msg_queue));

      // udp connesso?
      if (udp.connected())
      {
        // push data but do not perform immediate plotting

        for (uint16_t i = 0; i < itemSize; i++)
        {

          sprintf(st_ms[i], "%9.1f", ((float)items[i].timestamp / 1000.0));
          itoa(items[i].iax, sax[i], 10);
          itoa(items[i].iay, say[i], 10);
          itoa(items[i].iaz, saz[i], 10);
        }

        // float t1_ms = (float)item1.timestamp / 1000.0;
        // float t2_ms = (float)item2.timestamp / 1000.0;
        // float t3_ms = (float)item3.timestamp / 1000.0;
        // float t4_ms = (float)item4.timestamp / 1000.0;
        // float t5_ms = (float)item5.timestamp / 1000.0;
        // float t6_ms = (float)item6.timestamp / 1000.0;
        // invio UDP unicast
        // myValue:1627551892444:1;1627551892555:2;1627551892666:3
        payloadx[0] = '\0';
        strcat(payloadx, "Ax:");
        for (u_int16_t i = 0; i < itemSize; i++)
        { 
          strcat(payloadx, st_ms[i]);
          strcat(payloadx, ":");
          strcat(payloadx, sax[i]);
          if (i != itemSize - 1)
          {
            strcat(payloadx, ";");
          }
          else
          {
            strcat(payloadx, "\n");
          }
        }

        payloady[0] = '\0';
        strcat(payloady, "Ay:");
        for (u_int16_t i = 0; i < itemSize; i++)
        {
          strcat(payloady, st_ms[i]);
          strcat(payloady, ":");
          strcat(payloady, say[i]);
          if (i != itemSize - 1)
          {
            strcat(payloady, ";");
          }
          else
          {
            strcat(payloady, "\n");
          }
        }

        payloadz[0] = '\0';
        strcat(payloadz, "Az:");
        for (u_int16_t i = 0; i < itemSize; i++)
        {
          strcat(payloadz, st_ms[i]);
          strcat(payloadz, ":");
          strcat(payloadz, saz[i]);
          if (i != itemSize - 1)
          {
            strcat(payloadz, ";");
          }
          else
          {
            strcat(payloadz, "\n");
          }
        }
/*
        // concatena le tre parti
        payload[0] = '\0';
        strcat(payload, payloadx);
        strcat(payload, payloady);
        strcat(payload, payloadz);
*/
        /*        sprintf(payload, "Ax:%.1f:%4d;%.1f:%4d;%.1f:%4d;%.1f:%4d;%.1f:%4d;%.1f:%4d\nAy:%.1f:%4d;%.1f:%4d;%.1f:%4d;%.1f:%4d;%.1f:%4d;%.1f:%4d\nAz:%.1f:%4d;%.1f:%4d;%.1f:%4d;%.1f:%4d;%.1f:%4d;%.1f:%4d\n",
                        t1_ms, item1.iax, t2_ms, item2.iax, t3_ms, item3.iax, t4_ms, item4.iax, t5_ms, item5.iax, t6_ms, item6.iax,
                        t1_ms, item1.iay, t2_ms, item2.iay, t3_ms, item3.iay, t4_ms, item4.iay, t5_ms, item5.iay, t6_ms, item6.iay,
                        t1_ms, item1.iaz, t2_ms, item2.iaz, t3_ms, item3.iaz, t4_ms, item4.iaz, t5_ms, item5.iaz, t6_ms, item6.iaz);
          */
        Serial.print("X: "); Serial.println(strlen(payloadx));
        Serial.print("Y: "); Serial.println(strlen(payloady)); 
        Serial.print("Z: "); Serial.println(strlen(payloadz));       
        udp.print(payloadx);
        udp.print(payloady);
        udp.print(payloadz);
      }
    }
  }
}

void setup()
{
  Serial.begin(921600);
  Serial.printf("UDP Sampler %d campioni con ISR e coda\n", msgQueueLen);
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  if (WiFi.waitForConnectResult() != WL_CONNECTED)
  {
    Serial.println("WiFi Failed");
    while (1)
    {
      vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
  }
  vTaskDelay(3000 / portTICK_PERIOD_MS);
  // 192.168.0.67
  if (udp.connect(IPAddress(192, 168, 0, 67), TELEPLOT_PORT))
  {
    Serial.println("UDP connected");
  }

  // creazione della coda
  msg_queue = xQueueCreate(msgQueueLen, sizeof(Sample));

  // creazione ed avvio del task di stampa
  xTaskCreatePinnedToCore(
      printTask,    // funzione da richiamare nel task
      "Print Task", // nome del task (etichetta utile per debug)
      4096,         // dimensione in byte dello stack per le variabili locali del task (minimo 768 byte)
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
  static uint32_t t = 0;

  getSample(&item);
  item.timestamp = t;
  t += Ts_us;
  result = xQueueSendFromISR(msg_queue, (void *)&item, NULL);
}