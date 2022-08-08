#ifndef SERVER_H
#define SERVER_H

#include <QAbstractItemModel>
#include <QObject>
#include <QTcpServer>
#include <QTcpSocket>
#include <QMap>
#include "myclient.h"

class Server : public QTcpServer
{
    Q_OBJECT

public:
    explicit Server(QObject *parent = nullptr);
private:
    QMap<QString,MyClient*> m_plsocket;

protected:
    void incomingConnection(qintptr handle);
signals:
public slots:
    //void newClientConnect();
};

#endif // SERVER_H
