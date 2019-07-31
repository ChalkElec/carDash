#ifndef XMLPARSER_H
#define XMLPARSER_H

#include <QList>
#include <QXmlStreamReader>

typedef struct{
    QString name;           // Name of Cardash widget that will show/handle this parameter
    QByteArray send;        // Command to send to OBD2
    int replyLength;        // Expected # of bytes in reply
    int skipCount;          // Number of skips in rotations for this value
    int curCount;           // Current counter of rotation
    QString conversion;     // Conversion of result value (B0-B3 for bytes, V for full value, + - * / << >> and constants are allowed)
    QString units;          // Units of result value ("C","F","km/h","mph","bar","psi","bit", for "bit" we check lowest bit)
} Command;



class XmlParser {

public:
    XmlParser();

    bool process(const QString& fileName);
    void printAll();

    const QList<QString>& initCommands() const;
    const QList<Command*>& commands() const;

private: // methods
    void parseInit();
    void parseRotation();

private:
    QList<Command*> commands_;
    QList<QString> init_;
    QXmlStreamReader xml_;
};

#endif // XMLPARSER_H
