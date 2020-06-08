/*#include <time.h>*/

void setupTime() {
  configTime(0, 0, "10.1.1.1", "pool.ntp.org", "time.nist.gov");
  setenv("TZ", "CET-1CEST,M3.5.0,M10.5.0/3", 0);  
  Serial.println("\nWaiting for NTP server time");
  int count = 0;
  while(time(nullptr) <= 100000 && (count++ <100)) {
    Serial.print(".");
    delay(200);
  }
}

time_t getCurrentTime() {
  time_t now = time(nullptr);
  Serial.println(ctime(&now));
  return now;
}
