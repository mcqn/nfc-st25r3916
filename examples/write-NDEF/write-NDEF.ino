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
const int kPinWriteSuccessLED = 37;
const int kPinWriteFailedLED = 38;
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

// Whether or not we should try writing the tag
bool gWriteTag = false;
// Whether or not a tag is on the reader
volatile bool gTagPresent = false;
//bool multiSel = false;

void demoNotif( rfalNfcState st )
{
    uint8_t       devCnt;
    rfalNfcDevice *dev;
#if VERBOSE_LOGGING
    Serial.print(__FUNCTION__);
    Serial.print(".  st: ");
    Serial.println(st);
#endif

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
#if VERBOSE_LOGGING
      Serial.println("state start discovery");
#endif
        /* Clear multiple device selection flag */
//        multiSel = false;
    }
    else if (st == RFAL_NFC_STATE_ACTIVATED)
    {
      rfalNfcDevice* nfcDev;
      gNFCReader.rfalNfcGetActiveDevice(&nfcDev);
      if (!gTagPresent)
      {
        // Tag just presented!
        digitalWrite(kPinLED, HIGH);
        gTagPresent = true;
        Serial.println("Tag detected.");
        Serial.print("  Tag ID: ");
        for (int i=0; i < nfcDev->nfcidLen; i++)
        {
          if (nfcDev->nfcid[i] < 0x10)
          {
            Serial.print("0");
          }
          Serial.print(nfcDev->nfcid[i], HEX);
          Serial.print(" ");
        }
        Serial.println();

        // See if we can get an NDEF record from it
        NdefClass ndef(&gNFCReader);
        if (ndef.ndefPollerContextInitialization(nfcDev) == ERR_NONE)
        {
#if VERBOSE_LOGGING
          Serial.println("ndef context initialized");
#endif
          ndefInfo info;
          // might need to call ndefPollerTagFormat if Detect fails?
          if (ndef.ndefPollerNdefDetect(&info) == ERR_NONE)
          {
#if VERBOSE_LOGGING
            Serial.println("ndef detected");
#endif
            uint8_t rawmessage[300];
            uint32_t actual_size =0;
            if (ndef.ndefPollerReadRawMessage(rawmessage, 300, &actual_size) == ERR_NONE)
            {
              Serial.print("Read message size ");
              Serial.println(actual_size);
              //Serial.println((char*)rawmessage);
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
                  Serial.print("  NDEF record ");
                  Serial.println(idx);
                  ndefType tipo;
                  if (ndef.ndefRecordToRtdText(record, &tipo) == ERR_NONE)
                  {
                    // If this isn't a text record it'll fail
                    Serial.print("    Text record: ");
                    char text[300];
                    memcpy(text, tipo.data.text.bufSentence.buffer, MIN(300, tipo.data.text.bufSentence.length));
                    text[MIN(300-1, tipo.data.text.bufSentence.length)] = '\0';
                    Serial.println(text);
                  }
                  else if (ndef.ndefRecordToRtdUri(record, &tipo) == ERR_NONE)
                  {
                    // If this isn't a URI record it'll fail
                    Serial.print("    URI record: ");
                    char uri[200];
                    memcpy(uri, tipo.data.uri.bufUriString.buffer, MIN(200, tipo.data.uri.bufUriString.length));
                    uri[MIN(200-1, tipo.data.uri.bufUriString.length)] = '\0';
                    Serial.println(uri);
                  }
                  record = ndefMessageGetNextRecord(record);
                  idx++;
                }
              }
              Serial.println();
            }
          }
        }
      }
      if (gWriteTag)
      {
        // Clear it so we don't keep writing to it
        gWriteTag = false;
        // See if we can get an NDEF record from it
        NdefClass ndef(&gNFCReader);
        if (ndef.ndefPollerContextInitialization(nfcDev) == ERR_NONE)
        {
          Serial.println("ndef context initialized");
          ndefInfo info;
          // might need to call ndefPollerTagFormat if Detect fails?
          if (ndef.ndefPollerNdefDetect(&info) == ERR_NONE)
          {
            Serial.println("ndef detected");

            // Build up our NDEF message to write
            ReturnCode err =ERR_NONE;
            // Text record first
            ndefType tipoText;
            // {"id":"<ID>","audio":"<WAV filename>"}
            const uint8_t text[] = "{\"id\":\"7644\",\"configure\":\"wifi\"}";
            ndefConstBuffer textBuf = { text, strlen((const char*)text)+1 };
            const uint8_t language[] = "en";
            ndefConstBuffer8 languageCode = { language, strlen((const char*)language)+1 };
            err = ndef.ndefRtdText(&tipoText, TEXT_ENCODING_UTF8, &languageCode, &textBuf);
            if (err == ERR_NONE)
            {
              Serial.print("RTD text created/ ");
              ndefRecord textRecord;
              err = ndef.ndefRtdTextToRecord(&tipoText, &textRecord);
              if (err == ERR_NONE)
              {
                Serial.print("Text record created. ");
                // then URL record
                ndefType tipoUri;
                // "http://www.museuminabox.org"
                const uint8_t uri[] = "museuminabox.org";
                ndefConstBuffer uriBuf = { uri, strlen((const char*)uri)+1 };
                err = ndef.ndefRtdUri(&tipoUri, NDEF_URI_PREFIX_HTTPS_WWW,  &uriBuf);
                if (err == ERR_NONE)
                {
                  Serial.print("URI type created. ");
                  ndefRecord uriRecord;
                  err = ndef.ndefRtdUriToRecord(&tipoUri, &uriRecord);
                  if (err == ERR_NONE)
                  {
                    Serial.print("URI record created. ");
                    ndefMessage ndefMsg;
                    err = ndef.ndefMessageInit(&ndefMsg);
                    if (err == ERR_NONE)
                    {
                      Serial.print("ndefMsg initialised. ");
                      err = ndef.ndefMessageAppend(&ndefMsg, &textRecord);
                      if (err == ERR_NONE)
                      {
                        Serial.print("Appended test. ");
                        err = ndef.ndefMessageAppend(&ndefMsg, &uriRecord);
                        if (err == ERR_NONE)
                        {
                          // Now try writing it
                          Serial.println("Appended uri. ");
                          err = ndef.ndefPollerWriteMessage(&ndefMsg);
                          if (err == ERR_NONE)
                          {
                            digitalWrite(kPinWriteSuccessLED, HIGH);
                            Serial.println("!!! Yay! NDEF message written successfully !!!");
                          }
                        }
                      }
                    }
                  }
                }
              }
            }
            if (err != ERR_NONE)
            {
              digitalWrite(kPinWriteFailedLED, HIGH);
              Serial.print("##### Failed to write NDEF message.  Error: ");
              Serial.println(err);
            }
          }
        }
      }
      gNFCReader.rfalNfcDeactivate(true);
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
  pinMode(kPinWriteSuccessLED, OUTPUT);
  pinMode(kPinWriteFailedLED, OUTPUT);
  pinMode(kPinSS, OUTPUT);
  digitalWrite(kPinSS, HIGH);
  //clock miso mosi ss
  gSPI.begin(kPinSCK, kPinMISO, kPinMOSI, kPinSS);

  digitalWrite(kPinWriteSuccessLED, HIGH);
  digitalWrite(kPinWriteFailedLED, HIGH);
  for (int i = 0; i < 4; i++)
  {
    digitalWrite(kPinLED, HIGH);
    delay(1000);
    digitalWrite(kPinLED, LOW);
    delay(500);
  }
  digitalWrite(kPinWriteSuccessLED, LOW);
  digitalWrite(kPinWriteFailedLED, LOW);

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
  Serial.println("AAT Tuning Results:");
  Serial.print("     aat_a: ");  Serial.println(tune_result.aat_a);
  Serial.print("     aat_b: ");  Serial.println(tune_result.aat_b);
  Serial.print("       pha: ");  Serial.println(tune_result.pha);
  Serial.print("       amp: ");  Serial.println(tune_result.amp);
  Serial.print("measureCnt: "); Serial.println(tune_result.measureCnt);
  Serial.println();
  Serial.println("FIXME Need to then /use/ these values");

  rfalNfcDiscoverParam discover_params;
  discover_params.devLimit = 1;
  discover_params.techs2Find = RFAL_NFC_POLL_TECH_A;
  discover_params.GBLen = RFAL_NFCDEP_GB_MAX_LEN;
  discover_params.notifyCb = demoNotif;
  discover_params.totalDuration = 10U;
  discover_params.wakeupEnabled = true;
  Serial.print("Starting discovery mode: ");
  Serial.println(gNFCReader.rfalNfcDiscover(&discover_params));

  Serial.println();
  Serial.println("Ready to start writing NFC tags!");
  Serial.println("  type 'w' to write a tag.");
}

void loop() {
  gNFCReader.rfalNfcWorker();
  if (gNFCReader.rfalNfcGetState() == RFAL_NFC_STATE_LISTEN_TECHDETECT)
  {
    // The reader has started listening for a tag again.
    // That means there isn't one present!
    gTagPresent = false;
    digitalWrite(kPinWriteFailedLED, LOW);
    digitalWrite(kPinWriteSuccessLED, LOW);
    digitalWrite(kPinLED, LOW);
  }
  if (Serial.available())
  {
    int c = Serial.read();
    if ((c == 'w') || (c == 'W'))
    {
      Serial.println("Need to write a tag");
      if (gTagPresent)
      {
        gWriteTag = true;
      }
      else
      {
        digitalWrite(kPinWriteFailedLED, HIGH);
        Serial.println("No tag detected!");
      }
    }
  }
}
