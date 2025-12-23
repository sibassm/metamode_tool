#ifndef MTK_META_LOGIC_H
#define MTK_META_LOGIC_H

#include <QObject>
#include <QString>
#include <QSerialPort>

class MtkMetaLogic : public QObject {
    Q_OBJECT
public:
    explicit MtkMetaLogic(QObject *parent = nullptr);

    // Main Functions
    bool connectToMeta(int portNum);
    QString performFullReadInfo(int portNum);

signals:
    void logMsg(QString msg, int level);

private:
    // Low-Level Helpers
    QByteArray sendCmd(QSerialPort &serial, QByteArray cmd, int readLen);
    QString parseHealthStatus(int value);
    QString parseLifeTime(int value);
    QString getChipsetName(quint32 hwCode);
};

#endif