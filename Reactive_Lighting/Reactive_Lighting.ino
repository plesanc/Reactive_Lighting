int analogPin = 0; // MSGEQ7 OUT
int strobePin = 7; // MSGEQ7 STROBE
int resetPin = 8; // MSGEQ7 RESET
int spectrumValue[7];
double ledSet[3];
double prevLedSet[3];
int ledPins[] = {11, 10, 9, 5, 6}; // LED pins

int mode; // audio modes: 0 = default, 1 = base only
unsigned long lastTime, currTime; // for timeout use

// Mode 0 Vars
int domFreq; // determines the dominant frequencies
int domFreqMag; // stores dominant frequency calculated magnitudes

// Mode 1 Vars
int fade[] = {1000, 0, 0}; // fade ratio
int fadePos = 0;
int bass, fadeSweep; // bass amplitude, fade sweep during lots of sound

//double ledWeight1[] = {0,     0,   0,  20,  60,  80, 100};
//double ledWeight2[] = {0,    20,  40,  60,  40,  20,   0};
//double ledWeight3[] = {100,  80,  60,  20,   0,   0,   0};

double ledWeight1[] = {0,     0,   0,   0,  70,  90, 100};
double ledWeight2[] = {0,    10,  30, 100,  30,  10,   0};
double ledWeight3[] = {100,  90,  70,   0,   0,   0,   0};


// MSGEQ7 OUT pin produces values around 50-80
// when there is no input, so use this value to
// filter out a lot of the chaff.
int filterValue[] = {80, 80, 80, 80, 80, 80, 80};
int adjustValue = 1023;

void setup()
{
  Serial.begin(9600);
  // Read from MSGEQ7 OUT
  pinMode(analogPin, INPUT);
  // Write to MSGEQ7 STROBE and RESET
  pinMode(strobePin, OUTPUT);
  pinMode(resetPin, OUTPUT);

  // Write to led reactive lighting
  for (int j = 0; j < 3; j++)
  {
    pinMode(ledPins[j], OUTPUT);
  }

  // Set analogPin's reference voltage
  analogReference(DEFAULT); // 5V

  // Set startup values for pins
  digitalWrite(resetPin, LOW);
  digitalWrite(strobePin, HIGH);

  mode = 1; // mode select

  prevLedSet[0] = 0;
  prevLedSet[1] = 0;
  prevLedSet[2] = 0;

  lastTime = millis();
}

void loop()
{
  switch (mode) {
    case 0:
      while (1) // Default full spectrum response
      {
        // Set reset pin low to enable strobe
        digitalWrite(resetPin, HIGH);
        digitalWrite(resetPin, LOW);

        // Reset led values
        for (int j = 0; j < 3; j++)
        {
          ledSet[j] = 0;
        }

        // Get all 7 spectrum values from the MSGEQ7
        for (int i = 0; i < 7; i++)
        {
          digitalWrite(strobePin, LOW);
          delayMicroseconds(30); // Allow output to settle

          spectrumValue[i] = analogRead(analogPin);

          // Constrain any value above 1023 or below filterValue
          spectrumValue[i] = constrain(spectrumValue[i], filterValue[i], adjustValue);

          // Remap the value to a number between 0 and 255
          spectrumValue[i] = map(spectrumValue[i], filterValue[i], adjustValue, 0, 255);

          // Remove serial stuff after debugging
          //Serial.print(spectrumValue[i]);
          //Serial.print(" ");

          // Assigns weighting to each frequency // divide by 8 as it will add 8 values
          //ledSet[0] = ledSet[0] + (spectrumValue[i]*ledWeight1[i]/100)/8;
          //ledSet[1] = ledSet[1] + (spectrumValue[i]*ledWeight2[i]/100)/8;
          //ledSet[2] = ledSet[2] + (spectrumValue[i]*ledWeight3[i]/100)/8;

          digitalWrite(strobePin, HIGH);
        }

        domFreq = 0; // reset dominant frequency
        domFreqMag = 0;

        for (int i = 1; i < 6; i++)
        {
          if (domFreqMag < (spectrumValue[i - 1] / 2 + spectrumValue[i] + spectrumValue[i + 1] / 2))
          {
            domFreqMag = (spectrumValue[i - 1] / 2 + spectrumValue[i] + spectrumValue[i + 1] / 2);
            domFreq = i;
          }
        }

        switch (domFreq) {
          case 0:
            ledSet[0] = 100;
            ledSet[0] = 100;
            ledSet[0] = 100;
            break;
          case 1:
            ledSet[0] = domFreqMag / 2;
            ledSet[1] = 0;
            ledSet[2] = 0;
            break;
          case 2:
            ledSet[0] = domFreqMag / 2;
            ledSet[1] = domFreqMag / 2;
            ledSet[2] = 0;
            break;
          case 3:
            ledSet[0] = 0;
            ledSet[1] = domFreqMag / 2;
            ledSet[2] = 0;
            break;
          case 4:
            ledSet[0] = 0;
            ledSet[1] = domFreqMag / 2;
            ledSet[2] = domFreqMag / 2;
            break;
          case 5:
            ledSet[0] = 0;
            ledSet[1] = 0;
            ledSet[2] = domFreqMag / 2;
            break;
          case 6:
            ledSet[0] = domFreqMag / 2;
            ledSet[1] = domFreqMag / 2;
            ledSet[2] = domFreqMag / 2;
            break;
        }



        if ( (ledSet[0] + ledSet[1] + ledSet[2]) < 30)
        {
          ledSet[0] = 10;
          ledSet[1] = 10;
          ledSet[2] = 10;
        }

        prevLedSet[0] = (prevLedSet[0] / 3 + 2 * ledSet[0] / 3);
        prevLedSet[1] = (prevLedSet[1] / 3 + 2 * ledSet[1] / 3);
        prevLedSet[2] = (prevLedSet[2] / 3 + 2 * ledSet[2] / 3);

        for (int k = 0; k < 3; k++)
        {
          analogWrite(ledPins[k], ledSet[k]);
          prevLedSet[k] = ledSet[k];
        }

        delay(50);

        //Serial.println();
      }
    case 1: // Bass Response
      while (1)
      {
        // Set reset pin low to enable strobe
        digitalWrite(resetPin, HIGH);
        digitalWrite(resetPin, LOW);

        // Reset led values
        for (int j = 0; j < 3; j++)
        {
          ledSet[j] = 0;
        }

        // Get all 7 spectrum values from the MSGEQ7
        for (int i = 0; i < 7; i++)
        {
          digitalWrite(strobePin, LOW);
          delayMicroseconds(30); // Allow output to settle

          spectrumValue[i] = analogRead(analogPin);

          // Constrain any value above 1023 or below filterValue
          spectrumValue[i] = constrain(spectrumValue[i], filterValue[i], adjustValue);

          // Remap the value to a number between 0 and 255
          spectrumValue[i] = map(spectrumValue[i], filterValue[i], adjustValue, 0, 255);

          // Remove serial stuff after debugging
          //Serial.print(spectrumValue[i]);
          //Serial.print(" ");
          digitalWrite(strobePin, HIGH);
        }

        bass = (spectrumValue[0] + spectrumValue[1]) / 2;

        bass = constrain(bass, 5, 255);

        currTime = millis();

        if ((currTime - lastTime) > 10000) // wait for 10s timeout
        {
          if (bass > 30)
          {
            lastTime = millis();
          }

          bass = 50;
          fadeSweep = 1;
          /*
                    if (fade[0] == 1000){
                      if (fade[2] > 100){
                        fade[2]--;}
                      else if (fade[1] < 1000){
                        fade[1]++;}
                    }
                    if (fade[1] == 1000){
                      if (fade[0] > 100){
                        fade[0]--;}
                      else if (fade[2] < 1000){
                        fade[2]++;}
                    }
                    if (fade[2] == 1000){
                      if (fade[1] > 100){
                        fade[1]--;}
                      else if (fade[0] < 1000){
                        fade[0]++;}
                    }

                 fade[0] = 50;
                 fade[1] = 50;
                 fade[2] = 50;

                 for (int a = 0; a < 1900; a++) {
                   if (a < 925) {
                     fade[fadePos]++;
                   }
                   else {
                     fade[fadePos]--;
                   }
                   for (int k = 0; k < 3; k++)
                   {
                     ledSet[k] = (bass * fade[k]) / 1000;
                     analogWrite(ledPins[k], ledSet[k]);
                   }

                   delay(1);
                 }

                 fadePos++;

                 if (fadePos == 3) {
                   fadePos = 0;
                 }
          */
        }

        else
        {
          if (bass > 10) // if active reset timeout
          {
            lastTime = millis();
          }
          fadeSweep = (bass / 5);
        }



        if (fade[fadePos] >= 1000)
        {
          fade[fadePos] = 1000;
          fade[(fadePos + 2) % 3] = 0;
          fadePos = (fadePos + 1) % 3;
        }

        fade[fadePos] = fade[fadePos] + fadeSweep;
        fade[(fadePos + 2) % 3] = fade[(fadePos + 2) % 3] - fadeSweep;

        if (fade[(fadePos + 2) % 3] < 0)
        {
          fade[(fadePos + 2) % 3] = 0;
        }

        for (int k = 0; k < 3; k++)
        {
          ledSet[k] = (bass * fade[k]) / 1000;
          analogWrite(ledPins[k], ledSet[k]);
        }

        delay(10);

        //Serial.println();
      }
  }
}
