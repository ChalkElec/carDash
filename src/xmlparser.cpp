#include "QDebug"
#include "QFile"
#include "QXmlStreamReader"

#include "xmlparser.h"
#include "defines.h"

XmlParser::XmlParser()
{
    init_.clear();
    qDeleteAll(commands_);
    commands_.clear();
}

void XmlParser::printAll()
{
    LOGD << "Init section:";
    for (auto i : init_)
        LOGD << i;

    LOGD << "Rotation section:";
    for (auto c : commands_)
        LOGD << c->name << c->send << c->replyLength << c->skipCount << c->curCount << c->conversion << c->units;
}

// Parse init section of XML file
void XmlParser::parseInit()
{
    while (xml_.readNextStartElement()) {
        if (xml_.name() == "command") {
            for (auto attr : xml_.attributes()) {
                if (attr.name().toString() == "send")
                    init_.append(attr.value().toString());
            }
            xml_.readNext();
        }
        else
            break;
    }
}

// Parse rotation section of XML file
void XmlParser::parseRotation()
{
    while (xml_.readNextStartElement()) {
        if (xml_.name() == "command") {
            Command* c = new Command();
            c->skipCount = 0;
            c->curCount = 0;
            c->replyLength = 0;
            for (auto attr : xml_.attributes()) {
                if (attr.name().toString() == "name")
                    c->name = attr.value().toString();
                else if (attr.name().toString() == "targetId")
                    c->name = attr.value().toString();
                else if (attr.name().toString() == "send")
                    c->send = attr.value().toLatin1();
                else if (attr.name().toString() == "conversion")
                    c->conversion = attr.value().toString();
                else if (attr.name().toString() == "units")
                    c->units = attr.value().toString();
                else if (attr.name().toString() == "replyLength")
                    c->replyLength = attr.value().toInt();
                else if (attr.name().toString() == "skipCount")
                    c->skipCount = attr.value().toInt();
            }
            commands_.append(c);
            xml_.readNext();
        }
        else
            break;
    }
}

// Parse XML file, and place result in 2 lists: init_ and commands_
bool XmlParser::process(const QString& fileName)
{
    init_.clear();
    qDeleteAll(commands_);
    commands_.clear();

    QFile* file = new QFile(fileName);
    if (!file->open(QIODevice::ReadOnly | QIODevice::Text)) {
        LOGD << "Can't open XML config file:" << fileName;
        return false;
    }

    xml_.clear();
    xml_.setDevice(file);

    if (xml_.readNextStartElement()) {
        if (xml_.name() == "OBD2") {
            while (xml_.readNextStartElement()) {
                if (xml_.name() == "init")
                    parseInit();
                else if (xml_.name() == "rotation")
                    parseRotation();
                else
                    xml_.skipCurrentElement();
            }
        }
        else {
            LOGD << "Unknown tag in XML config file:" << xml_.name();
            return false;
        }
    }
    else if (xml_.hasError()) {
        LOGD << "Error in XML config file:" << xml_.errorString();
        return false;
    }
    return true;
}

const QList<Command*>& XmlParser::commands() const
{
    return commands_;
}

const QList<QString>& XmlParser::initCommands() const
{
    return init_;
}
