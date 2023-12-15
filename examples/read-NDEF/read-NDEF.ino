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

  // put your setup code here, to run once:
  Serial.print("Initializing NFC: ");
  Serial.println(gNFCReader.rfalNfcInitialize());

  rfalNfcDiscoverParam discover_params;
  discover_params.devLimit = 1;
  discover_params.techs2Find = RFAL_NFC_POLL_TECH_A;
  discover_params.GBLen = RFAL_NFCDEP_GB_MAX_LEN;
  discover_params.notifyCb = demoNotif;
  discover_params.totalDuration = 1000U;
  discover_params.wakeupEnabled = true;
  Serial.print("Starting discovery mode: ");
  Serial.println(gNFCReader.rfalNfcDiscover(&discover_params));
}

void loop() {
  // put your main code here, to run repeatedly:
  gNFCReader.rfalNfcWorker();
}
