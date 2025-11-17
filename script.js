// Elementi DOM
const readBtn = document.getElementById('readBtn');
const statusDiv = document.getElementById('status');
const resultDiv = document.getElementById('result');
const uidDiv = document.getElementById('uid');
const detailsDiv = document.getElementById('details');

// Funzione per convertire ArrayBuffer in stringa esadecimale
function bufferToHex(buffer) {
    return [...new Uint8Array(buffer)]
        .map(b => b.toString(16).padStart(2, '0').toUpperCase())
        .join(':');
}

// Funzione per convertire stringa hex in array di byte
function hexToBytes(hex) {
    const bytes = [];
    for (let i = 0; i < hex.length; i += 2) {
        bytes.push(parseInt(hex.substr(i, 2), 16));
    }
    return new Uint8Array(bytes);
}

// ========== MICROEL ALGORITHMS ==========

// Calcola la somma hex per generare la chiave
function calcolaSommaHex(uid) {
    const xorKey = [0x01, 0x92, 0xA7, 0x75, 0x2B, 0xF9];
    
    // Calcolare la somma di ogni singolo carattere
    let somma = 0;
    for (let i = 0; i < uid.length; i++) {
        somma += uid[i];
    }
    
    // Ottenere gli ultimi due numeri della somma
    let sommaDueNumeri = somma % 256;
    
    if (sommaDueNumeri % 2 === 1) {
        sommaDueNumeri += 2;
    }
    
    // Eseguire l'operazione XOR tra sommaDueNumeri e xorKey
    const sommaHex = [];
    for (let i = 0; i < xorKey.length; i++) {
        sommaHex[i] = sommaDueNumeri ^ xorKey[i];
    }
    
    return sommaHex;
}

// Genera Key A dall'UID
function generaKeyA(uid) {
    const sommaHex = calcolaSommaHex(uid);
    const primoCarattere = (sommaHex[0] >> 4) & 0xF;
    const keyA = [];
    
    if (primoCarattere === 0x2 || primoCarattere === 0x3 || 
        primoCarattere === 0xA || primoCarattere === 0xB) {
        // XOR WITH 0x40
        for (let i = 0; i < sommaHex.length; i++) {
            keyA[i] = 0x40 ^ sommaHex[i];
        }
    } else if (primoCarattere === 0x6 || primoCarattere === 0x7 || 
               primoCarattere === 0xE || primoCarattere === 0xF) {
        // XOR WITH 0xC0
        for (let i = 0; i < sommaHex.length; i++) {
            keyA[i] = 0xC0 ^ sommaHex[i];
        }
    } else {
        for (let i = 0; i < sommaHex.length; i++) {
            keyA[i] = sommaHex[i];
        }
    }
    
    return keyA;
}

// Genera Key B dalla Key A
function generaKeyB(keyA) {
    const keyB = [];
    for (let i = 0; i < keyA.length; i++) {
        keyB[i] = 0xFF ^ keyA[i];
    }
    return keyB;
}

// Converte array di byte in stringa hex
function bytesToHex(bytes) {
    return bytes.map(b => b.toString(16).padStart(2, '0').toUpperCase()).join('');
}

// Funzione per mostrare lo status
function showStatus(message, type = 'info') {
    statusDiv.textContent = message;
    statusDiv.className = `status ${type}`;
}

// Funzione per verificare il supporto NFC
function checkNFCSupport() {
    if (!('NDEFReader' in window)) {
        showStatus('❌ Web NFC non è supportato su questo browser/dispositivo', 'error');
        readBtn.disabled = true;
        return false;
    }
    return true;
}

// Funzione principale per leggere la card NFC
async function readNFC() {
    try {
        showStatus('🔍 Avvicinare la card al lettore NFC...', 'info');
        resultDiv.classList.add('hidden');
        readBtn.disabled = true;

        // Crea un nuovo lettore NDEF
        const ndef = new NDEFReader();
        
        // Richiedi permessi e inizia la scansione
        await ndef.scan();
        
        showStatus('✅ Scansione attiva. Avvicinare la card...', 'success');

        // Event listener per quando viene rilevata una card
        ndef.addEventListener('reading', async ({ message, serialNumber }) => {
            // Il serialNumber è l'UID della card
            const uid = serialNumber;
            
            // Converte UID in array di byte per i calcoli
            const uidBytes = uid.split(':').map(h => parseInt(h, 16));
            
            // Genera le chiavi Microel
            const keyA = generaKeyA(uidBytes);
            const keyB = generaKeyB(keyA);
            
            // Mostra i risultati
            uidDiv.textContent = uid;
            
            // Prepara i dettagli con le chiavi calcolate
            let detailsHTML = '<p><strong>Informazioni Card:</strong></p>';
            detailsHTML += `<p>📌 UID: ${uid}</p>`;
            detailsHTML += `<p>📏 Lunghezza UID: ${uid.split(':').length} byte</p>`;
            detailsHTML += `<p>� Key A (Microel): ${bytesToHex(keyA)}</p>`;
            detailsHTML += `<p>🔑 Key B (Microel): ${bytesToHex(keyB)}</p>`;
            detailsHTML += `<p>�🕐 Letto il: ${new Date().toLocaleString('it-IT')}</p>`;
            
            // Informazioni sui record NDEF se presenti
            if (message.records.length > 0) {
                detailsHTML += `<p>📝 Record NDEF trovati: ${message.records.length}</p>`;
                message.records.forEach((record, index) => {
                    detailsHTML += `<p>  - Record ${index + 1}: ${record.recordType}</p>`;
                });
            } else {
                detailsHTML += '<p>📝 Nessun record NDEF presente sulla card</p>';
            }
            
            // Sezione dump con soluzioni per lettura blocchi
            detailsHTML += '<hr style="margin: 15px 0; border: none; border-top: 1px solid #ddd;">';
            detailsHTML += '<p><strong>📋 Dump Mifare Classic 1K:</strong></p>';
            detailsHTML += '<div class="dump-info">';
            detailsHTML += '<p><strong>⚠️ Limitazioni Web NFC API:</strong></p>';
            detailsHTML += '<p style="font-size: 0.9em;">L\'API Web NFC standard può leggere solo UID e record NDEF, non i blocchi raw Mifare Classic.</p>';
            detailsHTML += '<br>';
            detailsHTML += '<p><strong>✅ Come leggere i blocchi Mifare:</strong></p>';
            detailsHTML += '<p style="font-size: 0.9em;"><strong>Opzione 1: App Android Nativa</strong></p>';
            detailsHTML += '<p style="font-size: 0.85em;">Crea un\'app Android che usa <code>android.nfc.tech.MifareClassic</code> con le chiavi calcolate sopra.</p>';
            detailsHTML += '<br>';
            detailsHTML += '<p style="font-size: 0.9em;"><strong>Opzione 2: App esistenti (consigliato)</strong></p>';
            detailsHTML += '<p style="font-size: 0.85em;">• <strong>Mifare Classic Tool (MCT)</strong> - App open source su F-Droid/GitHub</p>';
            detailsHTML += '<p style="font-size: 0.85em;">• <strong>NFC TagInfo by NXP</strong> - Reader avanzato su Play Store</p>';
            detailsHTML += '<p style="font-size: 0.85em;">• <strong>NFC Tools PRO</strong> - Con funzioni di dump avanzate</p>';
            detailsHTML += '<br>';
            detailsHTML += '<p style="font-size: 0.9em;"><strong>📝 Come usare le chiavi con MCT:</strong></p>';
            detailsHTML += '<p style="font-size: 0.85em;">1. Installa Mifare Classic Tool</p>';
            detailsHTML += '<p style="font-size: 0.85em;">2. Vai in "Edit or Add Key File"</p>';
            detailsHTML += `<p style="font-size: 0.85em;">3. Aggiungi Key A: <code>${bytesToHex(keyA)}</code></p>`;
            detailsHTML += `<p style="font-size: 0.85em;">4. Aggiungi Key B: <code>${bytesToHex(keyB)}</code></p>`;
            detailsHTML += '<p style="font-size: 0.85em;">5. Usa "Read Tag" per fare il dump completo</p>';
            detailsHTML += '<br>';
            detailsHTML += `<p><strong>Struttura Mifare Classic 1K:</strong></p>`;
            detailsHTML += `<p style="font-size: 0.85em;">• 16 Settori (0-15) con 4 blocchi ciascuno = 64 blocchi totali</p>`;
            detailsHTML += `<p style="font-size: 0.85em;">• 16 byte per blocco = 1024 byte totali (1K)</p>`;
            detailsHTML += `<p style="font-size: 0.85em;"><strong>• Blocco 4:</strong> Credito corrente (Microel)</p>`;
            detailsHTML += `<p style="font-size: 0.85em;"><strong>• Blocco 5:</strong> Credito precedente (Microel)</p>`;
            detailsHTML += `<p style="font-size: 0.85em;"><strong>• Blocco 6:</strong> Dati credito aggiuntivi</p>`;
            detailsHTML += '</div>';
            
            detailsDiv.innerHTML = detailsHTML;
            
            // Mostra i risultati
            resultDiv.classList.remove('hidden');
            showStatus('✅ Card letta - Usa le chiavi con Mifare Classic Tool (MCT)', 'success');
            
            // Riabilita il bottone
            readBtn.disabled = false;
        });

        // Gestione errori durante la lettura
        ndef.addEventListener('readingerror', () => {
            showStatus('❌ Errore durante la lettura della card', 'error');
            readBtn.disabled = false;
        });

    } catch (error) {
        console.error('Errore NFC:', error);
        
        let errorMessage = '❌ Errore: ';
        
        if (error.name === 'NotAllowedError') {
            errorMessage += 'Permesso NFC negato. Concedi i permessi nelle impostazioni del browser.';
        } else if (error.name === 'NotSupportedError') {
            errorMessage += 'NFC non supportato su questo dispositivo.';
        } else if (error.name === 'NotReadableError') {
            errorMessage += 'Impossibile accedere al lettore NFC. Assicurati che sia abilitato.';
        } else {
            errorMessage += error.message || 'Errore sconosciuto durante la lettura NFC.';
        }
        
        showStatus(errorMessage, 'error');
        readBtn.disabled = false;
    }
}

// Event listener per il bottone di lettura
readBtn.addEventListener('click', readNFC);

// Verifica supporto NFC all'avvio
window.addEventListener('DOMContentLoaded', () => {
    if (checkNFCSupport()) {
        showStatus('✅ Web NFC supportato. Premi "Leggi Card" per iniziare.', 'success');
    }
});
