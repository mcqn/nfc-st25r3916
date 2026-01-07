#include <Arduino.h>
#include <ndef_buffer.h>
#include <ndef_class.h>
#include <ndef_message.h>
#include <ndef_poller.h>
#include <ndef_record.h>
#include <ndef_type_wifi.h>
#include <ndef_types.h>
#include <ndef_types_mime.h>
#include <ndef_types_rtd.h>
#include <nfc_utils.h>
#include <rfal_isoDep.h>
#include <rfal_nfc.h>
#include <rfal_nfcDep.h>
#include <rfal_nfca.h>
#include <rfal_nfcb.h>
#include <rfal_nfcf.h>
#include <rfal_nfcv.h>
#include <rfal_rf.h>
#include <rfal_st25tb.h>
#include <rfal_st25xv.h>
#include <rfal_t1t.h>
#include <rfal_t2t.h>
#include <rfal_t4t.h>
#include <st_errno.h>
#include <rfal_rfst25r3916.h>

#if 1
// MiaB ESP32S3 board
const int kPinMOSI = 11;
const int kPinMISO = 13;
const int kPinSCK = 12;
const int kPinSS = 10;
const int kPinIRQ = 3;
const int kPinLED = 46;
#elif 0
// LyraT board
const int kPinMOSI = 13;
const int kPinMISO = 14;
const int kPinSCK = 12;
const int kPinSS = 15;
const int kPinIRQ = 18;
const int kPinLED = 23;
#else
// ESP32-S2 Saola 1 board
const int kPinMOSI = MOSI;
const int kPinMISO = MISO;
const int kPinSCK = SCK;
const int kPinSS = SS;
const int kPinIRQ = 14;
const int kPinLED = 5;
#endif

//uninitalised pointers to SPI objects
SPIClass gSPI;

RfalRfST25R3916Class gReaderHardware(&gSPI, kPinSS, kPinIRQ);
RfalNfcClass gNFCReader(&gReaderHardware);

//bool multiSel = false;

void demoNotif( rfalNfcState st )
{
    uint8_t       devCnt;
    rfalNfcDevice *dev;
    Serial.print(__FUNCTION__);
    Serial.print(".  st: ");
    Serial.println(st);

    if( st == RFAL_NFC_STATE_WAKEUP_MODE )
    {
        Serial.println("Wake Up mode started");
    }
    else if( st == RFAL_NFC_STATE_POLL_TECHDETECT )
    {
//        if( discParam.wakeupEnabled )
        {
            Serial.println("Wake Up mode terminated. Polling for devices \r\n");
        }
    }
    else if( st == RFAL_NFC_STATE_POLL_SELECT )
    {
      Serial.println("state poll select");
#if 0
        /* Check if in case of multiple devices, selection is already attempted */
        if( (!multiSel) )
        {
            multiSel = true;
            /* Multiple devices were found, activate first of them */
            rfalNfcGetDevicesFound( &dev, &devCnt );
            rfalNfcSelect( 0 );

            platformLog("Multiple Tags detected: %d \r\n", devCnt);
        }
        else
        {
            rfalNfcDeactivate( RFAL_NFC_DEACTIVATE_DISCOVERY );
        }
#endif
    }
    else if( st == RFAL_NFC_STATE_START_DISCOVERY )
    {
      Serial.println("state start discovery");
        /* Clear multiple device selection flag */
//        multiSel = false;
    }
    else if (st == RFAL_NFC_STATE_ACTIVATED)
    {
      digitalWrite(kPinLED, HIGH);
      rfalNfcDevice* nfcDev;
      gNFCReader.rfalNfcGetActiveDevice(&nfcDev);
      for (int i=0; i < nfcDev->nfcidLen; i++)
      {
        Serial.print(nfcDev->nfcid[i], HEX);
        Serial.print(" ");
      }
      // See if we can get an NDEF record from it
      NdefClass ndef(&gNFCReader);
      if (ndef.ndefPollerContextInitialization(nfcDev) == ERR_NONE)
      {
        Serial.println("ndef context initialized");
        ndefInfo info;
        if (ndef.ndefPollerNdefDetect(&info) == ERR_NONE)
        {
          Serial.println("ndef detected");
          uint8_t rawmessage[300];
          uint32_t actual_size =0;
          if (ndef.ndefPollerReadRawMessage(rawmessage, 300, &actual_size) == ERR_NONE)
          {
            Serial.print("Read message size ");
            Serial.println(actual_size);
            Serial.println((char*)rawmessage);
            ndefMessage ndefMsg;
            ndefConstBuffer ndefBuf;
            ndefBuf.buffer = rawmessage;
            ndefBuf.length = actual_size;
            if (ndef.ndefMessageDecode(&ndefBuf, &ndefMsg) == ERR_NONE)
            {
              // Got an NDEF "message" (a set of NDEF records)
              int idx =0;
              ndefRecord* record = ndefMessageGetFirstRecord(&ndefMsg);
              while (record)
              {
                Serial.print("NDEF record ");
                Serial.println(idx);
                ndefType tipo;
                if (ndef.ndefRecordToRtdText(record, &tipo) == ERR_NONE)
                {
                  // If this isn't a text record it'll fail
                  Serial.println("Text record");
                  char text[300];
                  memcpy(text, tipo.data.text.bufSentence.buffer, MIN(300, tipo.data.text.bufSentence.length));
                  text[MIN(300-1, tipo.data.text.bufSentence.length)] = '\0';
                  Serial.println(text);
                }
                else if (ndef.ndefRecordToRtdUri(record, &tipo) == ERR_NONE)
                {
                  // If this isn't a URI record it'll fail
                  Serial.println("URI record");
                  char uri[200];
                  memcpy(uri, tipo.data.uri.bufUriString.buffer, MIN(200, tipo.data.uri.bufUriString.length));
                  uri[MIN(200-1, tipo.data.uri.bufUriString.length)] = '\0';
                  Serial.println(uri);
                }
                record = ndefMessageGetNextRecord(record);
                idx++;
              }
            }
          }
        }
      }
      Serial.println();
      gNFCReader.rfalNfcDeactivate(true);
      digitalWrite(kPinLED, LOW);
    }
}

void setup() {
  Serial.begin(115200);
  Serial.println("Let's go!");
  Serial.print("MOSI: ");
  Serial.println(kPinMOSI);
  Serial.print("MISO: ");
  Serial.println(kPinMISO);
  Serial.print("SS: ");
  Serial.println(kPinSS);
  Serial.print("SCK: ");
  Serial.println(kPinSCK);
  pinMode(kPinLED, OUTPUT);
  pinMode(kPinSS, OUTPUT);
  digitalWrite(kPinSS, HIGH);
  //clock miso mosi ss
  gSPI.begin(kPinSCK, kPinMISO, kPinMOSI, kPinSS);

  for (int i = 0; i < 4; i++)
  {
    digitalWrite(kPinLED, HIGH);
    delay(1000);
    digitalWrite(kPinLED, LOW);
    delay(500);
  }

  // put your setup code here, to run once:
  Serial.print("Initializing NFC: ");
  delay(500);
  Serial.println(gNFCReader.rfalNfcInitialize());

uint8_t aat_a_val;
uint8_t aat_b_val;
  gReaderHardware.st25r3916ReadRegister(ST25R3916_REG_ANT_TUNE_A, &aat_a_val);
  gReaderHardware.st25r3916ReadRegister(ST25R3916_REG_ANT_TUNE_B, &aat_b_val);
Serial.println("AAT values:");
Serial.println(aat_a_val);
Serial.println(aat_b_val);
  
  struct st25r3916AatTuneResult tune_result;
  gReaderHardware.st25r3916AatTune(NULL, &tune_result);
  Serial.println("AAT Results:");
  Serial.print("     aat_a: ");  Serial.println(tune_result.aat_a);
  Serial.print("     aat_b: ");  Serial.println(tune_result.aat_b);
  Serial.print("       pha: ");  Serial.println(tune_result.pha);
  Serial.print("       amp: ");  Serial.println(tune_result.amp);
  Serial.print("measureCnt: "); Serial.println(tune_result.measureCnt);
  Serial.println();

  rfalNfcDiscoverParam discover_params;
  discover_params.devLimit = 1;
  discover_params.techs2Find = RFAL_NFC_POLL_TECH_A;
  discover_params.GBLen = RFAL_NFCDEP_GB_MAX_LEN;
  discover_params.notifyCb = demoNotif;
  discover_params.totalDuration = 10U;
  discover_params.wakeupEnabled = true;
  Serial.print("Starting discovery mode: ");
  Serial.println(gNFCReader.rfalNfcDiscover(&discover_params));

  Serial.println("Usage:");
  Serial.println("  typing <CR> on its own will increment both AAT_A and AAT_B by 8");
  Serial.println("  typing A will increment just AAT_A by 8");
  Serial.println("  typing B will increment just AAT_B by 8");
  Serial.println("  typing S will run a scan testing the amplitude at all values of AAT_B");
  Serial.println("  typing F will run a full scan testing the amplitude at all values of AAT_A and AAT_B");
  Serial.println("  typing <number1>,<number2> will set AAT_A to <number1> and AAT_B to <number2>");
}

uint8_t aat_a_value =0;
uint8_t aat_b_value =0;
const int commandBufLen = 20;
char command[commandBufLen];
int idx =0;

void loop() {
        gReaderHardware.st25r3916WriteRegister(ST25R3916_REG_ANT_TUNE_A, aat_a_value);
        gReaderHardware.st25r3916WriteRegister(ST25R3916_REG_ANT_TUNE_B, aat_b_value);
  gNFCReader.rfalNfcWorker();
  if (Serial.available())
  {
    int c = Serial.read();
    if (c == '\r')
    {
      // End of command!
      command[idx] = '\0';
      if (strlen(command))
      {
        Serial.print("Need to process: >>");
        Serial.print(command);
        Serial.println("<<");
        String cmd = command;
        Serial.print("First param? ");
        Serial.println(cmd);
        aat_a_value = cmd.toInt();
        cmd = cmd.substring(cmd.indexOf(',')+1);
        Serial.print("Second param? ");
        Serial.println(cmd);
        aat_b_value = cmd.toInt();
        Serial.print("Setting AAT_A to ");
        Serial.println(aat_a_value);
        Serial.print("Setting AAT_B to ");
        Serial.println(aat_b_value);
        gReaderHardware.st25r3916WriteRegister(ST25R3916_REG_ANT_TUNE_A, aat_a_value);
        gReaderHardware.st25r3916WriteRegister(ST25R3916_REG_ANT_TUNE_B, aat_b_value);
      }
      else
      {
        // Blank command, increment the AAT values
        aat_a_value += 8;
        aat_b_value += 8;
        Serial.print("Setting AAT_A to ");
        Serial.println(aat_a_value);
        Serial.print("Setting AAT_B to ");
        Serial.println(aat_b_value);
        gReaderHardware.st25r3916WriteRegister(ST25R3916_REG_ANT_TUNE_A, aat_a_value);
        gReaderHardware.st25r3916WriteRegister(ST25R3916_REG_ANT_TUNE_B, aat_b_value);
      }
      // Time for a new command
      idx = 0;
    }
    else if ((c == 'a') || (c == 'A'))
    {
      aat_a_value += 8;
      Serial.print("Setting AAT_A to ");
      Serial.println(aat_a_value);
      gReaderHardware.st25r3916WriteRegister(ST25R3916_REG_ANT_TUNE_A, aat_a_value);
    }
    else if ((c == 'b') || (c == 'B'))
    {
      aat_b_value += 8;
      Serial.print("Setting AAT_B to ");
      Serial.println(aat_b_value);
      gReaderHardware.st25r3916WriteRegister(ST25R3916_REG_ANT_TUNE_B, aat_b_value);
    }
    else if ((c == 's') || (c == 'S'))
    {
      Serial.println("Amplitude scan for AAT_B");
      Serial.println("aat_b,amplitude");
      for (int tempb =0; tempb <= 255; tempb++)
      {
        gReaderHardware.st25r3916WriteRegister(ST25R3916_REG_ANT_TUNE_B, tempb);
        Serial.print(tempb);
        delay(300);
        // Measure AM
        uint8_t amp;
        gReaderHardware.st25r3916MeasureAmplitude(&amp);
        Serial.print(",");
        Serial.println(amp);
      }
    }
    else if ((c == 'f') || (c == 'F'))
    {
      Serial.println("Full amplitude scan for AAT_B and AAT_A");
      Serial.println("aat_a,aat_b,amplitude");
      uint8_t lowest_amp =255;
      uint8_t highest_amp =0;
      for (int tempa =0; tempa <= 255; tempa+=8)
      {
        gReaderHardware.st25r3916WriteRegister(ST25R3916_REG_ANT_TUNE_A, tempa);
        for (int tempb =0; tempb <= 255; tempb+=8)
        {
          gReaderHardware.st25r3916WriteRegister(ST25R3916_REG_ANT_TUNE_B, tempb);
          Serial.print(tempa);
          Serial.print(",");
          Serial.print(tempb);
          delay(100);
          // Measure AM
          uint8_t amp;
          gReaderHardware.st25r3916MeasureAmplitude(&amp);
          lowest_amp = MIN(lowest_amp, amp);
          highest_amp = MAX(highest_amp, amp);
          Serial.print(",");
          Serial.println(amp);
        }
      }
      Serial.println();
      Serial.print("Signal ranges from ");
      Serial.print(lowest_amp);
      Serial.print(" to ");
      Serial.println(highest_amp);
    }
    else if ((c != '\n') && (idx < commandBufLen))
    {
      // Ignore newlines
      command[idx++] = c;
      // Temp print out AAT_A value
      uint8_t v;
      gReaderHardware.st25r3916ReadRegister(ST25R3916_REG_ANT_TUNE_A, &v);
      Serial.print("reading AAT_A: ");
      Serial.println(v);
      gReaderHardware.st25r3916ReadRegister(ST25R3916_REG_ANT_TUNE_B, &v);
      Serial.print("reading AAT_B: ");
      Serial.println(v);
    }
  }
}
