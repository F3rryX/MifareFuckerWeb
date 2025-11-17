#ifndef MIZIP_H
#define MIZIP_H

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#define KEY_LENGTH 6
#define CREDIT_MIZIP_BLOCK 8
#define PREVIUSCREDIT_MIZIP_BLOCK 9
#define MIZIP_CREDIT_POINTER 10
#define MODE 0 // Mode settings: 0 = Checksum, 1 = Double Credit (MIZIP)

void infoMIZIP();
void chargeMIZIP();
void resetMIZIP();
#endif
