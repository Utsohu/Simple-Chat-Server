#include "server.h"
#include "mainwindow.h"

Server::Server(QObject *parent)
    : QTcpServer(parent)
{
    listen(QHostAddress::Any,2016);
}

void Server::incomingConnection(qintptr handle){
    MyClient* client = new MyClient(this->parent());
    client->setSocketDescriptor(handle);
    MainWindow* mw = (MainWindow*) this->parent();
    mw->newConnectedClient(client);
    client->ChangeServer(this);
}
