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

## ⚠️ Limitazioni e Soluzioni

### Limitazioni Web NFC API
- L'API Web NFC può leggere solo l'UID e i record NDEF
- **Non è possibile leggere i blocchi Mifare Classic direttamente da browser**
- Questa è una limitazione del W3C Web NFC standard, non un bug

### ✅ Come Leggere i Blocchi Mifare

#### Opzione 1: App Android (Consigliato)
Usa **Mifare Classic Tool (MCT)** - App open source per Android:

1. Installa da [F-Droid](https://f-droid.org/packages/de.syss.MifareClassicTool/) o [GitHub](https://github.com/ikarus23/MifareClassicTool)
2. Leggi l'UID con questo sito web per ottenere le chiavi
3. In MCT vai su "Edit or Add Key File" 
4. Aggiungi le chiavi Key A e Key B mostrate dal sito
5. Usa "Read Tag" per il dump completo della card

#### Opzione 2: Altre App Android
- **NFC TagInfo by NXP** - Reader avanzato gratuito
- **NFC Tools PRO** - A pagamento ma con molte funzionalità

#### Opzione 3: Hardware Dedicato
- **Proxmark3** - Il più potente per ricerca e analisi
- **Chameleon Mini** - Emulatore e clonatore
- **ACR122U** + libnfc - Per uso con PC/Linux

### 🎯 Workflow Consigliato

1. **Usa questo sito** per leggere UID e generare le chiavi Microel
2. **Copia le chiavi** mostrate (Key A e Key B)
3. **Apri Mifare Classic Tool** sul tuo Android
4. **Importa le chiavi** nel key file
5. **Leggi la card** per ottenere il dump completo
6. **Modifica i blocchi** (es. Blocco 4 e 5 per il credito Microel)

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
