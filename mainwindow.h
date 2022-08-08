#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSystemTrayIcon>
#include "server.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    void showMessages(QString ip, QString messages);
    void newClient(QString ip);
    void clientDisconnect(QString ip);
    void newConnectedClient(MyClient* client);
    void sendTargetClients(QString ip, QString messages);
    void SendAvailableMultiChatUsers(QString ip);
    void proceedLoginRequest(QString request, QString ip);
    void proceedRegisterRequest(QString request, QString ip);

private:
    Ui::MainWindow *ui;
    Server* m_piServer;
    QSystemTrayIcon* m_pSystemTrayIcon;
    QMap<QString,MyClient*> m_plsocket;
    QMap<QString, QString> m_plipUser;
    QMap<QString,QString> m_pluserPass;
    QAction* mShowMainAction;
    QAction* mExitAppAction;
    QMenu* mMenu;
    void loadUserPass();
    void saveUserPass();
    void createActions();
    void createMenu();

protected:
    void closeEvent(QCloseEvent *event);

public slots:
    void showNames();
    void removeClient();
    void on_activatedSysTrayIcon(QSystemTrayIcon::ActivationReason reason);
    void on_showMainAction();
    void on_exitAppAction();
};

#endif // MAINWINDOW_H
