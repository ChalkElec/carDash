#ifndef HARDWARE_H
#define HARDWARE_H

#include <QObject>
#include <QSerialPort>
#include <QJSEngine>

#include "xmlparser.h"

class QThread;

class Hardware : public QObject {
    Q_OBJECT

    Q_PROPERTY(bool isInitialized READ isInitialized NOTIFY isInitializedChanged);

public:
    explicit Hardware(const QString& xmlConfig, QObject* parent = 0);
    ~Hardware();

    bool sendCmdAsync(const QByteArray& cmd, bool waitForComplete = false);
    QByteArray sendCmdSync(const QByteArray& cmd, int timeout = 200);

    void processData(const QByteArray& data);

    const XmlParser& parser() const;

    bool isInitialized() const;
    bool isReadyForSending() const;

    void open(const QString& port);
    void close();

    // This is the serial port returned, so that we could use it as receiver in QueuedConnection
    // to make functions run in the receiver's thread
    const QObject* workerThreadObject() const;

signals:
    void isInitializedChanged();
    void initFinished();

    void dataReceived(const QString& targetId, QVariant value);

private slots:
    void readData();

private:
    bool init();
    bool findBaudrate();
    bool setMaxBaudrate();
    void setInitialized(bool value);
    void processPacket(const QByteArray& str);

private:
    volatile bool isSearching_;
    bool waitForComplete_;
    QSerialPort* serialPort_; // Lives in another thread! Be careful to read and write in proper thread
    QByteArray buffer_; // Be careful to read and write in serial port thread

    bool isInitialized_;
    XmlParser xmlParser_;
    quint32 currBaudrate_;
    QThread* hardwareThread_;
    QJSEngine engine_;
};

#endif
