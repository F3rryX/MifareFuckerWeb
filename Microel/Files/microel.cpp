#include "microel.h"
#include "menu.h"
#include "srix4k.h"

extern PN532 nfc;

void calcolaSommaHex(uint8_t uid[], size_t uidSize, uint8_t sommaHex[])
{
  const uint8_t xorKey[] = {0x01, 0x92, 0xA7, 0x75, 0x2B, 0xF9};
  // Calcolare la somma di ogni singolo carattere
  int somma = 0;
  for (size_t i = 0; i < uidSize; i++)
  {
    somma += uid[i];
  }

  // Ottenere gli ultimi due numeri della somma
  int sommaDueNumeri = somma % 256;

  if (sommaDueNumeri % 2 == 1)
  {
    sommaDueNumeri += 2;
  }

  // Eseguire l'operazione XOR tra sommaDueNumeri e xorKey
  for (size_t i = 0; i < sizeof(xorKey); i++)
  {
    sommaHex[i] = sommaDueNumeri ^ xorKey[i];
  }
}

void generaKeyA(uint8_t uid[], uint8_t uidSize, uint8_t keyA[])
{
  uint8_t sommaHex[6];
  calcolaSommaHex(uid, uidSize, sommaHex);
  uint8_t primoCarattere = (sommaHex[0] >> 4) & 0xF;

  if (primoCarattere == 0x2 || primoCarattere == 0x3 || primoCarattere == 0xA || primoCarattere == 0xB)
  {
    // XOR WITH 0x40
    for (size_t i = 0; i < sizeof(sommaHex); i++)
    {
      keyA[i] = 0x40 ^ sommaHex[i];
    }
  }
  else if (primoCarattere == 0x6 || primoCarattere == 0x7 || primoCarattere == 0xE || primoCarattere == 0xF)
  {
    // XOR WITH 0xC0
    for (size_t i = 0; i < sizeof(sommaHex); i++)
    {
      keyA[i] = 0xC0 ^ sommaHex[i];
    }
  }
  else
  {
    for (size_t i = 0; i < sizeof(sommaHex); i++)
    {
      keyA[i] = sommaHex[i];
    }
  }
}

void generaKeyB(uint8_t keyA[], size_t keyASize, uint8_t keyB[])
{
  for (size_t i = 0; i < keyASize; i++)
  {
    keyB[i] = 0xFF ^ keyA[i];
  }
}

char *lastbyte(const uint8_t *data, size_t length)
{
  int sommaint = 0;

  for (size_t i = 0; i < length; ++i)
  {
    sommaint += data[i];
  }

  char ultimo_byte[3];
  sprintf(ultimo_byte, "%02X", sommaint);

  int byteIntValue = (int)strtol(ultimo_byte, NULL, 16);
  int nuovo_valore = byteIntValue + 33;

  char *risultato = (char *)malloc(3); // 2 caratteri per l'hex + '\0'
  if (risultato == NULL)
  {
    LOG_ERROR(F("Error to allocate lastbyte checksum microel"));
    return NULL;
  }

  sprintf(risultato, "%02X", nuovo_valore);

  if (strlen(risultato) == 3)
  {
    // Elimina il primo carattere
    return strdup(&risultato[1]);
  }

  return risultato;
}

void infoMicroel()
{
  uint8_t data[16];
  uint16_t previus_credit;
  uint16_t credit;
  uint8_t uidBuffer[4] = {0, 0, 0, 0};
  uint8_t uidLength;
  stampaCentrale(F("Waiting for tag..."));
  LOG_INFO(F("Waiting for microel tag..."));
  if (nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, &uidBuffer[0], &uidLength)) // Wait for a card
  {
    if (uidLength == 4)
    {
      uint8_t keya[6];
      generaKeyA(uidBuffer, uidLength, keya);
      if (nfc.mifareclassic_AuthenticateBlock(uidBuffer, uidLength, MICROEL_BLOCK_CREDIT, 0, keya)) // 0 = keyA 1 = keyB
      {
        if (nfc.mifareclassic_ReadDataBlock(MICROEL_BLOCK_CREDIT, data)) // Read credit
        {
          credit = (data[6] << 8) | (data[5]);
          if (nfc.mifareclassic_ReadDataBlock(MICROEL_BLOCK_PREVIUS_CREDIT, data)) // Read previus_credit
          {
            previus_credit = (data[6] << 8) | (data[5]);
            stampaInfoMifare(credit, previus_credit, uidLength, uidBuffer);
          }
        }
        else
        {
          LOG_ERROR(F("Read error, holds key still"));
          stampaCentrale(F("Read error, holds key still"));
        }
      }
      else
      {
        LOG_ERROR(F("Authentication failed for block "), MICROEL_BLOCK_CREDIT);
        stampaCentrale(F("Please enter Microel!"));
      }
    }
    delay(4000);
    stampaMenu(2);
  }
}
void chargeMicroel()
{
  uint8_t data[16];
  uint16_t credit;
  uint8_t uidBuffer[4] = {0, 0, 0, 0};
  uint8_t uidLength;
  stampaCentrale(F("Waiting for tag..."));
  LOG_INFO(F("Waiting for microel tag..."));
  if (nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, &uidBuffer[0], &uidLength)) // Wait for a card
  {
    if (uidLength == 4)
    {
      uint8_t keya[6];
      generaKeyA(uidBuffer, uidLength, keya);
      if (nfc.mifareclassic_AuthenticateBlock(uidBuffer, uidLength, MICROEL_BLOCK_CREDIT, 0, keya)) // 0 = keyA 1 = keyB
      {
        if (nfc.mifareclassic_ReadDataBlock(MICROEL_BLOCK_CREDIT, data)) // Read credit
        {
          // ottengo il credito e ci aggiungo 20 euro
          credit = (data[6] << 8) | (data[5]);
          credit += 3000;

          // Aggiungo il credito nelle posizione appropiate dell'array
          data[5] = (uint8_t)(credit & 0xFF);
          data[6] = (uint8_t)((credit >> 8) & 0xFF);

          // calcolo il checksum
          char *last_byte = lastbyte(data, 15);
          data[15] = strtol(last_byte, NULL, 16);

          /* Writing a key */
          if (nfc.mifareclassic_WriteDataBlock(MICROEL_BLOCK_CREDIT, data)) // Write credit
          {
            if (nfc.mifareclassic_WriteDataBlock(MICROEL_BLOCK_PREVIUS_CREDIT, data)) // Write actual_credit in previus_credit
            {
              if (nfc.mifareclassic_ReadDataBlock(MICROEL_BLOCK_CREDIT, data)) // Read microel block credit to check new credit added!
              {
                // check credit added with new read credit
                if (credit == (data[6] << 8) | (data[5]))
                {
                  stampaCentrale(String(credit / 100) + F("E is new credit!"));
                  LOG_INFO(F("Charge done! Credit if correctly added!"));
                }
                else
                {
                  LOG_ERROR(F("No credit change, contact support!"));
                  stampaCentrale(F("No credit change, contact support!"));
                }
              }
              else
              {
                LOG_ERROR(F("Error to read block 4")); // 3595
                stampaCentrale(F("Error to read block, try again!"));
              }
            }
            else
            {
              LOG_ERROR(F("Error to write block 5"));
              stampaCentrale(F("Writing error, try again!"));
            }
          }
          else
          {
            LOG_ERROR(F("Error to write block 4"));
            stampaCentrale(F("Writing error, try again!"));
          }
        }
        else
        {
          LOG_ERROR(F("Read error, holds key still"));
          stampaCentrale(F("Read error, holds key still"));
        }
      }
      else
      {
        LOG_ERROR(F("Authentication failed for block 4"));
        stampaCentrale(F("Please enter Microel!"));
      }
    }
    delay(4000);
    stampaMenu(2);
  }
}

void createMicroel()
{
  uint8_t success; // Flag to check if there was an error with the PN532
  int STOP_DATA_BLOCK = 8;
  uint8_t uid[4] = {0, 0, 0, 0}; // Buffer to store the returned UID
  uint8_t uidLength;             // Length of the UID (4 or 7 bytes depending on ISO14443A card type)
  uint8_t currentblock;          // Counter to keep track of which block we're on
  bool authenticated = false;    // Flag to indicate if the sector is authenticated
  uint8_t data[16];              // Array to store block data during reads

  // Key
  uint8_t keya[6];
  uint8_t keyb[6];
  uint8_t keyuniversal[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

  stampaCentrale(F("Waiting for tag..."));
  success = nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uidLength);
  if (success)
  {
    if (uidLength == 4)
    {
      // Generate key
      generaKeyA(uid, uidLength, keya);
      generaKeyB(keya, 6, keyb);

      // Now we try to go through all 16 sectors (each having 4 blocks)
      // authenticating each sector, and then dumping the blocks
      for (currentblock = 0; currentblock < STOP_DATA_BLOCK; currentblock++)
      {
        // Check if this is a new block so that we can reauthenticate
        if (nfc.mifareclassic_IsFirstBlock(currentblock))
          authenticated = false;

        // If the sector hasn't been authenticated, do so first
        if (!authenticated)
        {
          // Starting of a new sector ... try to to authenticate
          success = nfc.mifareclassic_AuthenticateBlock(uid, uidLength, currentblock, 1, keyuniversal);
          if (success)
          {
            authenticated = true;
          }
          else
          {
            stampaCentrale(F("Insert a fresh card!"));
            Serial.println(F("Authentication error"));
          }
        }
        // If we're still not authenticated just skip the block
        if (!authenticated)
        {
          Serial.print(F("Block "));
          Serial.print(currentblock, DEC);
          Serial.println(F(" unable to authenticate"));
          stampaCentrale(F("Insert a fresh card!"));
        }
        else
        {
          // Authenticated ... we should be able to read the block now
          // Dump the data into the 'data' array
          success = nfc.mifareclassic_ReadDataBlock(currentblock, data);
          if (success)
          {
            // Change default key and set key generated for microel tag
            if (currentblock == 3 || currentblock == 7)
            {
              data[0] = keya[0];
              data[1] = keya[1];
              data[2] = keya[2];
              data[3] = keya[3];
              data[4] = keya[4];
              data[5] = keya[5];
              data[6] = 0xFF;
              data[7] = 0x07;
              data[8] = 0x80;
              data[9] = 0x69;
              data[10] = keyb[0];
              data[11] = keyb[1];
              data[12] = keyb[2];
              data[13] = keyb[3];
              data[14] = keyb[4];
              data[15] = keyb[5];
              boolean isWritten = nfc.mifareclassic_WriteDataBlock(currentblock, data);
              if (!isWritten)
              {
                Serial.print(F("Block "));
                Serial.print(currentblock, DEC);
                Serial.println(F(" unable to write"));
                stampaCentrale(F("Error to write block!"));
              }
            }

            // Maybe vendor id idk :)
            if (currentblock == 1)
            {
              data[0] = 0xF9;
              data[1] = 0x05;
              data[2] = 0x01;
              data[3] = 0x57;
              data[4] = 0x09;
              data[5] = 0x01;
              data[6] = 0x00;
              data[7] = 0x00;
              data[8] = 0x00;
              data[9] = 0x00;
              data[10] = 0x00;
              data[11] = 0x42;
              data[12] = 0x7A;
              data[13] = 0xA8;
              data[14] = 0xF6;
              data[15] = 0xDB;
              boolean isWritten = nfc.mifareclassic_WriteDataBlock(currentblock, data);
              if (!isWritten)
              {
                Serial.print(F("Block "));
                Serial.print(currentblock, DEC);
                Serial.println(F(" unable to write"));
                stampaCentrale(F("Error to write block!"));
              }
            }

            // Current Credit block is 4 previus credit block 5, i add 80 euro to dump
            if (currentblock == 4 || currentblock == 5)
            {
              data[0] = 0x17;
              data[1] = 0x00;
              data[2] = 0xC6;
              data[3] = 0x01;
              data[4] = 0xC8;
              data[5] = 0x40;
              data[6] = 0x1F;
              data[7] = 0xCE;
              data[8] = 0x58;
              data[9] = 0x35;
              data[10] = 0x0B;
              data[11] = 0x00;
              data[12] = 0x00;
              data[13] = 0x64;
              data[14] = 0x00;
              data[15] = 0xF0;
              boolean isWritten = nfc.mifareclassic_WriteDataBlock(currentblock, data);
              if (!isWritten)
              {
                Serial.print(F("Block "));
                Serial.print(currentblock, DEC);
                Serial.println(F(" unable to write"));
                stampaCentrale(F("Error to write block!"));
              }
            }

            // Credit data idk :)
            if (currentblock == 6)
            {
              data[0] = 0x0C;
              data[1] = 0x00;
              data[2] = 0xC8;
              data[3] = 0x00;
              data[4] = 0x00;
              data[5] = 0x5E;
              data[6] = 0x01;
              data[7] = 0x82;
              data[8] = 0x99;
              data[9] = 0xB4;
              data[10] = 0x08;
              data[11] = 0x00;
              data[12] = 0x00;
              data[13] = 0x96;
              data[14] = 0x00;
              data[15] = 0xC1;
              boolean isWritten = nfc.mifareclassic_WriteDataBlock(currentblock, data);
              if (!isWritten)
              {
                Serial.print(F("Block "));
                Serial.print(currentblock, DEC);
                Serial.println(F(" unable to write"));
                stampaCentrale(F("Error to write block!"));
              }
            }
          }
          else
          {
            // Oops ... something happened
            Serial.print(F("Block "));
            Serial.print(currentblock, DEC);
            Serial.println(F(" unable to read this block"));
            stampaCentrale(F("Error to read block!"));
          }
        }
      }
      stampaCentrale(F("Microel created!"));
      delay(4000);
      stampaMenu(2);
    }
    else
    {
      Serial.println("Ooops ... this doesn't seem to be a Mifare Classic card!");
      stampaCentrale(F("Insert a fresh card!"));
    }
  }
}


//Visto che un dump vergine delle microel non esiste vado a inserire il dump del mio autolavaggio
void resetMicroel(){
  createMicroel();
}
