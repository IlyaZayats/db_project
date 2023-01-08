#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QtWidgets>
#include <QUdpSocket>

QT_BEGIN_NAMESPACE
namespace Ui { class Widget; }
QT_END_NAMESPACE

class Widget : public QWidget
{
    Q_OBJECT

public:
    QString strGetReportData1;
    QString genSendRep;
    QUdpSocket* udpSocketRep;
    QUdpSocket* udpSocketRep1;
    QUdpSocket* udpSocketRep2;
    QLabel* error;
    QString strUserInfoLogInSend;
    QUdpSocket *udpSocket;
    int idLogInRequest;
    int idLogOutRequest;
    int port;
    QString token;
    int roleGet;
    QString log;
    QString pass;
    QWidget* mainWidget;
    QGridLayout* mainLayout;
    QGridLayout* grid;
    //Окно авторизациий
    QString strUserInfoGet;
    QLabel* nameApp;
    QLabel* nameAuth;
    QLabel* login;
    QLineEdit* inputLogin;
    QLabel* password;
    QLineEdit* inputPassword;
    QPushButton* buttonAuth;
    //Окно трёх сервисов
    QLabel* nameMenu;
    QPushButton* btnServise1;
    QPushButton* btnServise2;
    QPushButton* btnServise3;
    QPushButton* btnLogOut;
    QLabel* nameRole;
    //Окно третьего сервиса
    QPushButton* btnBack;
    QPushButton* btnGenerateReport;
    QPushButton* btnCheckChangeStatusEmployee;
    //Окно формирования отчёта о переработках
    QLabel* nameGenerateReport;
    QTableWidget* tableGenerateReport;
    QTableWidgetItem* itemGenerateReport;
    QLabel* nameInputGenerateReport;
    QLineEdit* inputGenerateReport;
    QPushButton* btnInputChange;
    QPushButton* btnSendReport;
    //Кнопка отправления изменений при формировании отчёта
    QString inpChangesGenerateReport;
    //Кнопка изменения статуса сотрудника
    QString inpChangesCheckChangeStatusEmployee;
    //Окно просмотра и изменения статуса сотрудника
    QLabel* nameCheckChangeStatusEmployee;
    QTableWidget* tableSick;
    QTableWidget* tableVocation;
    QTableWidget* tableFired;
    QTableWidgetItem* itemCheckChangeStatusEmployee;
    QLabel* nameInputCheckChangeStatusEmployee;
    QLineEdit* inputCheckChangeStatusEmployee;
    QPushButton* btnSendChange;
    QLabel* nameTableSick;
    QLabel* nameTableVocation;
    QLabel* nameTableFired;
    //Окно выбора файла
    QLabel* fileLabel;
    QPushButton* useFileButton;
    QString genRep;
    QString strGetReportData;
    QString genRep1;
    Widget(QWidget *parent = nullptr);
    ~Widget();


private slots:
    void ReadingDataReportSend();
    bool createConnection();
    void creatingLogInWindow();
    void creatingAnotherUserWindow();
    void on_buttonAuth_clicked();
    void ReadingData();
    void ReadingDataLogOut();
    void reqBtnLogOut(QString str);
    void reqBtnAuth(QString str);
    bool checkPassRegExp(QRegularExpression reg, QString str1, QString str2);
    void creatingUserWindow();
    void on_buttonServ1_clicked();
    void on_buttonLogOut_clicked();
    void on_buttonServ3_clicked();
    void creatingServ3Window();
    void on_btnGenerateReport_clicked();
    void creatingGenerateReportWindow(QString str);
    void on_btnSendGenerateReport_clicked();
    void on_btnCheckChangeStatusEmployee_clicked();
    void creatingCheckChangeStatusEmployeeWindow();
    void on_btnInputChangeCheckChangeStatusEmployee_clicked();
    void on_useFileButton_clicked();
    void ReadingDataFile();
    void ReadingDataReport();
private:
    Ui::Widget *ui;
};
#endif // WIDGET_H
