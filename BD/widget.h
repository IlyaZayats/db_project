#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QtWidgets>

QT_BEGIN_NAMESPACE
namespace Ui { class Widget; }
QT_END_NAMESPACE

class Widget : public QWidget
{
    Q_OBJECT

public:
    int idLogInRequest;
    int idLogOutRequest;
    QString token;
    int roleGet;
    QString log;
    QString pass;
    QWidget* mainWidget;
    QGridLayout* mainLayout;
    QGridLayout* grid;
    //Окно авторизациий
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
    QTableWidget* tableCheckChangeStatusEmployee;
    QTableWidgetItem* itemCheckChangeStatusEmployee;
    QLabel* nameInputCheckChangeStatusEmployee;
    QLineEdit* inputCheckChangeStatusEmployee;
    QPushButton* btnSendChange;
    //Окно выбора файла
    QLabel* fileLabel;
    QPushButton* useFileButton;
    Widget(QWidget *parent = nullptr);
    ~Widget();


private slots:
    void creatingLogInWindow();
    void creatingAnotherUserWindow();
    void on_buttonAuth_clicked();
    bool checkPassRegExp(QRegularExpression reg, QString str1, QString str2);
    void creatingUserWindow();
    void on_buttonLogOut_clicked();
    void on_buttonServ3_clicked();
    void creatingServ3Window();
    void on_btnGenerateReport_clicked();
    void creatingGenerateReportWindow();
    void on_btnInputChangeGenerateReport_clicked();
    void on_btnSendGenerateReport_clicked();
    void on_btnCheckChangeStatusEmployee_clicked();
    void creatingCheckChangeStatusEmployeeWindow();
    void on_btnInputChangeCheckChangeStatusEmployee_clicked();
    void on_useFileButton_clicked();
private:
    Ui::Widget *ui;
};
#endif // WIDGET_H
