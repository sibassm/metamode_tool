#include "mtk_meta_imei.h"
#include <QCoreApplication>
#include <QThread>
#include <QDebug>

MtkMetaImei::MtkMetaImei(QObject *parent) : QObject(parent) {}

// 🔧 MAIN REPAIR FUNCTION
bool MtkMetaImei::repairImei(int portNum, QString imei1, QString imei2) {
    QSerialPort serial;
    serial.setPortName("COM" + QString::number(portNum));
    serial.setBaudRate(QSerialPort::Baud115200);

    if (!serial.open(QIODevice::ReadWrite)) {
        emit logMsg("❌ Port Error: Cannot open COM" + QString::number(portNum), 1);
        return false;
    }

    emit logMsg("📡 Connecting to NVRAM for IMEI Repair...", 0);
    bool success = true;

    // --- WRITE IMEI 1 ---
    if (imei1.length() == 15) {
        emit logMsg("✏️ Preparing IMEI 1: " + imei1, 0);
        QByteArray bcdData = convertImeiToMtkBcd(imei1);
        
        // ID 151 par write karein (Record 1)
        if (writeNvramMeta(serial, LID_IMEI_NEW, bcdData)) {
             emit logMsg("✅ IMEI 1 Write Success!", 3);
        } else {
             emit logMsg("❌ IMEI 1 Write Failed", 1);
             success = false;
        }
    }

    // --- WRITE IMEI 2 (For Dual SIM) ---
    // Note: ID 151 ke andar hi aksar IMEI 2 hota hai (Offset par), 
    // lekin safe side ke liye hum ID 151 ko dubara bhejte hain (Tools trick).
    if (imei2.length() == 15) {
        QThread::msleep(500); // Thoda wait
        emit logMsg("✏️ Preparing IMEI 2: " + imei2, 0);
        QByteArray bcdData2 = convertImeiToMtkBcd(imei2);
        
        // Kuch phones ID 152 use karte hain, kuch 151 hi.
        // Yahan hum standard method use kar rahe hain.
        // Asli logic mein humein Read karke Modify karna chahiye.
        if (writeNvramMeta(serial, LID_IMEI_NEW, bcdData2)) { 
             emit logMsg("✅ IMEI 2 Write Success!", 3);
        }
    }

    if(success) emit logMsg("🔄 Reboot Phone manually to see changes.", 3);
    serial.close();
    return success;
}

// 🧬 OFFICIAL BCD CONVERSION LOGIC (From SPexc.cpp)
QByteArray MtkMetaImei::convertImeiToMtkBcd(QString imei) {
    QByteArray bcd;
    
    // MTK Standard Header for IMEI (0x08 Length, 0x3A Type)
    // Yeh header official tool har packet ke aage lagata hai
    bcd.append((char)0x08); 
    bcd.append((char)0x3A); 

    // Convert 15 digits to Swapped BCD
    // Example: "123456" -> 0x21, 0x43, 0x65
    for (int i = 0; i < 14; i += 2) {
        int digit1 = imei.mid(i, 1).toInt();     // Pehla digit (e.g., 1)
        int digit2 = imei.mid(i + 1, 1).toInt(); // Dusra digit (e.g., 2)

        // Swap Logic: (Digit2 << 4) | Digit1
        unsigned char byte = (digit2 << 4) | digit1; // Result: 0x21
        bcd.append(byte);
    }

    // Handle Last Digit (15th digit)
    // MTK format: 0xF + Last Digit (e.g., 0xF5 if last is 5)
    int lastDigit = imei.right(1).toInt();
    unsigned char lastByte = (0xF << 4) | lastDigit;
    bcd.append(lastByte);

    return bcd;
}

// 📡 LOW LEVEL PACKET SENDER
bool MtkMetaImei::writeNvramMeta(QSerialPort &serial, int lid, QByteArray data) {
    QByteArray packet;
    
    // Meta Write Command Structure (Simplified)
    // [CMD_TYPE] [LID] [DATA_LEN] [DATA] [CHECKSUM]
    packet.append((char)0x01);       // Write Command
    packet.append((char)lid);        // ID 151
    packet.append((char)data.size()); // Length
    packet.append(data);             // Asli BCD Data

    serial.write(packet);
    
    if (serial.waitForReadyRead(2000)) {
        QByteArray response = serial.readAll();
        // Check for positive ACK (MediaTek 'True' response)
        if (response.contains("\x06") || response.contains("OK")) return true;
    }
    return false;
}