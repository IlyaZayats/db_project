#include "widget.h"
#include "./ui_widget.h"
#include "QtWidgets"
#include "QNetworkDatagram"
#include <QtSql>

#include "iostream"

Widget::Widget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Widget)
{
    QGridLayout* grid = new QGridLayout(this);
    mainWidget = new QWidget();
    mainWidget->resize(800,600);
    mainLayout = new QGridLayout(mainWidget);
    grid->addWidget(mainWidget,0,0,1,1);

    request = new QUdpSocket(this);
    request->bind(3001 + rand()%4);

//    response = new QUdpSocket(this);
//    response->bind(1111);




    initLogin();
    connect(request, SIGNAL(readyRead()), SLOT(getResponse()));
    ui->setupUi(this);
}

void Widget::getResponse(){
    std::cout << "OK";
    while(request->hasPendingDatagrams()) {
        QNetworkDatagram datagram = request->receiveDatagram();
        int s_port = datagram.senderPort();
        QString data = datagram.data();
        loginError->setText(data + " --- " +QString::number(s_port));
    }
}

void Widget::sendRequset(){
    QByteArray arr = (QString::number(10) + "/" + loginInput->text() + "|" + pwdInput->text()).toUtf8();
    request->writeDatagram(arr, QHostAddress::LocalHost, 25564);
}

void Widget::initLogin(){
    loginWidget = new QGroupBox("Login");
    QFormLayout* loginLayout = new QFormLayout(loginWidget);
    QLabel* loginLabel = new QLabel("Input login: ");
    loginLayout->setWidget(0, QFormLayout::LabelRole,loginLabel);
    loginInput = new QLineEdit();
    loginInput->setMaxLength(30);
    loginLayout->setWidget(0, QFormLayout::FieldRole, loginInput);
    //amountError = new QLabel("", formWidget);
    //amountError->setStyleSheet("color : red");
    //formLayout->setWidget(1, QFormLayout::FieldRole, amountError);
    QLabel* pwdLabel = new QLabel("Input password: ");
    loginLayout->setWidget(1, QFormLayout::LabelRole,pwdLabel);
    pwdInput = new QLineEdit();
    pwdInput->setMaxLength(30);
    loginLayout->setWidget(1, QFormLayout::FieldRole, pwdInput);
    loginError = new QLabel("");
    loginError->setStyleSheet("color : red");
    loginLayout->setWidget(2, QFormLayout::FieldRole, loginError);
    QPushButton* submit = new QPushButton("LOG IN");
    connect(submit, SIGNAL(clicked()), this, SLOT(loginPressed()));
    loginLayout->setWidget(3, QFormLayout::FieldRole, submit);
    //mainError = new QLabel("");
    //mainError->setStyleSheet("color : red; font : bold");
    //formLayout->setWidget(5, QFormLayout::FieldRole, mainError);
    mainLayout->addWidget(loginWidget, 0, 0, 1, 1);
}

void Widget::hideLogin(){
    loginWidget->hide();
}

void Widget::loginPressed(){
    //QByteArray pwd = QCryptographicHash::hash(pwdInput->text().toUtf8(), QCryptographicHash::Md5);
    //qDebug() << pwd;
    sendRequset();
//    QSqlDatabase db = QSqlDatabase::addDatabase("QPSQL");
//    db.setHostName("localhost");
//    db.setDatabaseName("postgres");
//    db.setUserName("postgres");
//    db.setPassword("12345");
//    bool ok = db.open();
//    qDebug() << ok;
//    QString input = loginInput->text();
//    int pos = 0;
//    bool error = true;
//    //к бд запрос
//    if(error){
//        loginError->setText("Wrong login or password");
//    } else {
//        loginError->setText("");
//    }

}

Widget::~Widget()
{
    delete ui;
}

