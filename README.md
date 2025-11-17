# Mifare Classic 1K Reader

Un'applicazione web semplice per leggere l'UID di card Mifare Classic 1K utilizzando l'API Web NFC di Chrome.

## 🎯 Funzionalità

- Lettura dell'UID di card Mifare Classic 1K
- Visualizzazione dell'UID in formato esadecimale
- Interfaccia utente semplice e intuitiva
- Informazioni dettagliate sulla card letta

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

- L'API Web NFC può leggere solo card che supportano NDEF
- Per card Mifare Classic 1K, l'UID è sempre leggibile
- Non tutte le operazioni di lettura/scrittura sono supportate dall'API Web NFC

## 📄 Licenza

Questo progetto è rilasciato sotto licenza MIT.
