#include "widget.h"
#include "./ui_widget.h"

QRegularExpression re_login("^[a-zA-Z1-9]+$");
QRegularExpressionValidator v_login(re_login, 0);

Widget::Widget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Widget)
{
    ui->setupUi(this);

    socket = new QUdpSocket(this);
    socket->bind(25565);

    initFrame();
}

void Widget::initFrame(){
    QGridLayout* gl = new QGridLayout(this);
    QGroupBox* w = new QGroupBox("Log: ");
    gl->addWidget(w);
    QGridLayout* glw = new QGridLayout(w);
    mainWidget = new QListWidget();
    mainWidget->setFlow(QListView::TopToBottom);
    glw->addWidget(mainWidget);
    db = QSqlDatabase::addDatabase("QPSQL");
    db.setHostName("localhost");
    db.setDatabaseName("postgres");
    db.setUserName("postgres");
    db.setPassword("12345");
    bool ok = db.open();
    if(!ok){
        item = new QListWidgetItem(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") + " DB connection error", mainWidget);
    }
    connect(socket, SIGNAL(readyRead()), SLOT(request()));
}

void Widget::request(){
    while(socket->hasPendingDatagrams()) {
        QNetworkDatagram datagram = socket->receiveDatagram();
        QByteArray replyData = datagram.data();
        item = new QListWidgetItem(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") + " Request: " + datagram.data(), mainWidget);
        processRequest(replyData);
        //socket->writeDatagram(replyData, QHostAddress::LocalHost, 22564);
        //item = new QListWidgetItem(QDateTime::currentDateTime().toString("hh:ss:mm") + " Response: " + replyData, mainWidget);

    }
}

QByteArray parse(QByteArray &data, int index){
    QByteArray r;
    for(int i=0; i<index; i++){
        r.push_back(data[i]);
    }
    data.remove(0, index+1);
    return r;
}

QString getRandomString(){
   const QString possibleCharacters("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789");
   const int randomStringLength = 12;
   QString randomString;
   for(int i=0; i<randomStringLength; ++i)
   {
       int index = 1 + rand() % possibleCharacters.length();
       QChar nextChar = possibleCharacters.at(index);
       randomString.append(nextChar);
   }
   return randomString;
}

void Widget::login(QByteArray data){
    int index = data.indexOf("|");
    if(index!=-1){
        QString login = parse(data,data.indexOf("|"));
        QByteArray pwd_raw = parse(data, data.indexOf("|"));
        int pos = 0;
        if(v_login.validate(login,pos)){
            //qDebug() << login;
            QByteArray pwd = QCryptographicHash::hash(pwd_raw, QCryptographicHash::Md5);
            QString token = getRandomString().toUtf8();
            qDebug() << login << Qt::endl;
            qDebug() << pwd << Qt::endl;
            qDebug() << token << Qt::endl;
            QSqlQuery query;
            //query.prepare("SELECT * FROM Auth WHERE login='" + login + "' AND pwd='" + QString::fromUtf8(pwd_raw) +"';");
            query.prepare("select * from Auth");
            //query.bindValue(":login", login);
            //query.bindValue(":pwd", pwd_raw);
            query.exec();
            QString body = "11/";
            if(query.value(0).toString().isEmpty()){
                qDebug() << query.driver();
                item = new QListWidgetItem(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") + " User with login " + login + " does not exist with recieved pwd " + QString::fromUtf8(pwd_raw) + " q " + query.lastQuery(), mainWidget);
                body+="0|";
            } else {
                body += "1|";
                query.prepare("UPDATE Auth WHERE login=? SET token=?, active=1");
                query.bindValue(0, login);
                query.bindValue(1, token);
            }
            body+=token+"|"+QString::number(data.toInt());
            socket->writeDatagram(body.toUtf8(),QHostAddress::LocalHost,25564);
            query.exec();
        } else {
            item = new QListWidgetItem(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") + " Invalid login", mainWidget);
        }
    } else {
        item = new QListWidgetItem(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") + " Parsing error", mainWidget);
    }
}

void Widget::authRequest(QByteArray data){
    int pos=0;
    QString token = parse(data, data.indexOf("|"));
    if(v_login.validate(token,pos)){
        QSqlQuery query;
        query.prepare("SELECT * FROM Auth WHERE active=1 AND token=:token");
        query.bindValue(":token", token);
        query.exec();
        QByteArray arr = "13/0";
        if(query.value(0).toString().isEmpty()){
            item = new QListWidgetItem(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") + " Session with token " + token + " does not exist", mainWidget);
        } else {
            arr = "13/";
        }
        socket->writeDatagram(arr,QHostAddress::LocalHost,25564);
    } else {
        QByteArray arr = "0";
        item = new QListWidgetItem(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") + " Invalid token", mainWidget);
        socket->writeDatagram(arr,QHostAddress::LocalHost,25564);
    }
}

void Widget::logOut(QByteArray data){

}

void Widget::processRequest(QByteArray data){
    if(db.isOpen()){
        int index = data.indexOf("/");
        int type = parse(data, index).toInt();
        switch(type){
        case 0: {
            login(data);
            break;
        }
        case 1: {
            authRequest(data);
            break;
        }
        case 2: {
            logOut(data);
            break;
        }
        default: {
            item = new QListWidgetItem(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") + " Unknown request type", mainWidget);
        }
        }
    }
}

Widget::~Widget()
{
    delete ui;
}

