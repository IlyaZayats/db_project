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

Widget::Widget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Widget)
{
    idLogInRequest = 10;
    idLogOutRequest = 12;
    token = "FGQu7SqSEbMi";
    grid = new QGridLayout(this);
    mainWidget = new QWidget();
    mainWidget->resize(800,600);
    mainLayout = new QGridLayout(mainWidget);
    grid->addWidget(mainWidget);
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
    buttonAuth->setText("Войти");
    connect(buttonAuth, SIGNAL(clicked()), this, SLOT(on_buttonAuth_clicked()));
    mainLayout->addWidget(nameApp, 0, 3, 1, 3,  Qt::AlignTop);
    mainLayout->addWidget(nameAuth, 1, 3, 1, 3, Qt::AlignCenter);
    mainLayout->addWidget(login, 2, 0, 1, 1, Qt::AlignTop);
    mainLayout->addWidget(inputLogin, 2, 1, 1, 7);
    mainLayout->addWidget(password, 3, 0, 1, 1, Qt::AlignTop);
    mainLayout->addWidget(inputPassword, 3, 1, 1, 7);
    mainLayout->addWidget(buttonAuth, 4, 1, 1, 2, Qt::AlignTop);
}


void Widget::on_useFileButton_clicked()
{
    QString filter = "*.csv";
    QString fileName = QFileDialog::getOpenFileName(this, tr("Open File"), filter, QDir::homePath());
    QFile file(fileName);
    if (!file.open(QFile::ReadOnly | QFile::Text)){
        QMessageBox::warning(this, "title", "file not open");
    }
    qDebug() << fileName;
    fileName.remove(0,fileName.lastIndexOf("/")+1);
    fileLabel->setText(fileName);
    QString data = file.readAll();
    qDebug() << data;
    //data.remove(QChar('\n'));
    QString q1 = "\n";
    data.remove(0, data.indexOf(q1)+1); //удаляю первую строку
    data.replace("\n", ";");
    data+=";";
    qDebug() << data;


//    QGridLayout* grid = new QGridLayout(this);
//    mainWidget = new QWidget();
//    mainLayout = new QGridLayout(mainWidget);
//    grid->addWidget(mainWidget,0,0,11,1);
//    QLabel* fileTextName = new QLabel(fileName);
//    grid->addWidget(fileTextName, 11, 0);
}


bool Widget::checkPassRegExp(QRegularExpression reg, QString str1, QString str2){
    auto match1 = reg.match(str1);
    auto match2 = reg.match(str2);
    if(match1.hasMatch() && match2.hasMatch() && str2.length()>=8 && str2.length()<=20) return true;
    else{
        QMessageBox::information(this, "", "Неверно введённые данные!");
        return false;
    }
}

void Widget::on_buttonAuth_clicked(){
    log = inputLogin->text();
    pass = inputPassword->text();
    QRegularExpression reg("[0-9]|[A-Z]|[a-z]|[!@#$%^&*?]");
    if(checkPassRegExp(reg, log, pass)){
        for (int i = 0; i < mainLayout->count(); ++i)
        {
            QWidget *widg = mainLayout->itemAt(i)->widget();
            widg->hide();
        }
        /*Блок отправки данных о логине и пароле и получении с бэка инфы о пользователе*/
        QString strUserInfoLogInSend = ""; //Строка о пользователе, отправленная на бэк
        strUserInfoLogInSend = QString::number(idLogInRequest) + "/" + log + "|" + pass;
        qDebug()<<strUserInfoLogInSend;
        QString strUserInfoGet = ""; //Строка о пользователе, полученная с бэка
        strUserInfoGet="0/1|1|FGQu7SqSEbMi";
        QString strAnswerGet = "";
        QString strRoleGet = "";
        for(int i=strUserInfoGet.indexOf("/")+1; i<strUserInfoGet.indexOf("|"); i++){
            strAnswerGet += strUserInfoGet[i];
        }
        for(int i=strUserInfoGet.indexOf("|")+1; i<strUserInfoGet.lastIndexOf("|"); i++){
            strRoleGet += strUserInfoGet[i];
        }
        qDebug() << strAnswerGet << " " <<strRoleGet;
        int answerGet = strAnswerGet.toInt();
        roleGet = strRoleGet.toInt();
        if(answerGet == 1){
            if(roleGet == 1 || roleGet == 5){
                creatingUserWindow();
            }
            else{
                creatingAnotherUserWindow();
            }
        }
        else{
            QMessageBox::information(this, "", "Ответ с сервера не получен!");
        }

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

void Widget::on_buttonLogOut_clicked(){
    for (int i = 0; i < mainLayout->count(); ++i)
    {
        QWidget *widg = mainLayout->itemAt(i)->widget();
        widg->hide();
    }
    QString strUserInfoLogInSend = "";
    strUserInfoLogInSend = QString::number(idLogOutRequest) + "/" + log + "|" + token;  //Отправка запроса на выход из учётной записи пользователя
    creatingLogInWindow();
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
    creatingGenerateReportWindow();
}

void Widget::creatingGenerateReportWindow(){
    btnBack = new QPushButton();
    btnBack->setText("<<");
    connect(btnBack, SIGNAL(clicked()), this, SLOT(on_buttonServ3_clicked()));
    nameGenerateReport = new QLabel();
    nameGenerateReport->setText("Отчёт о переработках");
    nameGenerateReport->setStyleSheet(QString("font-size: %1px").arg(25));
    tableGenerateReport = new QTableWidget( 3, 6 );
    QStringList arrGenerateReport;
    arrGenerateReport << "id работника" << "ФИО" << "Должность" << "Необходимое количество часов по ставке" << "Количество отработанных часов" << "Причина изменения(при наличии)";
    for (int j = 0 ; j < 6; ++j ) {
        itemGenerateReport = new QTableWidgetItem(arrGenerateReport[j]);
        tableGenerateReport->setHorizontalHeaderItem(j, itemGenerateReport);
    }
    tableGenerateReport->horizontalHeader()->setStretchLastSection(true);
    for (int i = 0; i < 3; i++ ) {
        for(int j = 0; j < 6; j++){
            QTableWidgetItem* itemGenerateReport1 = new QTableWidgetItem("a"+QString::number(i)+"b"+QString::number(j));
            tableGenerateReport->setItem(i, j, itemGenerateReport1);
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

void Widget::on_btnInputChangeGenerateReport_clicked(){
    inpChangesGenerateReport = inputGenerateReport->text();
    QString sendChanges = "";
    QStringList changes;
    changes = inpChangesGenerateReport.split(",");
    if(changes.length()==0){
        QMessageBox::information(this, "", "Поле для изменений не заполнено!");
    }
    else if(changes.length()==3){
        QMessageBox::information(this, "", "Данные успешно отправлены!");
        inputGenerateReport->clear();
    }
    else QMessageBox::information(this, "", "Проверьте ввёденные данные!");
}

void Widget::on_btnSendGenerateReport_clicked(){


    QFile f1;
          f1.setFileName("f1.csv");
          f1.open(QIODevice::WriteOnly | QIODevice::Text);
          QString strTable = "";
          QString strTableElem="";
          for (int i = 0; i < 3; i++ ) {
              for(int j = 0; j < 6; j++){
                   strTableElem = tableGenerateReport->takeItem(i,j)->text();
                   strTable += strTableElem + ",";
                   QTableWidgetItem* itemGenerateReport1 = new QTableWidgetItem(strTableElem);
                   tableGenerateReport->setItem(i, j, itemGenerateReport1);
              }
              strTable.resize(strTable.size() - 1);

              strTable += ";";
          }
          QTextStream stream( &f1 );
          stream << strTable;
                f1.close();
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
    nameCheckChangeStatusEmployee = new QLabel();
    nameCheckChangeStatusEmployee->setText("Статус больничных, отпусков, увольнений");
    nameCheckChangeStatusEmployee->setStyleSheet(QString("font-size: %1px").arg(25));
    tableCheckChangeStatusEmployee = new QTableWidget( 3, 7 );
    QStringList arrGenerateReport;
    arrGenerateReport << "id работника" << "ФИО" << "Должность" << "Статус" << "Дата начала" << "Дата окончания" << "Подтверждение";
    for (int j = 0 ; j < 7; ++j ) {
        itemCheckChangeStatusEmployee = new QTableWidgetItem(arrGenerateReport[j]);
        tableCheckChangeStatusEmployee->setHorizontalHeaderItem(j, itemCheckChangeStatusEmployee);
    }
    tableCheckChangeStatusEmployee->horizontalHeader()->setStretchLastSection(true);
    nameInputCheckChangeStatusEmployee = new QLabel();
    nameInputCheckChangeStatusEmployee->setText("Для добавления/изменения введите id работника, статус, дату начала, дату окончания, подтверждение(при наличии) через запятую");
    nameInputCheckChangeStatusEmployee->setStyleSheet(QString("font-size: %1px").arg(18));
    inputCheckChangeStatusEmployee = new QLineEdit();
    btnSendChange = new QPushButton();
    btnSendChange->setText("Внести изменения");
    connect(btnSendChange, SIGNAL(clicked()), this, SLOT(on_btnInputChangeCheckChangeStatusEmployee_clicked()));
    mainLayout->addWidget(btnBack, 0, 0, 1, 1);
    mainLayout->addWidget(nameCheckChangeStatusEmployee, 1, 0, 1, 10, Qt::AlignCenter);
    mainLayout->addWidget(tableCheckChangeStatusEmployee, 2, 0, 1, 10);
    mainLayout->addWidget(nameInputCheckChangeStatusEmployee, 3, 0, 1, 10, Qt::AlignCenter);
    mainLayout->addWidget(inputCheckChangeStatusEmployee, 4, 0, 1, 10);
    mainLayout->addWidget(btnSendChange, 5, 3, 1, 4);
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

