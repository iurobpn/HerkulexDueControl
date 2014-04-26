#include "delay.h"
#include <Arduino.h>

void delay_micros(unsigned int time_us) {
//  unsigned long t0 = micros();
//  unsigned long ti = t0;
//  while ((ti-t0)<time_us) ti=micros();
	unsigned int i, n;
	n=42*time_us;
	for(i=0;i<n;i++);
}

void delay_millis(unsigned long time_ms) {
  unsigned long t0 = millis();
  unsigned long ti = t0;
  while ((ti-t0)<time_ms) ti=millis();
}
