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
        ndef.addEventListener('reading', ({ message, serialNumber }) => {
            // Il serialNumber è l'UID della card
            const uid = serialNumber;
            
            // Mostra i risultati
            uidDiv.textContent = uid;
            
            // Prepara i dettagli
            let detailsHTML = '<p><strong>Informazioni Card:</strong></p>';
            detailsHTML += `<p>📌 UID: ${uid}</p>`;
            detailsHTML += `<p>📏 Lunghezza UID: ${uid.split(':').length} byte</p>`;
            detailsHTML += `<p>🕐 Letto il: ${new Date().toLocaleString('it-IT')}</p>`;
            
            // Informazioni sui record NDEF se presenti
            if (message.records.length > 0) {
                detailsHTML += `<p>📝 Record NDEF trovati: ${message.records.length}</p>`;
                message.records.forEach((record, index) => {
                    detailsHTML += `<p>  - Record ${index + 1}: ${record.recordType}</p>`;
                });
            } else {
                detailsHTML += '<p>📝 Nessun record NDEF presente sulla card</p>';
            }
            
            detailsDiv.innerHTML = detailsHTML;
            
            // Mostra i risultati
            resultDiv.classList.remove('hidden');
            showStatus('✅ Card letta con successo!', 'success');
            
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
