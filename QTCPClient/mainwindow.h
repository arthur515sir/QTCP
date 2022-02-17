#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QAbstractSocket>
#include <QDebug>
#include <QFile>
#include <QFileDialog>
#include <QHostAddress>
#include <QMessageBox>
#include <QMetaType>
#include <QString>
#include <QStandardPaths>
#include <QTcpSocket>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    int count_send=0;
    ~MainWindow();
signals:
    void newMessage(QString);
private slots:
    void readSocket();
    void discardSocket();
    void displayError(QAbstractSocket::SocketError socketError);

    void displayMessage(const QString& str);
    void on_pushButton_sendMessage_clicked();
    //void on_pushButton_sendMessage_clicked_2();
    void on_pushButton_sendAttachment_clicked();
    void on_pushButton_sendMessage_2_clicked();

    void on_pushButton_sendMessage_3_clicked();

    void on_pushButton_sendMessage_4_clicked();

    void on_pushButton_sendMessage_6_clicked();

    void on_pushButton_sendMessage_5_clicked();

    void on_pushButton_sendMessage_7_clicked();

private:
    Ui::MainWindow *ui;

    QTcpSocket* socket;
};

#endif // MAINWINDOW_H
