#include "myclient.h"
#include <QTcpServer>
#include <QByteArray>
#include "mainwindow.h"

#define TYPE_MSG    0x01
#define TYPE_PIC    0x02
#define TYPE_FILE   0x03
#define MSG_PUBLIC  0x01
#define MSG_PRIVATE_GROUP 0x02
#define MSG_PRIVATE_INDIVIDUAL 0x03

MyClient::MyClient(QObject *parent):QTcpSocket(parent)
{

}

void MyClient::ChangeServer(QTcpServer* newServer){
    server = newServer;
    MainWindow* mw = (MainWindow* ) this->parent();
    QString ip = this->peerAddress().toString();
    mw->newClient(ip);
    connect(this,SIGNAL(readyRead()),SLOT(readData()));
    connect(this,SIGNAL(disconnected()),SLOT(disconnect()));
    connect(this,SIGNAL(bytesWritten(qint64)),SLOT(sendData()));
    m_ipAddress = ip;
}

void MyClient::sendMessage(QString message, char type){
    QByteArray msg = message.toUtf8();
    QByteArray data;
    data.append(0x1a);
    data.append(0x1b);
    data.append(0x1f);
    data.append(0x1e);
    ushort len=msg.length();
    data.append(len%256);
    data.append(len/256);
    data.append(TYPE_MSG);
    data.append(type);
    m_writeBuf.append(data);
    m_writeBuf.append(msg);
    sendData();
}

void MyClient::readData(){
    while(true){
        QByteArray readBuf = this->readAll();
        if(readBuf.length() <=0) break;
        m_readBuf.append(readBuf);
        checkReceive();
    }
}

void MyClient::checkReceive(){
    int bufLength = m_readBuf.length();
    if (bufLength<8) return;
    if (m_readBuf.at(0) != 0x1a || m_readBuf.at(1) != 0x1b || m_readBuf.at(2) != 0x1f || m_readBuf.at(3) != 0x1e){
        m_readBuf.clear();
        return;
    }
    int msgLength = m_readBuf.at(4) + m_readBuf.at(5)*256;
    if (bufLength < msgLength+8) return;
    QByteArray trueMsg = m_readBuf.mid(8,msgLength);
    char msgType = m_readBuf.at(6);
    char contentType = m_readBuf.at(7);
    if (msgType == TYPE_MSG){
        MainWindow* mw = (MainWindow* ) this->parent();
        QString ip = this->peerAddress().toString();
        QString messages = "";
        messages.prepend(trueMsg);
        if (contentType == 0x01) mw->showMessages(ip,messages);
        else if (contentType == 0x02) mw->sendTargetClients(ip,messages);
        else if (contentType == 0x03) mw->proceedLoginRequest(messages,ip);
        else if (contentType == 0x04) mw->SendAvailableMultiChatUsers(ip);
        else if (contentType == 0x05) mw->proceedRegisterRequest(messages,ip);
    }
    else if (msgType == TYPE_PIC) {

    }
    else if (msgType == TYPE_FILE) {

    }
    m_readBuf = m_readBuf.mid(msgLength+8);
    checkReceive();
}

void MyClient::sendData(){
    if (m_writeBuf.isEmpty()) return;
    int writeLength = write(m_writeBuf);
    m_writeBuf = m_writeBuf.mid(writeLength);
}

void MyClient::disconnect(){
    MainWindow* mw = (MainWindow*) this->parent();
    mw->clientDisconnect(m_ipAddress);
    this->deleteLater();
}


void MyClient::clientConnected(){
    MainWindow* mw = (MainWindow*) this->parent();
    QString ip = this->peerAddress().toString();
    mw->newClient(ip);
}

void MyClient::closeClient(){
    this->close();
}
