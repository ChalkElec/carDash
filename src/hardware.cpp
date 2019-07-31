#include <QSerialPort>
#include <QTime>
#include <QTimer>
#include <QDebug>
#include <QJSValue>
#include <QThread>

#include "hardware.h"
#include "defines.h"

Hardware::Hardware(const QString& xmlConfig, QObject* parent)
    : QObject(parent)
    , isSearching_(false)
    , waitForComplete_(false)
    , serialPort_(nullptr)
    , isInitialized_(false)
    , currBaudrate_(0)
    , hardwareThread_(new QThread(this))
{
    xmlParser_.process(xmlConfig);
    xmlParser_.printAll();

    // Do not move it to another thread, instead create it in another thread
    connect(hardwareThread_, &QThread::started, [=] {
        Q_ASSERT(QThread::currentThread() == hardwareThread_);
        serialPort_ = new QSerialPort(); // can not make a child of a parent from
        // another thread
    });
    connect(hardwareThread_, &QThread::finished, [=] {
        Q_ASSERT(QThread::currentThread() == hardwareThread_);
        serialPort_->deleteLater();
        serialPort_ = NULL;
    });
    hardwareThread_->setObjectName("HardwareThread");
    hardwareThread_->start();
}

Hardware::~Hardware() { hardwareThread_ = nullptr; }

// Find current baudrate of adapter
bool Hardware::findBaudrate()
{
    const quint32 tryBaudrates[] = { 2000000, 115200, 38400, 9600, 230400,
        57600, 19200, 1000000, 500000 };
    // Try different baudrates to find correct one
    quint32 baud = 0;
    for (auto b : tryBaudrates) {
        serialPort_->setBaudRate(b);
        LOGD << "Trying baudrate:" << b;
        // Send no-meaning command to check for answer
        QByteArray r = sendCmdSync("\x7F\x7F\r", 1000);
        if (r.endsWith('>')) {
            baud = b;
            break;
        }
    }
    if (baud) {
        LOGD << "Connected with baudrate: " << baud;
        currBaudrate_ = baud;
        return true;
    }
    LOGW << "Can't find correct baudrate";
    return false;
}

// Set maximal baudrate that is supported by adapter
bool Hardware::setMaxBaudrate()
{
    LOGD;

    const quint32 setBaudrates[] = { 2000000, 1000000, 500000, 230400, 115200 };

    if (currBaudrate_ == setBaudrates[0])
        return true;

    qint32 savedBaudrate = serialPort_->baudRate();
    // Increase baudrate to maximum (try 2 000 000 bps first, then go down to 115
    // 200)
    QByteArray idString = sendCmdSync("ATI");
    LOGD << "Adapter ID:" << idString;
    if (idString.startsWith("ATI"))
        idString = idString.mid(3);

    QByteArray r = sendCmdSync("ATBRT20");
    if (!r.contains("OK")) {
        LOGD << "ATBRT command is not supported";
        return false;
    }

    for (auto s : setBaudrates) {
        serialPort_->setBaudRate(savedBaudrate);

        LOGD << "Trying to set baudrate:" << s;
        QByteArray r = sendCmdSync(QString("ATBRD%1")
                                       .arg(qRound(4000000.0 / s), 2, 16, QChar('0'))
                                       .toLatin1(),
            100);
        if (!r.contains("OK")) {
            LOGD << "Didn't get OK";
            continue;
        }
        else
            LOGD << "Got first reply:" << r;

        r.clear();
        // Now switch serial baudrate to new one and wait for id String
        serialPort_->setBaudRate(s);
        LOGD << "Interface baudrate is" << serialPort_->baudRate();

        if (serialPort_->waitForReadyRead(100)) {
            r.append(serialPort_->readAll());
            LOGD << "Got second reply:" << r;
        }
        else
            LOGW << "Did not get second reply";

        if (r.contains(idString)) {
            // We got correct reply, so lets confirm this baudrate by sending empty
            // '\r' to scanner
            sendCmdSync("");
            LOGD << "New (maximum) baudrate is " << serialPort_->baudRate();
            currBaudrate_ = s;
            return true;
        }
    }
    return false;
}

// Open serial port
void Hardware::open(const QString& port)
{
    isSearching_ = false;
    waitForComplete_ = false;

    if (isInitialized_) {
        isInitialized_ = false; // not using setIsInitialized to not force splash screen again
        QTimer::singleShot(0, serialPort_, [=] { serialPort_->close(); });
    }

    LOGD << "Opening serial port ..." << port;

    QTimer::singleShot(0, serialPort_, [=] {
        Q_ASSERT(QThread::currentThread() == hardwareThread_);

        buffer_.clear();

        serialPort_->setPortName(port);
        serialPort_->setDataBits(QSerialPort::Data8);
        serialPort_->setParity(QSerialPort::NoParity);
        serialPort_->setStopBits(QSerialPort::OneStop);
        serialPort_->setFlowControl(QSerialPort::NoFlowControl);
        serialPort_->setBaudRate(2000000);

        if (!serialPort_->open(QIODevice::ReadWrite)) {
            LOGD << "Failed to open serial port!";
            emit initFinished();
            return;
        }
        else
            LOGD << "Serial port is opened";

        // Find current baudrate of adapter
        if (!findBaudrate()) {
            emit initFinished();
            return;
        }

        // Set maximal baudarate
        if (!setMaxBaudrate()) {
            LOGW << "Failed to set max baud rate, continue with current";
            // return;
        }

        LOGD << "Initializing ELM327 ...";
        init();

        emit initFinished();
    });
}

// Close serial port
void Hardware::close()
{
    QTimer::singleShot(0, serialPort_, [=] {
        if (serialPort_->isOpen()) {
            disconnect(serialPort_);
            serialPort_->close();
        }
    });
    isInitialized_ = false;
}

// Check that ELM327 is connected
bool Hardware::init()
{
    Q_ASSERT(QThread::currentThread() == hardwareThread_);

    if (!serialPort_->isOpen())
        return false;

    const auto& cmdList = xmlParser_.initCommands();
    for (auto& c : cmdList) {
        QByteArray r = sendCmdSync(c.toLatin1(), 300);
        LOGD << ">" << r.simplified();
        if (!r.contains("OK")) {
            LOGW << "Error: closing port";
            serialPort_->close();
            return false;
        }
    }

    LOGD << "ELM327 is detected";
    connect(serialPort_, SIGNAL(readyRead()), SLOT(readData()),
        Qt::DirectConnection);

    setInitialized(true);

    return true;
}

bool Hardware::sendCmdAsync(const QByteArray& cmd, bool waitForComplete)
{
    if (!isInitialized_) {
        return false;
    }

    if (isSearching_) {
        LOGD << "searching";
        return false;
    }
    if (waitForComplete_) {
        LOGD << "waitForComplete";
        return false;
    }
    waitForComplete_ = waitForComplete;

    QTimer::singleShot(0, serialPort_, [=] {
        Q_ASSERT(QThread::currentThread() == hardwareThread_);
        LOGD << QDateTime::currentDateTime().toString("hh:mm:ss.zzz")
                 << "- sent data:" << cmd;
        QByteArray finalCmd = cmd.endsWith(0x0D) ? cmd : cmd + char(0x0D);
        serialPort_->write(finalCmd);
    });
    return true;
}

// Send AT command to ELM327
QByteArray Hardware::sendCmdSync(const QByteArray& cmd, int timeout)
{
    Q_ASSERT(QThread::currentThread() == hardwareThread_);

    LOGD << cmd << timeout;

    QByteArray r;

    if (waitForComplete_) {
        LOGD << "Previous command has not finished yet";
        return r;
    }

    if (serialPort_->isOpen()) {
        serialPort_->blockSignals(true);
        QByteArray finalCmd = cmd.endsWith(0x0D) ? cmd : cmd + char(0x0D);

        serialPort_->write(finalCmd);
        serialPort_->flush();

        while (true) {
            bool ready = serialPort_->waitForReadyRead(timeout);
            if (!ready) {
                LOGW << "No reply from serial port within a timeout:" << timeout;
                break;
            }

            QByteArray data = serialPort_->readAll();
            if (data.contains("SEARCHING")) {
                isSearching_ = true;
                continue;
            }

            isSearching_ = false;

            r.append(data);

            if (data.endsWith(">"))
                break;
        }

        serialPort_->blockSignals(false);
    }
    else
        LOGW << "Serial port is not open!";

    return r;
}

void Hardware::readData()
{
    Q_ASSERT(QThread::currentThread() == hardwareThread_);

    processData(serialPort_->readAll());
}

void Hardware::processData(const QByteArray& data)
{
    if (data.isEmpty())
        return;

    buffer_.append(data);
    if (waitForComplete_) {
        if (!buffer_.endsWith(">"))
            return;
        else
            waitForComplete_ = false;
    }
    LOGD << QDateTime::currentDateTime().toString("hh:mm:ss.zzz")
             << "- received data:" << buffer_;

    while (buffer_.contains(0x0D)) {
        int i = buffer_.indexOf(0x0D);
        QByteArray d = buffer_.left(i);
        QByteArray str = d.replace("\r", "")
                             .replace("\n", "")
                             .replace(" ", "")
                             .replace(">", "")
                             .trimmed();
        if (!str.isEmpty())
            processPacket(str);
        buffer_ = buffer_.mid(i + 1);
    }
}

void Hardware::processPacket(const QByteArray& str)
{
    Q_ASSERT(QThread::currentThread() == hardwareThread_);

    LOGD << "Raw data received:" << str;

    if (str.contains("SEARCHING")) {
        isSearching_ = true;
        return;
    }

    isSearching_ = false;

    if (!str.startsWith("41") || str.length() < 6)
        return;

    QByteArray pid = str.mid(2, 2).toLower();
    // LOGD << "Pid:" << pid;

    // ACCESSING parser which is in another thread, but since we are not messing
    // with it, I think it is OK
    auto& cmdList = xmlParser_.commands();
    for (auto& c : cmdList) {
        QString cmdPIDShort = c->send.toLower();
        if (cmdPIDShort.startsWith("01") && cmdPIDShort.length() > 2)
            cmdPIDShort.remove("01");

        if (pid == cmdPIDShort) {
            QByteArray val = str.mid(
                4, c->replyLength * 2); // * 2 because 1 byte takes 2 characters

            if (!(c->conversion.isEmpty())) {
                // Conversion example: (B0*256+B1)/32768*14.7
                QStringList possibleParams{ "B0", "B1", "B2", "B3", "V" };
                QMap<QString, int> convArgs;

                for (auto p : possibleParams) {
                    if (c->conversion.contains(p)) {
                        bool ok = false;
                        int byteNumber = p.right(1).toInt(&ok);

                        // In case of XML config error or V, index will be out of range, we
                        // take
                        // values as is.
                        if (!ok || byteNumber < 0 || byteNumber >= val.length())
                            convArgs.insert(p, val.toInt(NULL, 16));
                        else // B0-B3
                            convArgs.insert(p, val.mid(byteNumber * 2, 2).toInt(NULL, 16));
                    }
                }

                QString functionTemplate = "(function(%1) { return %2; })";
                QString funcStr = functionTemplate.arg(convArgs.keys().join(",")).arg(c->conversion);

                // LOGD << "funcStr:" << funcStr << ", args:" << convArgs.values();

                QJSValue jsFunc = engine_.evaluate(funcStr);
                QJSValueList args;
                for (auto v : convArgs.values())
                    args << v;

                QJSValue finalValue = jsFunc.call(args);

                // LOGD << "Calculated result:" << finalValue.toVariant();

                // I don't pass QJSValue directly, because it seems to get destroyed at
                // times
                // before it reaches QML handler, so receiving undefined instead.
                // That is mostlikely caused by passing QJSValue to QML from a different
                // thread
                emit dataReceived(c->name, finalValue.toVariant());
            }
            else {
                LOGD << "Conversion is empty for:" << pid;
                emit dataReceived(c->name, val.toInt(NULL, 16));
            }
            break;
        }
        // else
        //     LOGD << "Cmd send:" << c->send << "not equal to:" << pid;
    }
}

const XmlParser& Hardware::parser() const { return xmlParser_; }

void Hardware::setInitialized(bool isInitialized)
{
    LOGD << isInitialized;

    if (isInitialized_ == isInitialized)
        return;

    isInitialized_ = isInitialized;
    emit isInitializedChanged();
}

bool Hardware::isInitialized() const { return isInitialized_; }

bool Hardware::isReadyForSending() const {
    return isInitialized_ && !isSearching_ && !waitForComplete_;
}

const QObject* Hardware::workerThreadObject() const { return serialPort_; }
