#include <Arduino.h>
#include <userdatatypes/sampletype.h>
#include <acquisition/sampler.h>

// dimensione del buffer dati
const u_int32_t BUFFER_LENGTH = 10;

// buffer dei campioni
Sample sampleBuffer[BUFFER_LENGTH];
// puntatore alla prossima cella libera del buffer
Sample *pSampleBuffer;

// indice nel buffer dati
uint32_t indice = 0;

void setup() {
  Serial.begin(115200);
  Serial.printf("Sampler %d campioni\n", BUFFER_LENGTH);
  
}

void loop() {
  // rewind del puntatore e dell'indice
  pSampleBuffer = &sampleBuffer[0];
  indice = 0;

  // acquisizione dei campioni nel buffer
  do {
    getSample(pSampleBuffer);
    delay(1);
    indice++;
    pSampleBuffer++;
  } while (indice<BUFFER_LENGTH);

  // stampa dei dati
  for(indice = 0; indice < BUFFER_LENGTH; indice++) {
    Serial.printf("%4d\t%4d\t%4d\n", sampleBuffer[indice].iax, sampleBuffer[indice].iay, sampleBuffer[indice].iaz);
  }

  delay(500);

}