#include "widget.h"
#include "./ui_widget.h"

Widget::Widget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Widget)
{

    socket = new QUdpSocket(this);
    socket->bind(9999);
    initFrame();
    ui->setupUi(this);
}



void Widget::initFrame(){
    QGridLayout* gl = new QGridLayout(this);
    QGroupBox* w = new QGroupBox("Log: ");
    gl->addWidget(w,0,0,1,1);
    QGridLayout* glw = new QGridLayout(w);
    mainWidget = new QListWidget();
    mainWidget->setFlow(QListView::TopToBottom);
    glw->addWidget(mainWidget);
    QGroupBox *w1 = new QGroupBox("Input: ");
    QGridLayout *gl1 = new QGridLayout(w1);
    QLabel* ril = new QLabel("Request: ");
    requestInput = new QLineEdit();
    QLabel* pil = new QLabel("Port: ");
    portInput = new QLineEdit();
    QPushButton* submit = new QPushButton("OK");
    connect(submit, SIGNAL(clicked()), this, SLOT(buttonPressed()));
    gl1->addWidget(ril, 0, 0);
    gl1->addWidget(requestInput, 0, 1);
    gl1->addWidget(pil,1,0);
    gl1->addWidget(portInput,1,1);
    gl1->addWidget(submit,2,0,1,2);
    gl->addWidget(w1,1,0,1,1);

    connect(socket, SIGNAL(readyRead()), SLOT(getResponse()));
}

void Widget::buttonPressed(){
    sendRequest();
}

void Widget::sendRequest(){
    item = new QListWidgetItem(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") + " -> " + requestInput->text(), mainWidget);
    item->setFlags(Qt::ItemIsEditable | Qt::ItemIsEnabled);
    socket->writeDatagram(requestInput->text().toUtf8(), QHostAddress::LocalHost, portInput->text().toInt());
}

void Widget::getResponse(){
    while(socket->hasPendingDatagrams()) {
        QNetworkDatagram datagram = socket->receiveDatagram();
        QString data = datagram.data();
        item = new QListWidgetItem(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") + " <- " + data, mainWidget);
        item->setFlags(Qt::ItemIsEditable | Qt::ItemIsEnabled);
    }
}

Widget::~Widget()
{
    delete ui;
}

