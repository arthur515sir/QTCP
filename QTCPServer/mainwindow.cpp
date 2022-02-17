#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <qfile.h>
#include <iostream>
#include <string>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow)
{   MainWindow::file_path="C:\\Users\\MSI\\Desktop\\QTcpSocket-master\\QTCPServer\\gvn.txt";// dosya adresini kendi adresinize göre değiştiriniz
    ui->setupUi(this);
    m_server = new QTcpServer();

    if(m_server->listen(QHostAddress::Any, 8080))
    {
       connect(this, &MainWindow::newMessage, this, &MainWindow::displayMessage);
       connect(m_server, &QTcpServer::newConnection, this, &MainWindow::newConnection);
       ui->statusBar->showMessage("Server is listening...");
    }
    else
    {
        QMessageBox::critical(this,"QTCPServer",QString("Unable to start the server: %1.").arg(m_server->errorString()));
        exit(EXIT_FAILURE);
    }
}

MainWindow::~MainWindow()
{
    foreach (QTcpSocket* socket, connection_set)
    {
        socket->close();
        socket->deleteLater();
    }

    m_server->close();
    m_server->deleteLater();

    delete ui;
}

void MainWindow::newConnection()
{
    while (m_server->hasPendingConnections())
        appendToSocketList(m_server->nextPendingConnection());
}

void MainWindow::appendToSocketList(QTcpSocket* socket)
{
    connection_set.insert(socket);
    connect(socket, &QTcpSocket::readyRead, this, &MainWindow::readSocket);
    connect(socket, &QTcpSocket::disconnected, this, &MainWindow::discardSocket);
   // connect(socket, &QAbstractSocket::errorOccurred, this, &MainWindow::displayError);
    ui->comboBox_receiver->addItem(QString::number(socket->socketDescriptor()));
    displayMessage(QString("müşteri  %1  girişi ").arg(socket->socketDescriptor()),socket);
}

void MainWindow::readSocket()
{
    QTcpSocket* socket = reinterpret_cast<QTcpSocket*>(sender());

    QByteArray buffer;

    QDataStream socketStream(socket);
    socketStream.setVersion(QDataStream::Qt_5_7);

    socketStream.startTransaction();
    socketStream >> buffer;

    if(!socketStream.commitTransaction())
    {
        QString message = QString("%1 :: Waiting for more data to come..").arg(socket->socketDescriptor());
        emit newMessage(message,socket);
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
                emit newMessage(message,socket);
            }else
                QMessageBox::critical(this,"QTCPServer", "An error occurred while trying to write the attachment.");
        }else{
            QString message = QString("INFO :: Attachment from sd:%1 discarded").arg(socket->socketDescriptor());
            emit newMessage(message,socket);
        }
    }else if(fileType=="message"){
        QString message = QString("%1 :: %2").arg(socket->socketDescriptor()).arg(QString::fromStdString(buffer.toStdString()));
        QString message_2=QString("%1").arg(QString::fromStdString(buffer.toStdString()));
        emit newMessage(message_2,socket);///gvn

    }
}

void MainWindow::discardSocket()
{
    QTcpSocket* socket = reinterpret_cast<QTcpSocket*>(sender());
    QSet<QTcpSocket*>::iterator it = connection_set.find(socket);
    if (it != connection_set.end()){
        displayMessage(QString("INFO :: A client has just left the room").arg(socket->socketDescriptor()),socket);
        connection_set.remove(*it);
    }
    refreshComboBox();
    
    socket->deleteLater();
}

void MainWindow::displayError(QAbstractSocket::SocketError socketError)
{
    switch (socketError) {
        case QAbstractSocket::RemoteHostClosedError:
        break;
        case QAbstractSocket::HostNotFoundError:
            QMessageBox::information(this, "QTCPServer", "The host was not found. Please check the host name and port settings.");
        break;
        case QAbstractSocket::ConnectionRefusedError:
            QMessageBox::information(this, "QTCPServer", "The connection was refused by the peer. Make sure QTCPServer is running, and check that the host name and port settings are correct.");
        break;
        default:
            QTcpSocket* socket = qobject_cast<QTcpSocket*>(sender());
            QMessageBox::information(this, "QTCPServer", QString("The following error occurred: %1.").arg(socket->errorString()));
        break;
    }
}

void MainWindow::on_pushButton_sendMessage_clicked()
{
    QString receiver = ui->comboBox_receiver->currentText();

    if(receiver=="Broadcast")
    {
        foreach (QTcpSocket* socket,connection_set)
        {
            sendMessage(socket);
        }
    }
    else
    {
        foreach (QTcpSocket* socket,connection_set)
        {
            if(socket->socketDescriptor() == receiver.toLongLong())
            {
                sendMessage(socket);
                break;
            }
        }
    }
    ui->lineEdit_message->clear();
}


void MainWindow::on_pushButton_sendAttachment_clicked()
{
    QString receiver = ui->comboBox_receiver->currentText();

    QString filePath = QFileDialog::getOpenFileName(this, ("Select an attachment"), QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation), ("File (*.json *.txt *.png *.jpg *.jpeg)"));

    if(filePath.isEmpty()){
        QMessageBox::critical(this,"QTCPClient","You haven't selected any attachment!");
        return;
    }

    if(receiver=="Broadcast")
    {
        foreach (QTcpSocket* socket,connection_set)
        {
            sendAttachment(socket, filePath);
        }
    }
    else
    {
        foreach (QTcpSocket* socket,connection_set)
        {
            if(socket->socketDescriptor() == receiver.toLongLong())
            {
                sendAttachment(socket, filePath);
                break;
            }
        }
    }
    ui->lineEdit_message->clear();
}

void MainWindow::sendMessage(QTcpSocket* socket)
{
    if(socket)
    {
        if(socket->isOpen())
        {
            QString str = ui->lineEdit_message->text();
            QDataStream socketStream(socket);
            socketStream.setVersion(QDataStream::Qt_5_7);
            QByteArray header;
            header.prepend(QString("fileType:message,fileName:null,fileSize:%1;").arg(str.size()).toUtf8());
            header.resize(128);
            QByteArray byteArray = str.toUtf8();
            byteArray.prepend(header);
            socketStream.setVersion(QDataStream::Qt_5_7);
            socketStream << byteArray;
        }
        else
            QMessageBox::critical(this,"QTCPServer","Socket doesn't seem to be opened");
    }
    else
        QMessageBox::critical(this,"QTCPServer","Not connected");
}

void MainWindow::sendAttachment(QTcpSocket* socket, QString filePath)
{
    if(socket)
    {
        if(socket->isOpen())
        {
            QFile m_file(filePath);
            if(m_file.open(QIODevice::ReadOnly)){

                QFileInfo fileInfo(m_file.fileName());
                QString fileName(fileInfo.fileName());
                QDataStream socketStream(socket);
                socketStream.setVersion(QDataStream::Qt_5_7);
                QByteArray header;
                header.prepend(QString("fileType:attachment,fileName:%1,fileSize:%2;").arg(fileName).arg(m_file.size()).toUtf8());
                header.resize(128);
                QByteArray byteArray = m_file.readAll();
                byteArray.prepend(header);
                socketStream << byteArray;
            }else
                QMessageBox::critical(this,"QTCPClient","Couldn't open the attachment!");
        }
        else
            QMessageBox::critical(this,"QTCPServer","Socket doesn't seem to be opened");
    }
    else
        QMessageBox::critical(this,"QTCPServer","Not connected");
}

void MainWindow::displayMessage(const QString& str, QTcpSocket* socket)
{
    ui->textBrowser_receivedMessages->clear();
    QString dummy;
    dummy.append(str[0]);
    dummy.append(str[1]);
    dummy.append(str[2]);
    QString str_2="gvn";
    QString str_True="passTrue";
    QString str_False="passFalse";
    if(dummy=="Gir"){
        // todo giriş check
        QDataStream socketStream(socket);
        socketStream.setVersion(QDataStream::Qt_5_7);
        QStringList list1=str.split("*");
        float flag=MainWindow::openfilePassword(list1[1] ,list1[2]);
        QString moneystr = QString::number(MainWindow::money);
        if(flag){//// True ise burası
            QByteArray header;
            header.prepend(QString("fileType:message,fileName:null,fileSize:%1;").arg(str.size()).toUtf8());
            header.resize(128);
            str_True.append("*");
            str_True.append(moneystr);
            QByteArray byteArray = str_True.toUtf8();
            byteArray.prepend(header);
            socketStream.setVersion(QDataStream::Qt_5_7);
            socketStream << byteArray;
        }else {// false ise burası
            QByteArray header;
            header.prepend(QString("fileType:message,fileName:null,fileSize:%1;").arg(str.size()).toUtf8());
            header.resize(128);
            QByteArray byteArray = str_False.toUtf8();
            byteArray.prepend(header);
            socketStream.setVersion(QDataStream::Qt_5_7);
            socketStream << byteArray;
        }
    }else if(dummy=="Gon"){
        //todo para gönderme server
        QDataStream socketStream(socket);
        socketStream.setVersion(QDataStream::Qt_5_7);       
        float paramik=0.0;
        QStringList list1=str.split("*");
        QString miktar_str=list1[2];
        QString id_str=list1[1];
        paramik =miktar_str.toFloat();
        int dummy= MainWindow::openfileGon( id_str , paramik);
        QByteArray header;
        if(dummy==0){
            //hesap bulunamadı
            QString dum1="GonHes";
            header.prepend(QString("fileType:message,fileName:null,fileSize:%1;").arg(str.size()).toUtf8());
            header.resize(128);
            QByteArray byteArray = dum1.toUtf8();
            byteArray.prepend(header);
            socketStream.setVersion(QDataStream::Qt_5_7);
            socketStream << byteArray;
        }else if (dummy==1){
            // miktar yetersiz
            header.prepend(QString("fileType:message,fileName:null,fileSize:%1;").arg(str.size()).toUtf8());
            header.resize(128);
           QString dum2="GonMik";
            QByteArray byteArray = dum2.toUtf8();
            byteArray.prepend(header);
            socketStream.setVersion(QDataStream::Qt_5_7);
            socketStream << byteArray;
        }else if(dummy==2){
            // başarılı
            header.prepend(QString("fileType:message,fileName:null,fileSize:%1;").arg(str.size()).toUtf8());
            header.resize(128);
            QString dum3="GonTrue";
            QByteArray byteArray = dum3.toUtf8();
            byteArray.prepend(header);
            socketStream.setVersion(QDataStream::Qt_5_7);
            socketStream << byteArray;
        }
    }else if (dummy=="Cek"){
        QString str_True= "CekTrue";
        QString str_False= "CekFalse";
        float a;
        QStringList list1=str.split("*");
        //ui->textBrowser_receivedMessages->append(list1[1]);
        a=list1[1].toInt();
        QDataStream socketStream(socket);
        socketStream.setVersion(QDataStream::Qt_5_7);
        int b=MainWindow::openfileCek(a);//// todo çekilecel para mik gelicek
        if (!(b==(-1))){
            QString moneystr = QString::number(b);
            QByteArray header;
            header.prepend(QString("fileType:message,fileName:null,fileSize:%1;").arg(str.size()).toUtf8());
            header.resize(128);
            str_True.append("*");
            str_True.append(moneystr);
            QByteArray byteArray = str_True.toUtf8();
            byteArray.prepend(header);
            socketStream.setVersion(QDataStream::Qt_5_7);
            socketStream << byteArray;
        }else{
            QString moneystr = QString::number(0.0);
            QByteArray header;
            header.prepend(QString("fileType:message,fileName:null,fileSize:%1;").arg(str.size()).toUtf8());
            header.resize(128);
            str_False.append("*");
            str_False.append(moneystr);
            QByteArray byteArray = str_False.toUtf8();
            byteArray.prepend(header);
            socketStream.setVersion(QDataStream::Qt_5_7);
            socketStream << byteArray;
        }
    }else if(dummy=="Yat"){
         ui->textBrowser_receivedMessages->append("yatır ");
         ui->textBrowser_receivedMessages->append(str);
          QStringList list1=str.split("*");

         int para=MainWindow::openfileYat(list1[1].toInt());
         QString yat="Yat";

         yat.append("*");
         yat.append(QString::number(para));


         QDataStream socketStream(socket);
         socketStream.setVersion(QDataStream::Qt_5_7);
         QByteArray header;
         header.prepend(QString("fileType:message,fileName:null,fileSize:%1;").arg(yat.size()).toUtf8());
         header.resize(128);
         QByteArray byteArray = yat.toUtf8();
         byteArray.prepend(header);
         socketStream.setVersion(QDataStream::Qt_5_7);
         socketStream << byteArray;
    }
}
int  MainWindow::openfileYat(int paramik){
    QFile file(MainWindow::file_path);
    QStringList strings;
    int money_1=0;
    int numberOfAccount=0;
    if (file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QTextStream in(&file);
        while (!in.atEnd()) {
            strings += in.readLine();
            numberOfAccount=numberOfAccount+1;
        }
    file.close();
    }
    if (file.open(QIODevice::ReadWrite  | QIODevice::Text))
    {
        QTextStream in(&file);
        for (int i=0;i<numberOfAccount;i++){
            QStringList account=  strings[i].split(",");
            for(int j=0;j<4;j++){
                if ((account[j]==MainWindow::id )&& (account[j+1]==MainWindow::pass)){
                    in<<account[j];
                        money_1=account[j+2].toInt();
                        account[j+2]=QString::number(money_1+paramik);////// hesaptan para ekleme çıkarma
                        money_1=money_1+paramik;                       
                    if(j==3){

                    }else{
                      in<<",";
                    }
                }else {
                    in<<account[j];
                    if(j==3){

                    }else{
                        in<<",";
                    }
                }
            }
            in<<endl;
        }
        }
file.close();
return money_1;
}
int  MainWindow::openfileGon(QString id_of_occunt_last , int paramik){

    QFile file(MainWindow::file_path);
    QStringList strings;
    int flag_1=0;
    int flag_2=0;
    int flag_3=0;
    int numberOfAccount=0;
    int money=0;

    /******************para transfer *********************/
    if (file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QTextStream in(&file);
        while (!in.atEnd()) {
            strings += in.readLine();
            numberOfAccount=numberOfAccount+1;
        }
    file.close();
    }
    if (file.open(QIODevice::ReadWrite  | QIODevice::Text))
    {
        QTextStream in(&file);
        for (int i=0;i<numberOfAccount;i++){
            QStringList account=  strings[i].split(",");
            for(int j=0;j<4;j++){
                if ((account[j]==id_of_occunt_last )){
                    flag_1=1;
                    in<<account[j];
                    int dum1=account[j+2].toInt();
                    dum1=dum1+paramik;
                    account[j+2]=QString::number(dum1);
                     if(j==3){

                    }else {
                         in<<",";
                    }

                }else if ((account[j]==MainWindow::id )&& (account[j+1]==MainWindow::pass)) {                    
                    in<<account[j];
                    money=account[j+2].toInt();
                    if(money<paramik){
                        ui->textBrowser_receivedMessages->append("yetersiz para");
                        flag_2=1;
                    }else{
                        account[j+2]=QString::number(money-paramik);////// hesaptan para ekleme çıkarma
                        money=money-paramik;
                        flag_3=1;
                    }
                    if(j==3){

                    }else {
                        in<<",";
                    }
                }else {
                    in<<account[j];
                    if(j==3){

                    }else{
                        in<<",";
                    }
                }
            }
            in<<endl;
        }
        }

file.close();
if(flag_1==0){
    return 0;       //// hesap bulunamadı
}else if(flag_2==1){
    return 1;       // miktar yetersiz
}else if(flag_3==1){
    return 2;       /// başarılı

}
}
int MainWindow::openfileCek(int paramik){
    QFile file(MainWindow::file_path);
    QStringList strings;
    bool flag=0;
    int money_2=0;
    int numberOfAccount=0;
    if (file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QTextStream in(&file);
        while (!in.atEnd()) {
            strings += in.readLine();
            numberOfAccount=numberOfAccount+1;
        }
    file.close();



    }


    if (file.open(QIODevice::ReadWrite  | QIODevice::Text))
    {
        QTextStream in(&file);
        for (int i=0;i<numberOfAccount;i++){
            QStringList account=  strings[i].split(",");
            for(int j=0;j<4;j++){
                if ((account[j]==MainWindow::id )&& (account[j+1]==MainWindow::pass)){
                    in<<account[j];
                    money_2=account[j+2].toInt();
                    if(money_2<paramik){                    
                         flag=0;
                    }else{
                        account[j+2]=QString::number(money_2-paramik);////// hesaptan para ekleme çıkarma
                        money_2=money_2-paramik;                      
                        flag=1;
                    }
                    if(j==3){

                    }else{
                         in<<",";
                    }
                }else {
                    in<<account[j];
                    if(j==3){
                    }else{
                        in<<",";
                    }
                }
            }
            in<<endl;
        }
        }
file.close();
if(flag==0){
    return -1;
}else if(flag==1){
    return money_2;
}

}
int MainWindow::openfilePassword(QString a,QString b){
    int flag=0;
    QString isim;
    QFile file(MainWindow::file_path);
    QStringList strings;
    if (file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QTextStream in(&file);
        int numberOfAccount=0;
        while (!in.atEnd()) {
            strings += in.readLine();
            numberOfAccount=numberOfAccount+1;
        }
        for(int i=0;i<numberOfAccount;i++){
            QStringList account=  strings[i].split(",");
            for(int j=0;j<4;j++){
                if((account[j]==a )&& (account[j+1]==b)){
                    isim=account[j-1];
                    MainWindow::pass=account[j+1];
                    MainWindow::id=account[j];                   
                    MainWindow::money=account[j+2].toInt();
                   return 1;
                }else {
                }
            }
        }
    }
return 0;
}
void MainWindow::refreshComboBox(){
    ui->comboBox_receiver->clear();
    ui->comboBox_receiver->addItem("Broadcast");
    foreach(QTcpSocket* socket, connection_set)
        ui->comboBox_receiver->addItem(QString::number(socket->socketDescriptor()));
}
