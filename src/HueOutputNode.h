#pragma once

#include "SelfTimedReadBackOutputNode.h"
#include "DtlsAssociation.h"
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>
#include <string.h>
#include <QHostAddress>
#include <QUdpSocket>
#include <QHttpMultiPart>
#include <QHttpPart>
#include <QString>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkReply>

class HueOutputNodePrivate;

class HueOutputNode
    : public SelfTimedReadBackOutputNode {
    Q_OBJECT

public:
    HueOutputNode(Context *context, QSize chainSize);
    HueOutputNode(const HueOutputNode &other);
    HueOutputNode *clone() const override;
    void onFrame(QSize size, QByteArray frame);
    void initDataSocket(QJsonObject obj);

    // These static methods are required for VideoNode creation
    // through the registry

    // A string representation of this VideoNode type
    static QString typeName();
    QJsonObject parameters();

    //return the state of this node in the json format
    QJsonObject serialize() override;

    // Create a VideoNode from a JSON description of one
    // Returns nullptr if the description is invalid
    static VideoNode *deserialize(Context *context, QJsonObject obj);

    // Return true if a VideoNode could be created from
    // the given filename
    // This check should be very quick.
    static bool canCreateFromFile(QString filename);

    // Create a VideoNode from a filename
    // Returns nullptr if a VideoNode cannot be create from the given filename
    static VideoNode *fromFile(Context *context, QString filename);

    // Returns QML filenames that can be loaded
    // to instantiate custom instances of this VideoNode
    static QMap<QString, QString> customInstantiators();
private:
    QSharedPointer<HueOutputNodePrivate> d() const;
    HueOutputNode(QSharedPointer<HueOutputNodePrivate> other_ptr);
    QJsonObject generateHSVObject(float r, float g, float b);
};

class HueOutputNodePrivate : public SelfTimedReadBackOutputNodePrivate {
public:
    HueOutputNodePrivate(Context *context, QSize chainSize);
    // Reusable state for sending data packets
    QByteArray *outPacket;
    //handler for sending messages
    DtlsAssociation *encryptedSocket;
    //state that gets (de)serialized on restart
    QJsonObject *m_parameters;
};
