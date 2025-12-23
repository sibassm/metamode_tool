#ifndef MTK_META_IMEI_H
#define MTK_META_IMEI_H

#include <QObject>
#include <QString>
#include <QSerialPort>

class MtkMetaImei : public QObject {
    Q_OBJECT
public:
    explicit MtkMetaImei(QObject *parent = nullptr);

    // Main Repair Function
    bool repairImei(int portNum, QString imei1, QString imei2);

signals:
    void logMsg(QString msg, int level);

private:
    // Official NVRAM IDs (From SPexc.cpp)
    const int LID_IMEI_NEW = 151;  // Modern Androids (NVRAM_EF_IMEI_IMEISV_LID)
    const int LID_IMEI_OLD = 5;    // Keypad Phones

    // Internal Helpers
    QByteArray convertImeiToMtkBcd(QString imei);
    bool writeNvramMeta(QSerialPort &serial, int lid, QByteArray data);
};

#endif // MTK_META_IMEI_H