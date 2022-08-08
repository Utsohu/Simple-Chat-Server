#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "server.h"
#include <QDateTime>
#include <QTimer>
#include <QFile>
#include <QCloseEvent>
#include <QSystemTrayIcon>
#include <QAction>
#include <QMenu>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    m_piServer = new Server(this);
    connect(ui->Refresh,SIGNAL(clicked(bool)),this,SLOT(removeClient()));
    //connect(m_piServer,SIGNAL(newConnection()),this,SLOT(newClient()));
    connect(ui->Refresh,SIGNAL(clicked(bool)),this,SLOT(showNames()));
    loadUserPass();


    m_pSystemTrayIcon = new QSystemTrayIcon(this);
    QIcon icon = QIcon(":/image/smallIcon.png");
    m_pSystemTrayIcon->setIcon(icon);
    m_pSystemTrayIcon->setToolTip(QString::fromLocal8Bit("NetServer"));
    connect(m_pSystemTrayIcon,SIGNAL(activated(QSystemTrayIcon::ActivationReason)),this, SLOT(on_activatedSysTrayIcon(QSystemTrayIcon::ActivationReason)));
    createActions();
    createMenu();

    m_pSystemTrayIcon->show();
}

MainWindow::~MainWindow()
{
    saveUserPass();
    delete ui;
}

void MainWindow::showNames(){
    QStringList nameList = m_plsocket.keys();
    ui->ClientName->addItems(nameList);
}

void MainWindow::showMessages(QString ip, QString messages){
    QString message = m_plipUser[ip] + QString::fromLocal8Bit("说：") + messages;
    ui->MessageView->addItem(message);
    for (MyClient* client : m_plsocket){
        if (client->peerAddress().toString()!=ip){
            client->sendMessage(message,0x01);
        }
        else {
            QString extraMessage = QString::fromLocal8Bit("我说：") + messages;
            client->sendMessage(extraMessage,0x01);
        }
    }
}

void MainWindow::newClient(QString ip){
    ui->MessageView->addItem(ip + QString::fromLocal8Bit("已连接"));
    ui->ClientName->clear();
    ui->ClientName->addItem(QString::fromLocal8Bit("目前已连接客户:"));
    showNames();
    /*for (MyClient* client : m_plsocket){
        QString message = m_plipUser[ip] + QString::fromLocal8Bit("已登录");
        client->sendMessage(message,0x01);
        if (client->peerAddress().toString() != ip){
            QString message = ip + QString::fromLocal8Bit("已连接");
            client->sendMessage(message,0x01);
        }
        else{
            if (m_plsocket.size() <= 1){
                QString message = QString::fromLocal8Bit("欢迎连接到此服务器，目前在讨论组里没有其他人");
                client->sendMessage(message,0x01);
            }
            else{
                QString message = QString::fromLocal8Bit("欢迎连接到此服务器，目前在讨论组里的有:");
                for (QString clientIp : m_plsocket.keys()){
                    if (clientIp != ip){
                        message += clientIp;
                        message += " ";
                    }
                }
                client->sendMessage(message,0x01);
            }
        }
    }*/
}

void MainWindow::removeClient(){

}

void MainWindow::clientDisconnect(QString ip){
    QString message = ip + QString::fromLocal8Bit("已断开");
    for (MyClient* client : m_plsocket){
        if (client->peerAddress().toString() != ip){
            client->sendMessage(message,0x01);
        }
    }
    ui->MessageView->addItem(m_plipUser[ip] + QString::fromLocal8Bit("已退出"));
    m_plsocket.erase(m_plsocket.find(ip));
    ui->ClientName->clear();
    ui->ClientName->addItem(QString::fromLocal8Bit("目前已连接客户:"));
    showNames();
}

void MainWindow::newConnectedClient(MyClient* client){
    QString ip = client->peerAddress().toString();
    m_plsocket[ip] = client;
}

void MainWindow::SendAvailableMultiChatUsers(QString ip){
    //if (m_plsocket.size() <= 1) return;
    QString message = "@MultiChatGroupInfo";
    for (QString userIp : m_pluserPass.keys()){
        if (m_plipUser[ip] != userIp){
            message += "@UserName:" + userIp;
        }
    }
    for (MyClient* client : m_plsocket){
        if (client->peerAddress().toString() == ip){
            client->sendMessage(message,0x01);
        }
    }

    QString extraMessage = "@MultiChatGroupAva";
    for (QString names : m_plipUser){
        extraMessage += "@UserName:" + names;
    }
    for (MyClient* client : m_plsocket){
        if (client->peerAddress().toString() == ip){
            client->sendMessage(extraMessage,0x01);
        }
    }
}

void MainWindow::proceedLoginRequest(QString request, QString ip){
    QString userName = request.split("@Username").at(1).split("@UserPass").at(0);
    QString password = request.section("@UserPass",1);
    for (MyClient* client : m_plsocket){
        if (client->peerAddress().toString() == ip){
            if (!m_pluserPass.keys().contains(userName)){
                client->sendMessage("@UserLoginInResultfalse",0x03);
                return;
            }

            if (password != m_pluserPass[userName]){
                client->sendMessage("@UserLoginInResultfalse",0x03);
                return;
            }

            client->sendMessage("@UserLoginInResulttrue",0x03);
            m_plipUser[ip] = userName;
            for (MyClient* client : m_plsocket){
                QString message = userName + QString::fromLocal8Bit("已登录");
                client->sendMessage(message,0x01);
            }
            return;
        }
    }
}

void MainWindow::sendTargetClients(QString ip, QString messages){
    QStringList clientList = messages.split("@NewTargetUser:");
    QString message = m_plipUser[ip] + QString::fromLocal8Bit("悄悄对你说:") + clientList.at(clientList.length()-1);
    for (int i = 1; i<clientList.length()-1; i++){
        QString targetClient = clientList.at(i);
        for (MyClient* client : m_plsocket){
            QString realIp = "";
            for (QString ips : m_plipUser.keys()){
                if (m_plipUser[ips] == targetClient){
                    realIp = ips;
                    break;
                }
            }
            if (client->peerAddress().toString() == realIp){
                client->sendMessage(message,0x01);
            }
        }
    }
    for(MyClient* client : m_plsocket){
        if (client->peerAddress().toString()==ip){
            QString extraMessage = QString::fromLocal8Bit("我说:") + clientList.at(clientList.length()-1);
            client->sendMessage(extraMessage,0x01);
        }
    }
}

void MainWindow::proceedRegisterRequest(QString request, QString ip){
    if (!request.contains("@Password")){
        for (MyClient* client : m_plsocket){
            if (client->peerAddress().toString() == ip){
                client->sendMessage("@UserLoginInResultfalse",0x01);
            }
            return;
        }
    }
    QString username = request.split("@Password").at(0);
    QString password = request.split("@Password").at(1);
    m_pluserPass[username] = password;
    m_plipUser[ip] = username;
    for (MyClient* client : m_plsocket){
        if (client->peerAddress().toString() == ip){
            client->sendMessage("@UserLoginInResulttrue",0x01);
        }
    }
}

void MainWindow::saveUserPass(){
    QFile fileAddress("./userPass.txt");
    if(fileAddress.open(QIODevice::WriteOnly)){
        QDataStream ts(&fileAddress);
        ts << m_pluserPass.size();
        foreach (QString password , m_pluserPass){
            ts << password;
        }
        fileAddress.close();
    }

    QFile nameAddress("./userName.txt");
    if(nameAddress.open(QIODevice::WriteOnly)){
        QDataStream ts(&nameAddress);
        ts << m_pluserPass.keys().size();
        foreach (QString names , m_pluserPass.keys()){
            ts << names;
        }
        nameAddress.close();
    }
}

void MainWindow::loadUserPass(){
    QFile fileAddress("./userName.txt");
    QStringList* names = new QStringList();
    if (fileAddress.open(QIODevice::ReadOnly)){
        QDataStream ts(&fileAddress);
        int size;
        ts >> size;
        for (int index = 0;index<size;index++){
            QString name;
            ts >> name;
            names->append(name);
        }
        fileAddress.close();
     }

    QFile passAddress("./userPass.txt");
    QStringList* passwords = new QStringList();
    if (passAddress.open(QIODevice::ReadOnly)){
        QDataStream ts(&passAddress);
        int size;
        ts >> size;
        for (int index = 0;index<size;index++){
            QString pass;
            ts >> pass;
            passwords->append(pass);
        }
        passAddress.close();
     }

    for (int index = 0; index < names->length();index++){
        m_pluserPass[names->at(index)] = passwords->at(index);
    }
}

void MainWindow::closeEvent(QCloseEvent *event){
    this->hide();
    event->ignore();
}

void MainWindow::on_activatedSysTrayIcon(QSystemTrayIcon::ActivationReason reason){
    switch(reason){
    /*case QSystemTrayIcon::Trigger:
        m_pSystemTrayIcon->showMessage("MessageTitle","Orin",QSystemTrayIcon::Information,1000);
        break;*/
    case QSystemTrayIcon::Trigger:
        this->show();
        break;
    default:
        break;
    }
}

void MainWindow::createActions(){
    mShowMainAction = new QAction(QString::fromLocal8Bit("显示主界面"),this);
    connect(mShowMainAction,SIGNAL(triggered()),this,SLOT(on_showMainAction()));

    mExitAppAction = new QAction(QString::fromLocal8Bit("退出"),this);
    connect(mExitAppAction,SIGNAL(triggered()),this,SLOT(on_exitAppAction()));
}

void MainWindow::createMenu(){
    mMenu = new QMenu(this);
    mMenu->addAction(mShowMainAction);
    mMenu->addSeparator();
    mMenu->addAction(mExitAppAction);
    m_pSystemTrayIcon->setContextMenu(mMenu);
}

void MainWindow::on_showMainAction(){
    this->show();
}

void MainWindow::on_exitAppAction(){
    saveUserPass();
    exit(0);
}
