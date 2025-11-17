#include "mizip.h"
#include "menu.h"
#include "srix4k.h"
extern PN532 nfc;

void mizip_generate_key(uint8_t *uid, uint8_t keyA[4][KEY_LENGTH], uint8_t keyB[4][KEY_LENGTH])
{
  // Static XOR table for key generation
  static const uint8_t xor_table_keyA[4][6] = {
      {0x09, 0x12, 0x5A, 0x25, 0x89, 0xE5},
      {0xAB, 0x75, 0xC9, 0x37, 0x92, 0x2F},
      {0xE2, 0x72, 0x41, 0xAF, 0x2C, 0x09},
      {0x31, 0x7A, 0xB7, 0x2F, 0x44, 0x90}};

  static const uint8_t xor_table_keyB[4][6] = {
      {0xF1, 0x2C, 0x84, 0x53, 0xD8, 0x21},
      {0x73, 0xE7, 0x99, 0xFE, 0x32, 0x41},
      {0xAA, 0x4D, 0x13, 0x76, 0x56, 0xAE},
      {0xB0, 0x13, 0x27, 0x27, 0x2D, 0xFD}};

  // Permutation table for rearranging elements in uid
  static const uint8_t xorOrderA[6] = {0, 1, 2, 3, 0, 1};
  static const uint8_t xorOrderB[6] = {2, 3, 0, 1, 2, 3};

  // Generate key based on uid and XOR table
  for (uint8_t j = 1; j < 5; j++)
  {
    for (uint8_t i = 0; i < 6; i++)
    {
      keyA[j][i] = uid[xorOrderA[i]] ^ xor_table_keyA[j - 1][i];
      keyB[j][i] = uid[xorOrderB[i]] ^ xor_table_keyB[j - 1][i];
    }
  }
}

/*
void readAllBlockMizip()
{
  uint8_t uidBuffer[4] = {0, 0, 0, 0};
  uint8_t uidLength;
  uint8_t data[16];
  bool isAuthenticated;
  stampaCentrale(F("Waiting for MiZIP tag..."));
  LOG_INFO(F("Waiting for MiZIP tag..."));

  if (nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, &uidBuffer[0], &uidLength) && uidLength == 4)
  {
    // Generate MiZIP keys 
    uint8_t keyA[4][KEY_LENGTH];
    uint8_t keyB[4][KEY_LENGTH];
    mizip_generate_key(uidBuffer, keyA, keyB);

    keyB[0][0] = 0xB4;
    keyB[0][1] = 0xC1;
    keyB[0][2] = 0x32;
    keyB[0][3] = 0x43;
    keyB[0][4] = 0x9E;
    keyB[0][5] = 0xEF;

    // Authentication to sector  
    for (uint32_t blockIndex = 0; blockIndex < 20; blockIndex++)
    {
      if (int(blockIndex / 4) == 0)
      {
        isAuthenticated = nfc.mifareclassic_AuthenticateBlock(uidBuffer, uidLength, blockIndex, 1, keyB[0]);
      }
      if (int(blockIndex / 4) == 1)
      {
        isAuthenticated = nfc.mifareclassic_AuthenticateBlock(uidBuffer, uidLength, blockIndex, 1, keyB[1]);
      }
      else if (int(blockIndex / 4) == 2)
      {
        isAuthenticated = nfc.mifareclassic_AuthenticateBlock(uidBuffer, uidLength, blockIndex, 1, keyB[2]);
      }
      else if (int(blockIndex / 4) == 3)
      {
        isAuthenticated = nfc.mifareclassic_AuthenticateBlock(uidBuffer, uidLength, blockIndex, 1, keyB[3]);
      }
      else if (int(blockIndex / 4) == 4)
      {
        isAuthenticated = nfc.mifareclassic_AuthenticateBlock(uidBuffer, uidLength, blockIndex, 1, keyB[4]);
      }

      if (isAuthenticated)
      {
        // Read data block based on blockIndex
        if (nfc.mifareclassic_ReadDataBlock(blockIndex, data))
        {
          // Read successful
          Serial.print(F("Block "));
          Serial.print(blockIndex, DEC);
          if (blockIndex < 10)
          {
            Serial.print(F("  "));
          }
          else
          {
            Serial.print(F(" "));
          }
          // Dump the raw data
          nfc.PrintHexChar(data, 16);
        }
        else
        {
          // Oops ... something happened
          Serial.print(F("Block "));
          Serial.print(blockIndex, DEC);
          Serial.println(F(" unable to read this block"));
        }
      }
      else
      {
        LOG_ERROR(F("Authentication failed for block "), blockIndex);
      }
    }
  }
}*/

void infoMIZIP()
{
  //readAllBlockMizip();
  uint8_t uidBuffer[4] = {0, 0, 0, 0};
  uint8_t uidLength;
  uint16_t credit;
  uint16_t previus_credit;
  uint8_t data[16];
  LOG_INFO(F("Waiting for MiZIP tag..."));
  stampaCentrale(F("Waiting for tag..."));

  /* Find a tag and get uid */
  if (nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, &uidBuffer[0], &uidLength) && uidLength == 4)
  {
    /* Generate MiZIP keys */
    uint8_t keyA[4][KEY_LENGTH];
    uint8_t keyB[4][KEY_LENGTH];
    mizip_generate_key(uidBuffer, keyA, keyB);

    /* Authentication to sector two (block data: 8/9/10) */
    if (nfc.mifareclassic_AuthenticateBlock(uidBuffer, uidLength, CREDIT_MIZIP_BLOCK, 1, keyB[2]))
    {
      /* Read block 10 for view block select pointer credit */
      if (nfc.mifareclassic_ReadDataBlock(MIZIP_CREDIT_POINTER, data))
      {
        /* Get Operation Pointer and Operation Counter */
        uint8_t pointer = data[0];
        uint8_t creditBlockDetected = CREDIT_MIZIP_BLOCK;
        uint8_t previusCreditBlockDetected = CREDIT_MIZIP_BLOCK;
        switch (data[0])
        {
        case 0xAA:
          creditBlockDetected = 8;
          previusCreditBlockDetected = 9;
          break;
        case 0x55:
          creditBlockDetected = 9;
          previusCreditBlockDetected = 8;
          break;
        }

        /* Current credit */
        if (nfc.mifareclassic_ReadDataBlock(creditBlockDetected, data))
        {
          credit = (data[2] << 8) | (data[1]);

          /* Previous credit */
          if (nfc.mifareclassic_ReadDataBlock(previusCreditBlockDetected, data))
          {
            previus_credit = (data[2] << 8) | (data[1]);
            stampaInfoMifare(credit, previus_credit, uidLength, uidBuffer);
          }
          else
          {
            stampaCentrale(F("Read error, holds key still"));
            LOG_ERROR(F("Reading previus_credit error"));
          }
        }
        else
        {
          stampaCentrale(F("Read error, holds key still"));
          LOG_ERROR(F("Reading actual_credit error"));
        }
      }
      else
      {
        LOG_ERROR(F("Error to read MiZIP Credit Pointer"));
        stampaCentrale(F("Read error, holds key still"));
      }
    }
    else
    {
      stampaCentrale(F("Please enter MiZIP!"));
      LOG_ERROR(F("Authentication failed for block 8"));
    }
    delay(4000);
    stampaMenu(1);
  }
}

void chargeMIZIP()
{
  uint8_t uidBuffer[4] = {0, 0, 0, 0};
  uint8_t uidLength;
  uint8_t data[16];
  uint8_t data_previus_credit[16];
  stampaCentrale(F("Waiting for tag..."));
  LOG_INFO(F("Waiting for MiZIP tag..."));

  if (nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, &uidBuffer[0], &uidLength) && uidLength == 4)
  {
    /* Generate MiZIP keys */
    uint8_t keyA[4][KEY_LENGTH];
    uint8_t keyB[4][KEY_LENGTH];
    mizip_generate_key(uidBuffer, keyA, keyB);

    /* Authentication to sector two */
    if (nfc.mifareclassic_AuthenticateBlock(uidBuffer, uidLength, 8, 1, keyB[2]))
    {
      /* Read block 10 for view block select pointer credit */
      if (nfc.mifareclassic_ReadDataBlock(MIZIP_CREDIT_POINTER, data))
      {
        /* Get Operation Pointer and Operation Counter */
        uint8_t pointer = data[0];
        uint8_t creditBlockDetected = CREDIT_MIZIP_BLOCK;
        uint8_t previusCreditBlockDetected = CREDIT_MIZIP_BLOCK;
        switch (data[0])
        {
        case 0xAA:
          creditBlockDetected = 8;
          previusCreditBlockDetected = 9;
          break;
        case 0x55:
          creditBlockDetected = 9;
          previusCreditBlockDetected = 8;
          break;
        }

        /* Current credit */
        if (nfc.mifareclassic_ReadDataBlock(creditBlockDetected, data))
        {
          /* Copy actual_credit for set to previus_credit */
          memcpy(data_previus_credit, data, 16);

          /* MiZIP have a two method for write and check credit*/
          switch (MODE)
          {
          // 0 = Checksum mode (Il credito è formato da due nibble di conseguenza vengono xorati tra loro come checksum)
          case (0):
          {
            /* Add 0x200 = 512 cent to key */
            data[2] += 0x02;
            data[3] = data[1] ^ data[2];
            break;
          }
          // 1 = Doppio credito mode
          case (1):
          {
            /* Add 0x200 = 512 cent to key */
            data[2] += 0x02;
            data[4] += 0x02;
            break;
          }
          }

          /* Write credit */
          if (nfc.mifareclassic_WriteDataBlock(creditBlockDetected, data))
          {
            /* Write previus credit */
            if (nfc.mifareclassic_WriteDataBlock(previusCreditBlockDetected, data_previus_credit))
            {
              /* Print credit succesfuly charge! */
              uint16_t credit = (data[2] << 8) | (data[1]);
              stampaCentrale(String(credit / 100) + "E is new credit!");
              LOG_INFO(F("Credit added!"));
            }
            else
            {
              stampaCentrale(F("Writing previusCredit error"));
              LOG_ERROR(F("Writing previus credit error"));
            }
          }
          else
          {
            stampaCentrale(F("Writing credit error"));
            LOG_ERROR(F("Writing credit error"));
          }
        }
        else
        {
          stampaCentrale(F("Read error credit, holds key still"));
          LOG_ERROR(F("Reading actual_credit error"));
        }
      }
      else
      {
        stampaCentrale(F("Read error pointer, holds key still"));
        LOG_ERROR(F("Read error, holds key still"));
      }
    }
    else
    {
      stampaCentrale(F("Please enter MiZIP!"));
      LOG_ERROR(F("Authentication failed for block 8"));
    }
    delay(4000);
    stampaMenu(1);
  }
}

void resetMIZIP()
{
  uint8_t uidBuffer[4] = {0, 0, 0, 0};
  uint8_t data[16];
  uint8_t uidLength;
  bool isAuthenticated;
  bool isWritten;
  uint8_t Settori[6][16] = {
      {0x01, 0x00, 0x00, 0x01, 0x00, 0x00, 0x80, 0x01, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x80, 0x00},
      {0x01, 0x00, 0x00, 0x01, 0x00, 0x00, 0x80, 0x01, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x80, 0x01},
      {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
      {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01},
      {0x55, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}};
  stampaCentrale(F("Waiting for tag..."));
  LOG_INFO(F("Waiting for MiZIP tag..."));

  if (nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, &uidBuffer[0], &uidLength) && uidLength == 4)
  {
    /* Generate MiZIP keys */
    uint8_t keyA[4][KEY_LENGTH];
    uint8_t keyB[4][KEY_LENGTH];
    mizip_generate_key(uidBuffer, keyA, keyB);

    /* Authentication to sector  */
    for (uint32_t blockIndex = 2; blockIndex < 11; blockIndex++)
    {
      if (blockIndex == 3 || blockIndex == 7)
      {
        continue;
      }
      if (int(blockIndex / 4) == 1)
      {
        isAuthenticated = nfc.mifareclassic_AuthenticateBlock(uidBuffer, uidLength, blockIndex, 1, keyB[1]);
      }
      else if (int(blockIndex / 4) == 2)
      {
        isAuthenticated = nfc.mifareclassic_AuthenticateBlock(uidBuffer, uidLength, blockIndex, 1, keyB[2]);
      }

      if (isAuthenticated)
      {
        // Write data block based on blockIndex
        switch (blockIndex)
        {
        case 4:
          isWritten = nfc.mifareclassic_WriteDataBlock(blockIndex, Settori[0]);
          break;
        case 5:
          isWritten = nfc.mifareclassic_WriteDataBlock(blockIndex, Settori[1]);
          break;
        case 6:
          isWritten = nfc.mifareclassic_WriteDataBlock(blockIndex, Settori[4]);
          break;
        case 8:
          isWritten = nfc.mifareclassic_WriteDataBlock(blockIndex, Settori[2]);
          break;
        case 9:
          isWritten = nfc.mifareclassic_WriteDataBlock(blockIndex, Settori[3]);
          break;
        case 10:
          isWritten = nfc.mifareclassic_WriteDataBlock(blockIndex, Settori[4]);
          break;
        default:
          break;
        }
      }
      else
      {
        LOG_ERROR(F("Authentication failed for block "), blockIndex);
      }
    }

    /* Check if a reset is good */
    if (nfc.mifareclassic_ReadDataBlock(8, data))
    {
      LOG_INFO(F("Key has been reset!"));
      stampaCentrale((data[2] == 0x00) ? F("Key has been reset!") : F("Writing error"));
    }
    else
    {
      LOG_ERROR(F("Reading error"));
      stampaCentrale(F("Reading error"));
    }

    // Turn to menu for mizip
    delay(4000);
    stampaMenu(1);
  }
}
