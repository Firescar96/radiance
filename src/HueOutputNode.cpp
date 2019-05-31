#include "HueOutputNode.h"
#include <iostream>
#include <algorithm>
#include <string.h>

HueOutputNode::HueOutputNode(Context *context, QSize chainSize)
    : SelfTimedReadBackOutputNode(new HueOutputNodePrivate(context, chainSize), context, chainSize, 20) {
    connect(this, &SelfTimedReadBackOutputNode::frame, this, &HueOutputNode::onFrame, Qt::DirectConnection);

    start();
}

HueOutputNode::HueOutputNode(const HueOutputNode &other)
: SelfTimedReadBackOutputNode(other) {
}

HueOutputNode *HueOutputNode::clone() const {
    return new HueOutputNode(*this);
}

QSharedPointer<HueOutputNodePrivate> HueOutputNode::d() const {
    return d_ptr.staticCast<HueOutputNodePrivate>();
}

void HueOutputNode::onFrame(QSize size, QByteArray frame) {
    char header[] = {
            'H', 'u', 'e', 'S', 't', 'r', 'e', 'a', 'm', //protocol
            0x01, 0x00, //version 1.0
            0x01, //sequence number 1
            0x00, 0x00, //reserved
            0x00, //color mode HSV
            0x00, //linear filter
    };

    d()->outPacket->clear();
    d()->outPacket->append(header, sizeof(header));

    // filter the rgba values from radiance into rgb for hue
    for (int i = 0; i < frame.size(); i+=4) {
      char lightID = i/4 +1;
      quint16 R = frame.at(i) * 0xff;
      quint16 G = frame.at(i+1) * 0xff;
      quint16 B = frame.at(i+2) * 0xff;

      char light_stream[] = {0x00, 0x00, lightID};
      d()->outPacket->append(light_stream, sizeof(light_stream));
      d()->outPacket->append(R>>8);
      d()->outPacket->append(R & 0xff);
      d()->outPacket->append(G>>8);
      d()->outPacket->append(G & 0xff);
      d()->outPacket->append(B>>8);
      d()->outPacket->append(B & 0xff);
    }

    d()->encryptedSocket->sendMessage(*d()->outPacket);
}

QString HueOutputNode::typeName() {
    return "HueOutputNode";
}

QJsonObject HueOutputNode::parameters() {
    QMutexLocker locker(&d()->m_stateLock);
    return *d()->m_parameters;
}

QJsonObject HueOutputNode::serialize() {
    QJsonObject o = VideoNode::serialize();
    QJsonObject params = parameters();
    for (QString key : params.keys()) {
      o.insert(key, params.value(key));
    }
    return o;
}

void HueOutputNode::initDataSocket(QJsonObject obj) {

    QString host = obj.value("host").toString();
    QString username = obj.value("username").toString();
    QString secret = obj.value("secret").toString();
    QHostAddress *hostAddress = new QHostAddress(host);

    QUrl url("http://" + host + "/api/" + username + "/groups/" + obj.value("group").toString());

    QNetworkRequest *networkRequest = new QNetworkRequest();
    networkRequest->setUrl(url);

    QNetworkAccessManager *networkManager = new QNetworkAccessManager(this);
    QNetworkReply *reply = networkManager->put(*networkRequest, "{\"stream\": {\"active\":true}}");
    QEventLoop loop;
    connect(reply, SIGNAL(finished()), &loop, SLOT(quit()));
    loop.exec();
    d()->encryptedSocket = new DtlsAssociation(*hostAddress, 2100, username, secret);
    d()->encryptedSocket->startHandshake();
}

VideoNode *HueOutputNode::deserialize(Context *context, QJsonObject obj) {
    std::string swidth = obj.value("width").toString().toUtf8().constData();
    swidth = swidth.compare("") == 0 ? "1" : swidth;

    int width = std::stoi(swidth);

    HueOutputNode *e = new HueOutputNode(context, QSize(width, 1));
    e->initDataSocket(obj);

    e->d()->m_parameters = new QJsonObject({
      {"username", obj.value("username").toString()},
      {"secret", obj.value("secret").toString()},
      {"width", obj.value("width").toString()},
      {"group", obj.value("group").toString()},
      {"host", obj.value("host").toString()},
    });

    return e;
}

bool HueOutputNode::canCreateFromFile(QString filename) {
    return false;
}

VideoNode *HueOutputNode::fromFile(Context *context, QString filename) {
    return nullptr;
}

QMap<QString, QString> HueOutputNode::customInstantiators() {
    auto m = QMap<QString, QString>();
    m.insert("Hue Lightbulb", "HueOutputInstantiator.qml");
    return m;
}

HueOutputNodePrivate::HueOutputNodePrivate(Context *context, QSize chainSize)
: SelfTimedReadBackOutputNodePrivate(context, chainSize)
{
  outPacket = new QByteArray();

  //state that gets (de)serialized on restart
  QJsonObject m_parameters;
}
