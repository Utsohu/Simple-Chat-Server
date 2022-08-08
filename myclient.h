#ifndef MYCLIENT_H
#define MYCLIENT_H

#include <QTcpSocket>
#include <QTcpServer>


class MyClient : public QTcpSocket
{
    Q_OBJECT
public:
    MyClient(QObject *parent = nullptr);
    void ChangeServer(QTcpServer* newServer);
    void sendMessage(QString message,char type);
    QString username;
    bool m_bNeedClose;
private:
    void checkReceive();
private:
    QTcpServer* server;
    QString m_ipAddress;
    QByteArray m_writeBuf;
    QByteArray m_readBuf;
private slots:
    void readData();
    void sendData();
    void disconnect();
    void clientConnected();
    void closeClient();
};

#endif // MYCLIENT_H
