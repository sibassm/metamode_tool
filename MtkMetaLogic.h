#ifndef MTK_META_LOGIC_H
#define MTK_META_LOGIC_H

#include <QObject>
#include <QString>
#include <QByteArray>

class MtkMetaLogic : public QObject {
    Q_OBJECT
public:
    explicit MtkMetaLogic(QObject *parent = nullptr);
    
    // Core Handshake logic from mtkclient
    bool enterMetaMode(int portNum, QString mode = "METAMETA");
    
    // A to Z Deep Read Logic
    QString readFullDeviceDetails(int portNum);
    
    // Security Operations
    bool repairImei(int portNum, QString imei1, QString imei2);
    bool resetFactory(int portNum);

signals:
    void logMsg(QString msg, int level);

private:
    // Low level USB commands (Backend)
    bool sendMetaCommand(const QByteArray &cmd);
    QByteArray readMetaResponse(int size);
    
    // Partition & NVRAM Helpers
    QByteArray readNvItem(int id);
    QString parseGptTable();
};

#endif