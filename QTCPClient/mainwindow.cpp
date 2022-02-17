#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <qdebug.h>
MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->pushButton_sendMessage_2->hide();
    ui->pushButton_sendMessage_3->hide();
    ui->label_4->hide();
    ui->label_5->hide();
    ui->lineEdit_message_4->hide();
    ui->lineEdit_message_5->hide();
    ui->label_3->hide();
    ui->lineEdit_message_3->hide();
    ui->pushButton_sendMessage_4->hide();
    ui->pushButton_sendMessage_5->hide();
    ui->pushButton_sendMessage_6->hide();
    ui->txtbro->hide();
    ui->label_6->hide();
    ui->lineEdit_message_6->hide();
    ui->pushButton_sendMessage_7->hide();

    socket = new QTcpSocket(this);

    connect(this, &MainWindow::newMessage, this, &MainWindow::displayMessage);
    connect(socket, &QTcpSocket::readyRead, this, &MainWindow::readSocket);
    connect(socket, &QTcpSocket::disconnected, this, &MainWindow::discardSocket);
    //connect(socket, &QAbstractSocket::errorOccurred, this, &MainWindow::displayError);

    socket->connectToHost(QHostAddress::LocalHost,8080);

    if(socket->waitForConnected())
        ui->statusBar->showMessage("Connected to Server");
    else{
        QMessageBox::critical(this,"QTCPClient", QString("The following error occurred: %1.").arg(socket->errorString()));
        exit(EXIT_FAILURE);
    }
}

MainWindow::~MainWindow()
{
    if(socket->isOpen())
        socket->close();
    delete ui;
}

void MainWindow::readSocket()
{
    QByteArray buffer;

    QDataStream socketStream(socket);
    socketStream.setVersion(QDataStream::Qt_5_7);

    socketStream.startTransaction();
    socketStream >> buffer;

    if(!socketStream.commitTransaction())
    {
        QString message = QString("%1 :: Waiting for more data to come..").arg(socket->socketDescriptor());
        emit newMessage(message);
        return;
    }

    QString header = buffer.mid(0,128);
    QString fileType = header.split(",")[0].split(":")[1];

    buffer = buffer.mid(128);

    if(fileType=="attachment"){
        QString fileName = header.split(",")[1].split(":")[1];
        QString ext = fileName.split(".")[1];
        QString size = header.split(",")[2].split(":")[1].split(";")[0];

        if (QMessageBox::Yes == QMessageBox::question(this, "QTCPServer", QString("You are receiving an attachment from sd:%1 of size: %2 bytes, called %3. Do you want to accept it?").arg(socket->socketDescriptor()).arg(size).arg(fileName)))
        {
            QString filePath = QFileDialog::getSaveFileName(this, tr("Save File"), QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation)+"/"+fileName, QString("File (*.%1)").arg(ext));

            QFile file(filePath);
            if(file.open(QIODevice::WriteOnly)){
                file.write(buffer);
                QString message = QString("INFO :: Attachment from sd:%1 successfully stored on disk under the path %2").arg(socket->socketDescriptor()).arg(QString(filePath));
                emit newMessage(message);
            }else
                QMessageBox::critical(this,"QTCPServer", "An error occurred while trying to write the attachment.");
        }else{
            QString message = QString("INFO :: Attachment from sd:%1 discarded").arg(socket->socketDescriptor());
            emit newMessage(message);
        }
    }else if(fileType=="message"){
        QString message = QString("%1 :: %2").arg(socket->socketDescriptor()).arg(QString::fromStdString(buffer.toStdString()));
        QString message_2=QString("%1").arg(QString::fromStdString(buffer.toStdString()));
        emit newMessage(message_2);
    }
}

void MainWindow::discardSocket()
{
    socket->deleteLater();
    socket=nullptr;

    ui->statusBar->showMessage("Disconnected!");
}

void MainWindow::displayError(QAbstractSocket::SocketError socketError)
{
    switch (socketError) {
        case QAbstractSocket::RemoteHostClosedError:
        break;
        case QAbstractSocket::HostNotFoundError:
            QMessageBox::information(this, "QTCPClient", "The host was not found. Please check the host name and port settings.");
        break;
        case QAbstractSocket::ConnectionRefusedError:
            QMessageBox::information(this, "QTCPClient", "The connection was refused by the peer. Make sure QTCPServer is running, and check that the host name and port settings are correct.");
        break;
        default:
            QMessageBox::information(this, "QTCPClient", QString("The following error occurred: %1.").arg(socket->errorString()));
        break;
    }
}

void MainWindow::on_pushButton_sendMessage_clicked()
{
    //ui->pushButton_sendMessage_2->show();

    if(socket)
    {
        if(socket->isOpen())
        {   /////gvn
            QString str = ui->lineEdit_message->text();
            QString str_4="*";
            QString str_2 = ui->lineEdit_message_2->text();
            QString str_3="Gir";
            str_4.append(str_2);
            str.append(str_4);
            str_3.append("*");
            str_3.append(str);
            QDataStream socketStream(socket);
            socketStream.setVersion(QDataStream::Qt_5_7);

            QByteArray header;
            header.prepend(QString("fileType:message,fileName:null,fileSize:%1;").arg(str.size()).toUtf8());
            header.resize(128);

            QByteArray byteArray = str_3.toUtf8();
            byteArray.prepend(header);

            socketStream << byteArray;

            ui->lineEdit_message->clear();
            ui->lineEdit_message_2->clear();
        }
        else
            QMessageBox::critical(this,"QTCPClient","Socket doesn't seem to be opened");
    }
    else
        QMessageBox::critical(this,"QTCPClient","Not connected");
}

void MainWindow::on_pushButton_sendAttachment_clicked()
{
}

void MainWindow::displayMessage(const QString& str)
{
    //ui->txtbro->append(str);


    if(str.contains("passTrue")){
       QStringList list2 = str.split("*");
       QString money=list2[1];
       QString hesaptakipara="hesaptaki para:";
       hesaptakipara.append(money);
       ui->pushButton_sendMessage_4->show();
       ui->pushButton_sendMessage_5->show();
       ui->pushButton_sendMessage_6->show();
       ui->txtbro->append(hesaptakipara);
       ui->lineEdit_message_2->hide();
       ui->lineEdit_message->hide();
       ui->pushButton_sendMessage->hide();
       ui->label->hide();
       ui->label_2->hide();
    } if (str.contains("passFalse")) {

    }
     if(str.contains("GonMik")){
        // Todo para çekmenin aynısı
        ui->txtbro->clear();
        ui->txtbro->append("mikatar hatası ");


    }else if (str.contains("GonHes")) {
            // hesap yok hatası
            ui->txtbro->clear();
            ui->txtbro->append("hesap  hatası ");

    }else if (str.contains("GonTrue")){
         // başarılı
         ui->txtbro->clear();
         ui->txtbro->append("işlem başarılı ");

     }
    if(str.contains("CekTrue")){
        MainWindow:: count_send=MainWindow::count_send+1;

        QStringList list2 = str.split("*");
        QString yeni_para="yeni para miktarı ";
        yeni_para.append(list2[1]);
        ui->txtbro->clear();

        ui->txtbro->append(yeni_para);
        ui->pushButton_sendMessage_3->show();

        if(MainWindow:: count_send==4){
            ui->pushButton_sendMessage_3->hide();
            ui->txtbro->append("günlük para çekme mikatrına ulaştnız ");
        }



    }else if(str.contains("CekFalse")){
        QStringList list2 = str.split("*");
        QString yeni_para="yetersiz bakiye ";
        yeni_para.append(list2[1]);
        ui->txtbro->clear();
        ui->txtbro->append(yeni_para);
        ui->pushButton_sendMessage_3->show();

    }else if((str.contains("Yat"))){
            ui->txtbro->clear();
            QStringList list3=str.split("*");
            QString yeni="yeni para miktarı ";
            yeni.append(list3[1]);
             ui->txtbro->clear();
             ui->txtbro->append(yeni);

    }





}


void MainWindow::on_pushButton_sendMessage_2_clicked()
{
    /// implement para gönder
     QString str = "Gon";
     QString str_1 = "*";
     QString str_2 = ui->lineEdit_message_3->text();
     QString str_3 = ui->lineEdit_message_4->text();
     str.append(str_1);
     str.append(str_2);
     str.append(str_1);
     str.append(str_3);
     //str.append(str_1);

     //ui->txtbro->append(str);

     QDataStream socketStream(socket);
     socketStream.setVersion(QDataStream::Qt_5_7);

     QByteArray header;
     header.prepend(QString("fileType:message,fileName:null,fileSize:%1;").arg(str.size()).toUtf8());
     header.resize(128);

     QByteArray byteArray = str.toUtf8();
     byteArray.prepend(header);

     socketStream << byteArray;
}

void MainWindow::on_pushButton_sendMessage_3_clicked()
{
    ///// to do implement para çek
    ///
    ///

    ui->pushButton_sendMessage_3->hide();




    if(socket)
    {
        if(socket->isOpen())
        {
            QString str = ui->lineEdit_message_5->text();
            QString str_4="*";
            QString str_2 = ui->lineEdit_message_2->text();
            QString str_3="Cek";
            str_4.append(str_2);
            str.append(str_4);
            str_3.append("*");
            str_3.append(str);
            QDataStream socketStream(socket);
            socketStream.setVersion(QDataStream::Qt_5_7);

            QByteArray header;
            header.prepend(QString("fileType:message,fileName:null,fileSize:%1;").arg(str.size()).toUtf8());
            header.resize(128);

            QByteArray byteArray = str_3.toUtf8();
            byteArray.prepend(header);

            socketStream << byteArray;

            ui->lineEdit_message->clear();
            ui->lineEdit_message_2->clear();

        }
        else
            QMessageBox::critical(this,"QTCPClient","Socket doesn't seem to be opened");
    }
    else
        QMessageBox::critical(this,"QTCPClient","Not connected");
}

void MainWindow::on_pushButton_sendMessage_4_clicked()
{
    ui->pushButton_sendMessage_4->hide();
    ui->pushButton_sendMessage_5->hide();
    ui->pushButton_sendMessage_6->hide();
    ui->label_4->show();
    ui->lineEdit_message_5->show();
    ui->pushButton_sendMessage_3->show();
    ui->txtbro->show();

}

void MainWindow::on_pushButton_sendMessage_6_clicked()
{
    ui->pushButton_sendMessage_4->hide();
    ui->pushButton_sendMessage_5->hide();
    ui->pushButton_sendMessage_6->hide();
    ui->label_3->show();
    ui->lineEdit_message_3->show();
    ui->label_5->show();
    ui->lineEdit_message_4->show();
    ui->pushButton_sendMessage_2->show();
    ui->txtbro->show();
}

void MainWindow::on_pushButton_sendMessage_5_clicked()
{
    ui->pushButton_sendMessage_4->hide();
    ui->pushButton_sendMessage_5->hide();
    ui->pushButton_sendMessage_6->hide();
    ui->txtbro->show();
    ui->label_6->show();
    ui->lineEdit_message_6->show();
    ui->pushButton_sendMessage_7->show();

}

void MainWindow::on_pushButton_sendMessage_7_clicked()
{

    ui->pushButton_sendMessage_3->hide();




    if(socket)
    {
        if(socket->isOpen())
        {   QString str1="Yat";
            QString str2="*";
            QString str3=ui->lineEdit_message_6->text();
            str1.append(str2);
            str1.append(str3);
            QDataStream socketStream(socket);
            socketStream.setVersion(QDataStream::Qt_5_7);

            QByteArray header;
            header.prepend(QString("fileType:message,fileName:null,fileSize:%1;").arg(str1.size()).toUtf8());
            header.resize(128);

            QByteArray byteArray = str1.toUtf8();
            byteArray.prepend(header);

            socketStream << byteArray;

            ui->lineEdit_message_6->clear();
            ui->lineEdit_message_2->clear();

        }
        else
            QMessageBox::critical(this,"QTCPClient","Socket doesn't seem to be opened");
    }
    else
        QMessageBox::critical(this,"QTCPClient","Not connected");




}
