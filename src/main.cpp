#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QTimer>
#include <QQmlContext>
#include <QCursor>
#include <QSerialPortInfo>
#include <QSettings>
#include <QDebug>
#include <QThread>

#include "hardware.h"
#include "xmlparser.h"
#include "defines.h"

int main(int argc, char* argv[])
{
    if (argc < 2) {
        qDebug() << "Usage: ./carDash <path/to/commands_config.xml> <serial_port_optional>\n"
             "Example: ./carDash ../config/obd2.xml /dev/ttyUSB0";
        return EXIT_FAILURE;
    }

    QGuiApplication app(argc, argv);
    app.setOverrideCursor(QCursor(Qt::BlankCursor));

    Hardware hw(argv[1]);

    // Load QML first with splashscreen while we are loading
    QQmlApplicationEngine engine;
    engine.rootContext()->setContextProperty("HardwareClass", &hw);
    engine.load(QUrl(QStringLiteral("qrc:/qml/main.qml")));

    QString port;
    QSettings sett("ChalkElec", "CarDash");
    QList<QSerialPortInfo> portList = QSerialPortInfo::availablePorts();

    // Find correct serial port of OBD2 scanner
    if (app.arguments().count() > 2) {
        port = app.arguments().at(2);
        LOGD << "Using command-line port" << port;
    }
    else if (!sett.value("port").toString().isEmpty()) {
        port = sett.value("port").toString();
        LOGD << "Using saved port" << port;
    }
    else if (!portList.isEmpty()) {
        for (QSerialPortInfo p : portList) {
            QString vidpid = "VID:PID=";
            port = p.systemLocation();
            if (p.hasVendorIdentifier())
                vidpid += QString("%1:").arg(p.vendorIdentifier(), 4, 16);
            else
                vidpid += "0:";
            if (p.hasProductIdentifier())
                vidpid += QString("%1").arg(p.productIdentifier(), 4, 16);
            else
                vidpid += "0";
            LOGD << "Found serial port:" << port << "," << vidpid;
        }
        LOGD << "Using scanned port" << port;
    }
    else {
        qFatal("OBD2 scanner is not detected!");
    }

    QObject::connect(&hw, &Hardware::initFinished, [&] {
        if (hw.isInitialized()) {
            sett.setValue("port", port);

            // Run cycle to send commands to controller in the proper thread
            QTimer::singleShot(0, const_cast<QObject*>(hw.workerThreadObject()), [&] {
                Q_ASSERT(QThread::currentThread()->objectName() == "HardwareThread");

                if (hw.isReadyForSending()) {
                    auto& cmdList = hw.parser().commands();
                    while (true) {
                        for (auto c : cmdList) {
                            // If skipCount is 0 (by default) we will process the command each time
                            if (++(c->curCount) >= c->skipCount) {
                                c->curCount = 0;

                                LOGD << "Sending command:" << c->send;
                                QByteArray reply = hw.sendCmdSync(c->send + char(0x0D), 10000); // First command takes long for me so timeout is high
                                hw.processData(reply);
                            }
                            // else
                            //     LOGD << "Skipping count for" << c->name;
                        }
                    }
                }
                else {
                    LOGW << "HW is not ready for sending";
                }
            });
        }
        else
            qFatal("Can't open serial port of OBD2 scanner!");
    });

    // Open serial port of OBD2
    hw.open(port);

    return app.exec();
}
