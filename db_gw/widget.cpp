#include "widget.h"
#include "./ui_widget.h"

Widget::Widget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Widget)
{
    //initMainWindow();

    socket = new QUdpSocket(this);
    socket->bind(QHostAddress::LocalHost, 25564);

    initFrame();

    ui->setupUi(this);
}

void Widget::initFrame(){
    QGridLayout* gl = new QGridLayout(this);
    QGroupBox* w = new QGroupBox("Log: ");
    gl->addWidget(w);
    QGridLayout* glw = new QGridLayout(w);
    mainWidget = new QListWidget();
    mainWidget->setFlow(QListView::TopToBottom);
    glw->addWidget(mainWidget);
    connect(socket, SIGNAL(readyRead()), SLOT(request()));
}

QByteArray parse(QByteArray &data, int index){
    QByteArray r;
    for(int i=0; i<index; i++){
        r.push_back(data[i]);
    }
    data.remove(0, index+1);
    return r;
}

bool isValidType(int type){
    QVector<int> v_types;
    v_types = {20,21,22,23,24,25,26};
    return v_types.contains(type);
}

void Widget::processServiceRequest(QString login, int type, int role, QString port, QByteArray data){
    item = new QListWidgetItem(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") + " W_Service request: " + login, mainWidget);
    switch(type){
    case 20:{
        socket->writeDatagram(("0/"+port+"|"+data).toUtf8(), QHostAddress::LocalHost, 25566);
        break;
    }
    case 21:{
        socket->writeDatagram(("1/"+port).toUtf8(), QHostAddress::LocalHost, 25566);
        break;
    }
    case 22:{
        socket->writeDatagram(("2/"+port+"?"+QString::fromUtf8(data)).toUtf8(), QHostAddress::LocalHost, 25566);
        break;
    }
    case 23:{
        socket->writeDatagram(("3/"+port+"?"+QString::fromUtf8(data)).toUtf8(), QHostAddress::LocalHost, 25566);
        break;
    }
    case 24:{
        socket->writeDatagram(("4/"+port+"?"+QString::fromUtf8(data)).toUtf8(), QHostAddress::LocalHost, 25566);
        break;
    }
    case 25:{
        socket->writeDatagram(("5/"+port+"?"+QString::fromUtf8(data)).toUtf8(), QHostAddress::LocalHost, 25566);
        break;
    }
    case 26:{
        socket->writeDatagram(("6/"+port+"?"+QString::fromUtf8(data)).toUtf8(), QHostAddress::LocalHost, 25566);
        break;
    }
    default:{
        item = new QListWidgetItem(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") + " Unknown request at pSV", mainWidget);
        break;
    }
    }
}

void Widget::processRequest(QNetworkDatagram datagram){
    int s_port = datagram.senderPort();
    QByteArray data = datagram.data();
    int type = parse(data, data.indexOf("/")).toInt();
    switch(type){
    //login cl->gw->a
    case 10:{
        QString body = "0/";
        QString login = parse(data,data.indexOf("|"));
        QString pwd = data;
        body += login + "|" + pwd + "|" + QString::number(s_port);
        socket->writeDatagram(body.toUtf8(), QHostAddress::LocalHost, 25565);
        break;
    }
    //login a->gw->cl
    case 11:{
        QByteArray answer = parse(data, data.indexOf("|"));
        QString token = parse(data, data.indexOf("|"));
        int s_port = parse(data, data.indexOf("|")).toInt();
        QString role = data;
        QString body = "0/";
        if(answer.toInt()==1){
            item = new QListWidgetItem(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") + " Authenticated: " + QString::number(s_port), mainWidget);
            body+="1";
        } else {
            item = new QListWidgetItem(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") + " Authentication failed: " + QString::number(s_port), mainWidget);
            body+="0";
        }
        socket->writeDatagram((body+"|"+role+"|"+token).toUtf8(), QHostAddress::LocalHost, s_port);
        break;
    }
    //logout cl->gw->a 12/login|token
    case 12:{
        QString login = parse(data, data.indexOf("|"));
        QString token = data;
        QString body = "2/"+login+"|"+token+"|"+QString::number(s_port);
        item = new QListWidgetItem(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") + " Attempting to logout: " + login, mainWidget);
        socket->writeDatagram(body.toUtf8(), QHostAddress::LocalHost, 25565);
        break;
    }
    //logout a->gw->cl 13/answer|s_port
    case 13:{
        QString answer = parse(data, data.indexOf("|"));
        int s_port = data.toInt();
        QString body = "1/"+answer;
        item = new QListWidgetItem(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") + " Log out status for " + QString::number(s_port) + " is " + answer, mainWidget);
        socket->writeDatagram(body.toUtf8(),QHostAddress::LocalHost, s_port);
        break;
    }
    //other requests auth a->gw->services 14/answer|login|r_type|role|port?data
    case 14:{
        int answer = parse(data, data.indexOf("|")).toInt();
        QString login = parse(data, data.indexOf("|"));
        int r_type = parse(data, data.indexOf("|")).toInt();
        int role = parse(data, data.indexOf("|")).toInt();
        QString port = parse(data, data.indexOf("?"));
        if(answer == 1){
            item = new QListWidgetItem(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") + " Got correct token " + login, mainWidget);
            processServiceRequest(login,r_type,role,port,data);
        } else {
            item = new QListWidgetItem(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") + " Got wrong token " + login, mainWidget);
        }
        break;
    }
    case 15:{
        int port = data.toInt();
        item = new QListWidgetItem(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") + " Force logout message " + QString::number(port), mainWidget);
        socket->writeDatagram(("3/"+QString::number(port)).toUtf8(),QHostAddress::LocalHost, port);
        break;
    }
    //w_service->gw->cl checkOutput
    case 200:{
        QString port = parse(data, data.indexOf("?"));
        QString answer = data;
        socket->writeDatagram(("200/"+answer+"?"+data).toUtf8(), QHostAddress::LocalHost, port.toInt());
        break;
    }
    //w_service->gw->cl statusOutput
    case 201:{
        QString port = parse(data, data.indexOf("?"));
        QString answer = data;
        socket->writeDatagram(("201/"+answer+"?"+data).toUtf8(), QHostAddress::LocalHost, port.toInt());
        break;
    }
    //w_service->gw->cl Загрузка role.csv
    case 202:{
        QString answers = parse(data, data.lastIndexOf("|"));
        QString port = data;
        socket->writeDatagram(("202/"+answers).toUtf8(), QHostAddress::LocalHost, port.toInt());
        break;
    }
    //w_service->gw->cl show Загрузка employ.csv
    case 203:{
        QString answers = parse(data, data.lastIndexOf("|"));
        QString port = data;
        socket->writeDatagram(("203/"+answers).toUtf8(), QHostAddress::LocalHost, port.toInt());
        break;
    }
    //w_service->gw->cl checkInsertChanges
    case 204:{
        QString answers = parse(data, data.lastIndexOf("|"));
        QString port = data;
        socket->writeDatagram(("204/"+answers).toUtf8(), QHostAddress::LocalHost, port.toInt());
        break;
    }
    //w_service->gw->cl statusInsertChanges
    case 205:{
        QString port = parse(data, data.indexOf("|"));
        QString answer = data;
        socket->writeDatagram(("205/"+answer).toUtf8(), QHostAddress::LocalHost, port.toInt());
        break;
    }
    case 206:{
        QString answer = parse(data, data.lastIndexOf("|"));
        QString port = data;
        socket->writeDatagram(("206/"+answer).toUtf8(), QHostAddress::LocalHost, port.toInt());
        break;
    }
    //other requests auth cl->gw->a type/login|token?data
    default:{
        if (isValidType(type)){
            QString login = parse(data, data.indexOf("|"));
            QString token = parse(data, data.indexOf("?"));
            item = new QListWidgetItem(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") + " Send token auth for " + login + " request ", mainWidget);
            QString body = "1/" + login + "|" + token + "|" + QString::number(type)+"|"+QString::number(s_port) + "?" + QString::fromUtf8(data);
            item = new QListWidgetItem(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") + " Debug " + body, mainWidget);
            socket->writeDatagram(body.toUtf8(),QHostAddress::LocalHost, 25565);
        } else {
            item = new QListWidgetItem(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") + " Unknown request ", mainWidget);
        }
        break;
    }
    }
}

void Widget::request(){
    while(socket->hasPendingDatagrams()) {
        QNetworkDatagram datagram = socket->receiveDatagram();
        //int s_port = datagram.senderPort();
        //QString data = datagram.data();
        item = new QListWidgetItem(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") + " Request: " + datagram.data(), mainWidget);
        processRequest(datagram);
        //QByteArray replyData = datagram.data();
        //QLabel* msg = new QLabel();
        //qDebug() << data + " / " +QString::number(s_port);
        //socket->writeDatagram(replyData, QHostAddress::LocalHost, s_port);
        //sabLayout->addWidget(msg);
    }
}

Widget::~Widget()
{
    delete ui;
}

