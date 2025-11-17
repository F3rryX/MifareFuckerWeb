# Mifare Classic 1K Reader

Un'applicazione web semplice per leggere l'UID di card Mifare Classic 1K utilizzando l'API Web NFC di Chrome, con calcolo automatico delle chiavi Microel.

## 🎯 Funzionalità

- Lettura dell'UID di card Mifare Classic 1K
- **Calcolo automatico delle chiavi Microel (Key A e Key B)**
- Visualizzazione dell'UID in formato esadecimale
- Interfaccia utente semplice e intuitiva
- Informazioni dettagliate sulla card letta
- Informazioni sulla struttura Mifare Classic 1K

## 🔐 Algoritmo Microel

L'applicazione implementa l'algoritmo proprietario Microel per la generazione delle chiavi di autenticazione:

1. **calcolaSommaHex(uid)**: Calcola una somma hash dall'UID usando XOR con chiave specifica
2. **generaKeyA(uid)**: Genera la Key A dall'UID basandosi sul primo nibble del hash
3. **generaKeyB(keyA)**: Genera la Key B facendo XOR della Key A con 0xFF

Queste chiavi permettono di autenticarsi sui settori della card Microel per leggere/scrivere i dati del credito.

## 📋 Requisiti

- **Browser**: Chrome o Edge (versione Android)
- **Sistema Operativo**: Android con NFC abilitato
- **Permessi**: NFC deve essere abilitato sul dispositivo

## 🚀 Come Usare

1. Apri il file `index.html` nel browser Chrome su un dispositivo Android
2. Assicurati che l'NFC sia abilitato sul dispositivo
3. Premi il pulsante "📖 Leggi Card"
4. Concedi i permessi NFC quando richiesto
5. Avvicina la card Mifare Classic 1K al lettore NFC del dispositivo
6. L'UID verrà visualizzato automaticamente a schermo

## 📝 Note

- L'API Web NFC è supportata solo su Chrome/Edge per Android
- Non funziona su desktop o iOS
- Richiede una connessione HTTPS per funzionare (tranne per localhost)

## 🔧 Sviluppo Locale

Per testare localmente:

1. Puoi aprire direttamente il file `index.html` dal filesystem
2. Oppure usa un server locale:
   ```bash
   python3 -m http.server 8000
   ```
   Poi apri `http://localhost:8000` sul tuo dispositivo Android

## ⚠️ Limitazioni

- L'API Web NFC può leggere solo l'UID e i record NDEF
- **Non è possibile leggere i blocchi Mifare Classic direttamente da browser**
- Per card Mifare Classic 1K, l'UID è sempre leggibile
- Per il dump completo dei blocchi, è necessario utilizzare:
  - Hardware: Lettore ACR122U o Proxmark3
  - Software: `libnfc`, `mfoc`, `mfcuk` o l'applicazione Microel originale
- Le chiavi calcolate sono corrette ma la lettura dei blocchi richiede software nativo

## 📊 Struttura Mifare Classic 1K

- **16 Settori** (0-15)
- **4 Blocchi per settore** (64 blocchi totali)
- **16 byte per blocco**
- **Blocchi Microel specifici:**
  - Blocco 4: Credito corrente
  - Blocco 5: Credito precedente
  - Blocco 6: Dati credito aggiuntivi

## 📄 Licenza

Questo progetto è rilasciato sotto licenza MIT.
