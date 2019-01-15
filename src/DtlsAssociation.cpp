#include "DtlsAssociation.h"

DtlsAssociation::DtlsAssociation(QHostAddress &address, quint16 port,
                                 QString &connectionName, QString connectionSecret)
    : name(connectionName),secret(connectionSecret),
      crypto(QSslSocket::SslClientMode)
{
    auto configuration = QSslConfiguration::defaultDtlsConfiguration();
    configuration.setPeerVerifyMode(QSslSocket::VerifyNone);
    crypto.setPeer(address, port);
    crypto.setDtlsConfiguration(configuration);

    connect(&crypto, &QDtls::handshakeTimeout, this, &DtlsAssociation::handshakeTimeout);
    connect(&crypto, &QDtls::pskRequired, this, &DtlsAssociation::pskRequired);
    socket.connectToHost(address.toString(), port);
    connect(&socket, &QUdpSocket::readyRead, this, &DtlsAssociation::readyRead);
    // pingTimer.setInterval(5000);
    // connect(&pingTimer, &QTimer::timeout, this, &DtlsAssociation::pingTimeout);
}

DtlsAssociation::~DtlsAssociation()
{
    if (crypto.isConnectionEncrypted())
        crypto.shutdown(&socket);
}

void DtlsAssociation::startHandshake()
{
    if (socket.state() != QAbstractSocket::ConnectedState) {
        qInfo() << tr("%1: connecting UDP socket first ...").arg(name);
        connect(&socket, &QAbstractSocket::connected, this, &DtlsAssociation::udpSocketConnected);
        return;
    }

    if (!crypto.doHandshake(&socket))
        qInfo() << tr("%1: failed to start a handshake - %2").arg(name, crypto.dtlsErrorString());
    else
      qInfo() << tr("%1: starting a handshake").arg(name);

      //*****************CALL THE ACTIVE ROUTE BEFORESTARTINGHANDSHAKE*******************//
      //*****************CALL TODO
}

void DtlsAssociation::sendMessage(QByteArray data) {
  crypto.writeDatagramEncrypted(&socket, data);
}

void DtlsAssociation::udpSocketConnected()
{
    qInfo() << tr("%1: UDP socket is now in ConnectedState, continue with handshake ...").arg(name);
    startHandshake();
}

void DtlsAssociation::readyRead()
{
    QByteArray dgram(socket.pendingDatagramSize(), Qt::Uninitialized);
    const qint64 bytesRead = socket.readDatagram(dgram.data(), dgram.size());
    if (bytesRead <= 0) {
        qInfo() << tr("%1: spurious read notification?").arg(name);
        return;
    }

    dgram.resize(bytesRead);
    if (crypto.isConnectionEncrypted()) {
        const QByteArray plainText = crypto.decryptDatagram(&socket, dgram);
        if (plainText.size()) {
            qInfo() << name << dgram << plainText;
            return;
        }

        if (crypto.dtlsError() == QDtlsError::RemoteClosedConnectionError) {
            qInfo() << tr("%1: shutdown alert received").arg(name);
            socket.close();
            pingTimer.stop();
            return;
        }

        qInfo() << tr("%1: zero-length datagram received?").arg(name);
    } else {
        if (!crypto.doHandshake(&socket, dgram)) {
            qInfo() << tr("%1: handshake error - %2").arg(name, crypto.dtlsErrorString());
            return;
        }

        if (crypto.isConnectionEncrypted()) {
            qInfo() << tr("%1: encrypted connection established!").arg(name);
            pingTimer.start();
            pingTimeout();
        } else {
          qInfo() << tr("%1: continuing with handshake ...").arg(name);
        }
    }
}

void DtlsAssociation::handshakeTimeout()
{
    qInfo() << tr("%1: handshake timeout, trying to re-transmit").arg(name);
    if (!crypto.handleTimeout(&socket))
        qInfo() << tr("%1: failed to re-transmit - %2").arg(name, crypto.dtlsErrorString());
}

void DtlsAssociation::pskRequired(QSslPreSharedKeyAuthenticator *auth)
{
    Q_ASSERT(auth);

    qInfo() << tr("%1: providing pre-shared key ...").arg(name);
    auth->setIdentity(name.toLatin1());
    auth->setPreSharedKey(QByteArray::fromHex(secret.toUtf8()));
}

void DtlsAssociation::pingTimeout()
{
    static const QString message = QStringLiteral("I am %1, please, accept our ping %2");
    const qint64 written = crypto.writeDatagramEncrypted(&socket, message.arg(name).arg(ping).toLatin1());
    if (written <= 0) {
        emit errorMessage(tr("%1: failed to send a ping - %2").arg(name, crypto.dtlsErrorString()));
        pingTimer.stop();
        return;
    }

    ++ping;
}
