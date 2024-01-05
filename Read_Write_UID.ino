#include <SPI.h>
#include <MFRC522.h>

#define RST_PIN         22
#define SS_PIN          21

MFRC522 mfrc522(SS_PIN, RST_PIN);

MFRC522::MIFARE_Key key;

void setup() {
    Serial.begin(9600); // Initialize serial communications with the PC
    SPI.begin();        // Init SPI bus
    mfrc522.PCD_Init(); // Init MFRC522 card

    // Prepare the key - all keys are set to FFFFFFFFFFFFh at chip delivery from the factory
    for (byte i = 0; i < 6; i++) {
        key.keyByte[i] = 0xFF;
    }

    Serial.println(F("Choose option: \n1. Read Card UID \n2. Write New UID \n3. Copy Card UID"));
}

void loop() {
    if (Serial.available()) {
        char choice = Serial.read();
        delay(100); // Debounce delay

        if (choice == '1') {
            readCardUID();
        } else if (choice == '2') {
            writeUID();
        }  else {
            Serial.println(F("Invalid choice. Please select 1, 2, or 3."));
        }

        // Clear the Serial buffer
        while (Serial.available()) {
            Serial.read();
        }

        Serial.println(F("Choose option: \n1. Read Card UID \n2. Write New UID \n3. Copy Card UID"));
    }
}

void readCardUID() {
    if (!mfrc522.PICC_IsNewCardPresent() || !mfrc522.PICC_ReadCardSerial()) {
        Serial.println(F("No card detected."));
        return;
    }
    Serial.print(F("Card UID:"));
    for (byte i = 0; i < mfrc522.uid.size; i++) {
        Serial.print(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " ");
        Serial.print(mfrc522.uid.uidByte[i], HEX);
    }
    Serial.println();
    mfrc522.PICC_HaltA();
    mfrc522.PCD_StopCrypto1();
}

void writeUID() {
    Serial.println(F("Enter new UID to write (in HEX format, e.g., AA BB CC DD):"));
    while (!Serial.available()) {
        // Wait for UID input
    }
    String inputUID = Serial.readStringUntil('\n');
    byte newUid[4];
    parseHexString(inputUID, newUid);

    if (setCardUID(newUid)) {
        Serial.println(F("Wrote new UID to card."));
    } else {
        Serial.println(F("Failed to write new UID."));
    }
}



bool setCardUID(byte *newUid) {
    if (!mfrc522.PICC_IsNewCardPresent() || !mfrc522.PICC_ReadCardSerial()) {
        Serial.println(F("No card detected."));
        return false;
    }

    // Set new UID
    if (mfrc522.MIFARE_SetUid(newUid, (byte)4, true)) {
        Serial.println(F("Wrote new UID to card."));
        mfrc522.PICC_HaltA(); // Halt PICC
        mfrc522.PCD_StopCrypto1(); // Stop encryption on PCD
        return true;
    } else {
        Serial.println(F("Failed to set new UID."));
        return false;
    }
}

byte parseHexString(String hexString, byte *bytes) {
    hexString.trim();
    hexString.replace(" ", "");
    int byteCount = 0;
    for (unsigned int i = 0; i < hexString.length(); i += 2) {
        if (i + 1 < hexString.length()) {
            String byteString = hexString.substring(i, i + 2);
            bytes[byteCount++] = (byte) strtol(byteString.c_str(), NULL, 16);
        }
    }
    return byteCount;
}
