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
    QVector<session> activeSessions;
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

void Widget::processRequest(QNetworkDatagram datagram){
    int s_port = datagram.senderPort();
    QByteArray data = datagram.data();
    switch(parse(data, data.indexOf("/")).toInt()){
    case 10:{
        QString body = "0/";
        QString login = parse(data,data.indexOf("|"));
        QString pwd = data;
        body += login + "|" + pwd + "|" + QString::number(s_port);
        socket->writeDatagram(body.toUtf8(), QHostAddress::LocalHost, 25565);
        break;
    }
    case 11:{
        QByteArray answer = parse(data, data.indexOf("|"));
        session s;
        s.token = parse(data, data.indexOf("|"));
        s.port = data.toInt();
        QString body = "0/";
        if(answer.toInt()==1){
            activeSessions.push_back(s);
            item = new QListWidgetItem(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") + " Authenticated: " + QString::number(s.port), mainWidget);
            body+="1";
        } else {
            item = new QListWidgetItem(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") + " Authentication failed: " + QString::number(s.port), mainWidget);
            body+="0";
        }
        socket->writeDatagram(body.toUtf8(), QHostAddress::LocalHost, s.port);
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

