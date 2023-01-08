#include "widget.h"
#include "./ui_widget.h"
#include <QtWidgets>
#include <QFile>
#include <QString>
#include <QMessageBox>
#include <QFileDialog>
#include <QLabel>
#include <QDir>
#include <QRegularExpression>
#include <QTableWidget>
#include <iostream>
#include "Random.hpp"
#include <QtSql/QSql>
#include <QtSql/QSqlDatabase>
#include <QUdpSocket>

Widget::Widget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Widget)
{
    idLogInRequest = 10;
    idLogOutRequest = 12;
    grid = new QGridLayout(this);
    mainWidget = new QWidget();
    mainWidget->resize(800,600);
    mainLayout = new QGridLayout(mainWidget);
    grid->addWidget(mainWidget);
    udpSocket = new QUdpSocket(this);
    udpSocket->bind(QHostAddress::LocalHost, 25564);
    creatingLogInWindow();
    //////////////////////////////////////////////////////////////ВЫБРАТЬ ФАЙЛ//////////////
//    useFileButton = new QPushButton("Выбрать файл");
//    fileLabel = new QLabel("");
//    connect(useFileButton, SIGNAL(clicked()), this, SLOT(on_useFileButton_clicked()));
//    mainLayout->addWidget(useFileButton, 0, 0, 1, 2);
//    mainLayout->addWidget(fileLabel, 1, 0, 1, 1);
    ui->setupUi(this);

    //QString fileName = QFileDialog::getOpenFileName(this, tr("Open File"), "/home", tr("Images (*.png *.xpm *.jpg)"));

}


void Widget::creatingLogInWindow(){
    if(!createConnection()) qDebug() << "Не удалось подключиться к бд";
    qDebug() << port; //генерация случайного числа
    nameApp = new QLabel();
    nameApp->setText("ЛУЧШЕЕ ПРИЛОЖЕНИЕ");
    nameApp->setStyleSheet(QString("font-size: %1px").arg(40));
    nameAuth = new QLabel();
    nameAuth->setText("Авторизация");
    nameAuth->setStyleSheet(QString("font-size: %1px").arg(30));
    login = new QLabel();
    login->setText("Логин");
    inputLogin = new QLineEdit();
    password = new QLabel();
    password->setText("Пароль");
    inputPassword = new QLineEdit();
    buttonAuth = new QPushButton();
    error = new QLabel("");
    buttonAuth->setText("Войти");
    connect(buttonAuth, SIGNAL(clicked()), this, SLOT(on_buttonAuth_clicked()));
    mainLayout->addWidget(nameApp, 0, 3, 1, 3,  Qt::AlignTop);
    mainLayout->addWidget(nameAuth, 1, 3, 1, 3, Qt::AlignCenter);
    mainLayout->addWidget(login, 2, 0, 1, 1, Qt::AlignTop);
    mainLayout->addWidget(inputLogin, 2, 1, 1, 7);
    mainLayout->addWidget(password, 3, 0, 1, 1, Qt::AlignTop);
    mainLayout->addWidget(inputPassword, 3, 1, 1, 7);
    mainLayout->addWidget(error, 4, 1, 1, 7);
    mainLayout->addWidget(buttonAuth, 5, 1, 1, 2, Qt::AlignTop);
}

bool Widget:: createConnection(){
    QSqlDatabase db = QSqlDatabase::addDatabase("QPSQL");
    db.setDatabaseName("postgres");;
       db.setUserName("postgres");
       db.setPassword("279091");
       if (!db.open())
       {
           return false;
       }
       else{
           return true;
       }
       return true;
}

//bool createTables(){
//    QSqlQuery query;
//    query.exec("CREATE TABLES passwords ("
//               "id INTEGER PRIMARY KEY, "
//               "login VARCHAR NOT NULL)");
//    return(1);
//}


void Widget::on_useFileButton_clicked()
{
    QString filter = "*.csv";
    QString fileName = QFileDialog::getOpenFileName(this, tr("Open File"), filter, QDir::homePath());
    QFile file(fileName);
    if (!file.open(QFile::ReadOnly | QFile::Text)){
        QMessageBox::warning(this, "title", "file not open");
    }
    fileName.remove(0,fileName.lastIndexOf("/")+1);
    fileLabel->setText(fileName);
    QString data = file.readAll();
    qDebug() << data;
    udpSocket->writeDatagram(data.toUtf8(), QHostAddress::LocalHost, 25564);
    connect(udpSocket, SIGNAL(readyRead()), this, SLOT(ReadingDataFile()));


//    QGridLayout* grid = new QGridLayout(this);
//    mainWidget = new QWidget();
//    mainLayout = new QGridLayout(mainWidget);
//    grid->addWidget(mainWidget,0,0,11,1);
//    QLabel* fileTextName = new QLabel(fileName);
//    grid->addWidget(fileTextName, 11, 0);
}

void Widget::ReadingDataFile(){
    QHostAddress sender;
    quint16 senderPort;
    QString data;
    while(udpSocket->hasPendingDatagrams()){
        QByteArray datagram;
        datagram.resize(udpSocket->pendingDatagramSize());
//        udpSocket->readDatagram(datagram.data(), datagram.size(), &sender, &senderPort);
        udpSocket->readDatagram(datagram.data(), datagram.size(), &sender, &senderPort);
        data = QString(datagram);
        qDebug()<<data;
    }
}


bool Widget::checkPassRegExp(QRegularExpression reg, QString str1, QString str2){
    auto match1 = reg.match(str1);
    auto match2 = reg.match(str2);
    if(match1.hasMatch() && match2.hasMatch() && str2.length()>=5 && str2.length()<=20) return true;
    else{
        error->setText("Неверно введённые данные!");
        error->setStyleSheet("color : red");
        inputLogin->clear();
        inputPassword->clear();
        return false;
    }
}

void Widget::ReadingData(){
    QHostAddress sender;
    quint16 senderPort;
    while(udpSocket->hasPendingDatagrams()){
        QByteArray datagram;
        datagram.resize(udpSocket->pendingDatagramSize());
//        udpSocket->readDatagram(datagram.data(), datagram.size(), &sender, &senderPort);
        udpSocket->readDatagram(datagram.data(), datagram.size(), &sender, &senderPort);
        strUserInfoGet = QString(datagram);

    }
    reqBtnAuth(strUserInfoGet);
}

void Widget::reqBtnAuth(QString str){
    //Строка о пользователе, полученная с бэка
    QString strAnswerGet = "";
    QString strRoleGet = "";
    token = "";
    for(int i=strUserInfoGet.lastIndexOf("|")+1; i<strUserInfoGet.length(); i++){
        token += strUserInfoGet[i];
    }
    qDebug() << token;
    for(int i=strUserInfoGet.indexOf("/")+1; i<strUserInfoGet.indexOf("|"); i++){
        strAnswerGet += strUserInfoGet[i];
    }
    for(int i=strUserInfoGet.indexOf("|")+1; i<strUserInfoGet.lastIndexOf("|"); i++){
        strRoleGet += strUserInfoGet[i];
    }
    qDebug() << strUserInfoGet;
    int answerGet = strAnswerGet.toInt();
    roleGet = strRoleGet.toInt();
    if(answerGet == 1){
        if(roleGet == 1 || roleGet == 5){
            for (int i = 0; i < mainLayout->count(); ++i)
            {
                QWidget *widg = mainLayout->itemAt(i)->widget();
                widg->hide();
            }
            creatingUserWindow();
        }
        else{
            for (int i = 0; i < mainLayout->count(); ++i)
            {
                QWidget *widg = mainLayout->itemAt(i)->widget();
                widg->hide();
            }
            creatingAnotherUserWindow();
        }
    }
    else{
        error->setText("Введён неверный логин или пароль!");
        error->setStyleSheet("color : red");
        creatingLogInWindow();
    }
}

void Widget::on_buttonAuth_clicked(){
    log = inputLogin->text();
    pass = inputPassword->text();
    QRegularExpression reg("[0-9]|[A-Z]|[a-z]|[!@#$%^&*?]");
    if(checkPassRegExp(reg, log, pass)){
        /*Блок отправки данных о логине и пароле и получении с бэка инфы о пользователе*/
        QString strUserInfoLogInSend = ""; //Строка о пользователе, отправленная на бэк
        strUserInfoLogInSend = QString::number(idLogInRequest) + "/" + log + "|" + pass;
        qDebug()<<strUserInfoLogInSend;
        port = Random::get(3000, 9999);
        qDebug() << "Port: " <<port;
        udpSocket->writeDatagram(strUserInfoLogInSend.toUtf8(), QHostAddress::LocalHost, 25564);
        connect(udpSocket, SIGNAL(readyRead()), this, SLOT(ReadingData()));
    }

}

void Widget::creatingUserWindow(){
    for (int i = 0; i < mainLayout->count(); ++i)
    {
        QWidget *widg = mainLayout->itemAt(i)->widget();
        widg->hide();
    }
    nameMenu = new QLabel();
    nameMenu->setText("Меню");
    nameMenu->setStyleSheet(QString("font-size: %1px").arg(30));
    btnServise1 = new QPushButton();
    btnServise1->setText("Сервис 1");
    connect(btnServise1, SIGNAL(clicked()), this, SLOT(on_buttonServ1_clicked()));
    btnServise2 = new QPushButton();
    btnServise2->setText("Сервис 2");
    btnServise3 = new QPushButton();
    btnServise3->setText("Сервис 3");
    btnLogOut = new QPushButton();
    btnLogOut->setText("Выйти");
    connect(btnLogOut, SIGNAL(clicked()), this, SLOT(on_buttonLogOut_clicked()));
    nameRole = new QLabel();
    switch(roleGet){
        case 1:{
            nameRole->setText("sysadmin");
            break;
        }
        case 5:{
            nameRole->setText("director");
            break;
        }
        default:{
            qDebug() <<"Должность не найдена!";
        }
    }

    mainLayout->addWidget(nameMenu, 0, 4, 1, 2, Qt::AlignCenter);
    mainLayout->setVerticalSpacing(10);
    mainLayout->addWidget(btnServise1, 1, 0, 2, 10, Qt::AlignTop);
    mainLayout->addWidget(btnServise2, 2, 0, 2, 10, Qt::AlignTop);
    mainLayout->addWidget(btnServise3, 3, 0, 2, 10, Qt::AlignTop);
    connect(btnServise3, SIGNAL(clicked()), this, SLOT(on_buttonServ3_clicked()));
    mainLayout->addWidget(btnLogOut, 4, 4, 2, 2);
    mainLayout->addWidget(nameRole, 4, 8, 2, 2);
}

void Widget::on_buttonServ1_clicked(){
    for (int i = 0; i < mainLayout->count(); ++i)
    {
        QWidget *widg = mainLayout->itemAt(i)->widget();
        widg->hide();
    }
    btnBack = new QPushButton();
    btnBack->setText("<<");
    connect(btnBack, SIGNAL(clicked()), this, SLOT(creatingUserWindow()));
    useFileButton = new QPushButton("Выбрать файл");
    fileLabel = new QLabel("");
    connect(useFileButton, SIGNAL(clicked()), this, SLOT(on_useFileButton_clicked()));
    mainLayout->addWidget(btnBack, 0, 0, 1, 2);
    mainLayout->addWidget(useFileButton, 1, 0, 1, 5);
    mainLayout->addWidget(fileLabel, 2, 0, 1, 3);
}

//Окно для ролей с id=2,3 или 4
void Widget::creatingAnotherUserWindow(){
    for (int i = 0; i < mainLayout->count(); ++i)
    {
        QWidget *widg = mainLayout->itemAt(i)->widget();
        widg->hide();
    }
    nameMenu = new QLabel();
    nameMenu->setText("Меню");
    nameMenu->setStyleSheet(QString("font-size: %1px").arg(30));
    btnLogOut = new QPushButton();
    btnLogOut->setText("Выйти");
    connect(btnLogOut, SIGNAL(clicked()), this, SLOT(on_buttonLogOut_clicked()));
    nameRole = new QLabel();
    switch(roleGet){
        case 2:{
            nameRole->setText("stockman");
            break;
        }
        case 3:{
            nameRole->setText("cashier");
            break;
        }
        case 4:{
            nameRole->setText("merchandiser");
            break;
        }
        default:{
            qDebug() <<"Должность не найдена!";
        }
    }
    mainLayout->addWidget(nameMenu, 0, 0, 1, 10, Qt::AlignCenter);
    mainLayout->addWidget(btnLogOut, 1, 4, 1, 2, Qt::AlignCenter);
    mainLayout->addWidget(nameRole, 1, 8, 1, 2, Qt::AlignCenter);
}

void Widget::ReadingDataLogOut(){
    QHostAddress sender;
    quint16 senderPort;
    while(udpSocket->hasPendingDatagrams()){
        QByteArray datagram;
        datagram.resize(udpSocket->pendingDatagramSize());
//        udpSocket->readDatagram(datagram.data(), datagram.size(), &sender, &senderPort);
        udpSocket->readDatagram(datagram.data(), datagram.size(), &sender, &senderPort);
        strUserInfoGet = QString(datagram);

    }
    reqBtnLogOut(strUserInfoGet);
}

void Widget::reqBtnLogOut(QString str){
    qDebug() <<str;
    QString answerLogOut = "";
    for(int i=str.indexOf("/")+1; i<str.length(); i++){
        answerLogOut += str[i];
    }
    if(answerLogOut.toInt() == 1){
        for (int i = 0; i < mainLayout->count(); ++i)
        {
            QWidget *widg = mainLayout->itemAt(i)->widget();
            widg->hide();
        }
        creatingLogInWindow();
    }
}

void Widget::on_buttonLogOut_clicked(){
    for (int i = 0; i < mainLayout->count(); ++i)
    {
        QWidget *widg = mainLayout->itemAt(i)->widget();
        widg->hide();
    }
    strUserInfoLogInSend = "";
    strUserInfoLogInSend = QString::number(idLogOutRequest) + "/" + log + "|" + token;
    connect(udpSocket, SIGNAL(readyRead()), this, SLOT(ReadingDataLogOut()));
    udpSocket->writeDatagram(strUserInfoLogInSend.toUtf8(), QHostAddress::LocalHost, 25564);

}

void Widget::on_buttonServ3_clicked(){
    for (int i = 0; i < mainLayout->count(); ++i)
    {
        QWidget *widg = mainLayout->itemAt(i)->widget();
        widg->hide();
    }
    creatingServ3Window();
}

void Widget::creatingServ3Window(){
    nameMenu = new QLabel();
    nameMenu->setText("Меню");
    nameMenu->setStyleSheet(QString("font-size: %1px").arg(30));
    btnBack = new QPushButton();
    btnBack->setText("<<");
    connect(btnBack, SIGNAL(clicked()), this, SLOT(creatingUserWindow()));
    btnLogOut = new QPushButton();
    btnLogOut->setText("Выйти");
    connect(btnLogOut, SIGNAL(clicked()), this, SLOT(on_buttonLogOut_clicked()));
    btnGenerateReport = new QPushButton();
    btnGenerateReport->setText("Сформировать отчёт о переработках");
    connect(btnGenerateReport, SIGNAL(clicked()), this, SLOT(on_btnGenerateReport_clicked()));
    btnCheckChangeStatusEmployee = new QPushButton();
    btnCheckChangeStatusEmployee->setText("Просмотреть, изменить статус сотрудников");
    connect(btnCheckChangeStatusEmployee, SIGNAL(clicked()), this, SLOT(on_btnCheckChangeStatusEmployee_clicked()));
    mainLayout->addWidget(btnBack, 0, 0, 1, 1, Qt::AlignCenter);
    mainLayout->addWidget(nameMenu, 1, 0, 1, 10, Qt::AlignCenter);
    mainLayout->addWidget(btnLogOut, 2, 0, 2, 10, Qt::AlignTop);
    mainLayout->addWidget(btnGenerateReport, 3, 0, 2, 10, Qt::AlignTop);
    mainLayout->addWidget(btnCheckChangeStatusEmployee, 4, 0, 2, 10, Qt::AlignTop);
}

void Widget::on_btnGenerateReport_clicked(){
    for (int i = 0; i < mainLayout->count(); ++i)
    {
        QWidget *widg = mainLayout->itemAt(i)->widget();
        widg->hide();
    }
    genRep = "";
    for(int i = 1; i<=11; i++){
        genRep = "20/" + log + "|" + token + "?" + QString::number(i);
        udpSocketRep = new QUdpSocket();
        udpSocketRep->writeDatagram(genRep.toUtf8(), QHostAddress::LocalHost, 25564);
        connect(udpSocketRep, SIGNAL(readyRead()), this, SLOT(ReadingDataReport()));
    }


}

void Widget::creatingGenerateReportWindow(QString str){
    btnBack = new QPushButton();
    btnBack->setText("<<");
    connect(btnBack, SIGNAL(clicked()), this, SLOT(on_buttonServ3_clicked()));
    nameGenerateReport = new QLabel();
    nameGenerateReport->setText("Отчёт о переработках");
    nameGenerateReport->setStyleSheet(QString("font-size: %1px").arg(25));
    tableGenerateReport = new QTableWidget( 22, 23 );
    QStringList arrGenerateReport;
    arrGenerateReport << "Фамилия" << "Имя" << "Отчество" << "Роль работника" << "id расписания" << "id расписания работника" << "Часы в понедельник(по расписанию)" << "Часы во вторник(по расписанию)" << "Часы в среду(по расписанию)" << "Часы в четверг(по расписанию)" << "Часы в пятницу(по расписанию)" << "Часы в субботу(по расписанию)" << "Часы в воскресенье(по расписанию)" <<"Всего часов(по расписанию)" <<"id отработанных часов" <<"Отработанные часы в понедельник" <<"Отработанные часы во вторник" <<"Отработанные часы в среду" <<"Отработанные часы в четверг" <<"Отработанные часы в пятницу" <<"Отработанные часы в субботу" <<"Отработанные часы в воскресенье" <<"Всего отработанных часов";
    for (int j = 0 ; j < 23; ++j ) {
        itemGenerateReport = new QTableWidgetItem(arrGenerateReport[j]);
        tableGenerateReport->setHorizontalHeaderItem(j, itemGenerateReport);
        itemGenerateReport->setFlags(Qt::ItemIsEditable);
    }
    tableGenerateReport->horizontalHeader()->setStretchLastSection(true);
    str.remove("200/1|");
    str.replace("*", ";");
    qDebug() << "Строчка: " <<str;
    QString strVr="";
    int rowNumber = 0;
    int colNumber = 0;
    for(int i=0; i<str.length(); i++){
        if(colNumber==23){
            colNumber = 0;
        }
        if(QString(str[i])==";"){

            QTableWidgetItem* itemGenerateReport1 = new QTableWidgetItem(strVr);
            tableGenerateReport->setItem(rowNumber, colNumber, itemGenerateReport1);
            rowNumber++;
            colNumber++;
            strVr="";
        }
        else if(QString(str[i])!=","){
            strVr+=str[i];
        }
        else if(QString(str[i])==","){
            QTableWidgetItem* itemGenerateReport1 = new QTableWidgetItem(strVr);
            tableGenerateReport->setItem(rowNumber, colNumber, itemGenerateReport1);
            colNumber++;
            strVr="";
        }

    }
    /*
    for (int i = 0; i < 3; i++ ) {
        for(int j = 0; j < 6; j++){
            QTableWidgetItem* itemGenerateReport1 = new QTableWidgetItem();
             qDebug()<<tableGenerateReport->takeItem(i,j)->text();
        }
    }
    */

    nameInputGenerateReport = new QLabel();
    nameInputGenerateReport->setText("Для изменения введите id работника, количество отработанных часов за неделю и причину изменения через запятую");
    nameInputGenerateReport->setStyleSheet(QString("font-size: %1px").arg(18));
    inputGenerateReport = new QLineEdit();
    btnInputChange = new QPushButton();
    btnInputChange->setText("Внести изменения");
    connect(btnInputChange, SIGNAL(clicked()), this, SLOT(on_btnInputChangeGenerateReport_clicked()));
    btnSendReport = new QPushButton();
    btnSendReport->setText("Отправить отчёт в ГК");
    connect(btnSendReport, SIGNAL(clicked()), this, SLOT(on_btnSendGenerateReport_clicked()));
    mainLayout->addWidget(btnBack, 0, 0, 1, 1);
    mainLayout->addWidget(nameGenerateReport, 1, 0, 1, 10, Qt::AlignCenter);
    mainLayout->addWidget(tableGenerateReport, 2, 0, 1, 10);
    mainLayout->addWidget(nameInputGenerateReport, 3, 0, 1, 10, Qt::AlignCenter);
    mainLayout->addWidget(inputGenerateReport, 4, 0, 1, 10);
    mainLayout->addWidget(btnInputChange, 5, 3, 1, 4);
    mainLayout->addWidget(btnSendReport, 6, 3, 1, 4);
}

void Widget::ReadingDataReport(){
    QHostAddress sender;
    quint16 senderPort;
    while(udpSocketRep->hasPendingDatagrams()){
        QByteArray datagram;
        datagram.resize(udpSocketRep->pendingDatagramSize());
//        udpSocket->readDatagram(datagram.data(), datagram.size(), &sender, &senderPort);
        udpSocketRep->readDatagram(datagram.data(), datagram.size(), &sender, &senderPort);
        strGetReportData = QString(datagram);
        creatingGenerateReportWindow(strGetReportData);
    }
}

void Widget::on_btnSendGenerateReport_clicked(){

}

void Widget::ReadingDataReportSend(){
    QHostAddress sender;
    quint16 senderPort;
    while(udpSocketRep1->hasPendingDatagrams()){
        QByteArray datagram;
        datagram.resize(udpSocketRep1->pendingDatagramSize());
//        udpSocket->readDatagram(datagram.data(), datagram.size(), &sender, &senderPort);
        udpSocketRep1->readDatagram(datagram.data(), datagram.size(), &sender, &senderPort);
        strGetReportData = QString(datagram);
        creatingGenerateReportWindow(strGetReportData);
    }
}

void Widget::on_btnCheckChangeStatusEmployee_clicked(){
    for (int i = 0; i < mainLayout->count(); ++i)
    {
        QWidget *widg = mainLayout->itemAt(i)->widget();
        widg->hide();
    }
    creatingCheckChangeStatusEmployeeWindow();
}

void Widget::creatingCheckChangeStatusEmployeeWindow(){
    btnBack = new QPushButton();
    btnBack->setText("<<");
    connect(btnBack, SIGNAL(clicked()), this, SLOT(on_buttonServ3_clicked()));
    nameTableSick = new QLabel();
    nameTableSick->setText("Статус больничных");
    nameTableSick->setStyleSheet(QString("font-size: %1px").arg(25));
    nameTableVocation = new QLabel();
    nameTableVocation->setText("Статус отпусков");
    nameTableVocation->setStyleSheet(QString("font-size: %1px").arg(25));
    nameTableFired = new QLabel();
    nameTableFired->setText("Статус увольнений");
    nameTableFired->setStyleSheet(QString("font-size: %1px").arg(25));
    nameInputCheckChangeStatusEmployee = new QLabel();
    nameInputCheckChangeStatusEmployee->setText("Для добавления/изменения введите id работника, статус, дату начала, дату окончания, подтверждение(при наличии) через запятую");
    nameInputCheckChangeStatusEmployee->setStyleSheet(QString("font-size: %1px").arg(18));
    inputCheckChangeStatusEmployee = new QLineEdit();
    btnSendChange = new QPushButton();
    btnSendChange->setText("Внести изменения");
    connect(btnSendChange, SIGNAL(clicked()), this, SLOT(on_btnInputChangeCheckChangeStatusEmployee_clicked()));


    tableSick = new QTableWidget(3,4);
    QStringList listSick;
    listSick << "id работника" << "Дата начала больничного" << "Дата окончания больничного" << "Подтверждение";
    QTableWidgetItem* itemSick;
    for (int j = 0 ; j < 4; ++j ) {
        itemSick = new QTableWidgetItem(listSick[j]);
        tableSick->setHorizontalHeaderItem(j, itemSick);
        itemSick->setFlags(Qt::ItemIsEditable);
    }
    tableSick->horizontalHeader()->setStretchLastSection(true);


    tableVocation = new QTableWidget(3,4);
    QStringList listVocation;
    listVocation << "id работника" << "Дата начала отпуска" << "Дата окончания отпуска" << "Подтверждение";
    QTableWidgetItem* itemVocation;
    for (int j = 0 ; j < 4; ++j ) {
        itemVocation = new QTableWidgetItem(listVocation[j]);
        tableVocation->setHorizontalHeaderItem(j, itemVocation);
        itemVocation->setFlags(Qt::ItemIsEditable);
    }
    tableVocation->horizontalHeader()->setStretchLastSection(true);


    tableFired = new QTableWidget(3,2);
    QStringList listFired;
    listFired << "id работника" << "Дата увольнения";
    QTableWidgetItem* itemFired;
    for (int j = 0 ; j < 2; ++j ) {
        itemFired = new QTableWidgetItem(listFired[j]);
        tableFired->setHorizontalHeaderItem(j, itemFired);
        itemFired->setFlags(Qt::ItemIsEditable);
    }
    tableFired->horizontalHeader()->setStretchLastSection(true);


    mainLayout->addWidget(btnBack, 0, 0, 1, 1);
    mainLayout->addWidget(nameTableSick, 1, 0, 1, 10, Qt::AlignCenter);
    mainLayout->addWidget(tableSick, 2, 0, 1, 10);
    mainLayout->addWidget(nameTableVocation, 3, 0, 1, 10, Qt::AlignCenter);
    mainLayout->addWidget(tableVocation, 4, 0, 1, 10);
    mainLayout->addWidget(nameTableFired, 5, 0, 1, 10, Qt::AlignCenter);
    mainLayout->addWidget(tableFired, 6, 0, 1, 10);
    mainLayout->addWidget(nameInputCheckChangeStatusEmployee, 7, 0, 1, 10, Qt::AlignCenter);
    mainLayout->addWidget(inputCheckChangeStatusEmployee, 8, 0, 1, 10);
    mainLayout->addWidget(btnSendChange, 9, 3, 1, 4);
}

void Widget::on_btnInputChangeCheckChangeStatusEmployee_clicked(){
    inpChangesCheckChangeStatusEmployee = inputCheckChangeStatusEmployee->text();
    QString sendChanges = "";
    QStringList changes;
    changes = inpChangesCheckChangeStatusEmployee.split(",");
    if(changes.length()==0){
        QMessageBox::information(this, "", "Поле для изменений не заполнено!");
    }
    else if(changes.length()==4 || changes.length()==5){
        QMessageBox::information(this, "", "Данные успешно отправлены!");
        inputCheckChangeStatusEmployee->clear();
    }
    else QMessageBox::information(this, "", "Проверьте ввёденные данные!");
}

Widget::~Widget()
{
    delete ui;
}

