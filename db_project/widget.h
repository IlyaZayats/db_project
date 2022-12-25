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
    QWidget* mainWidget;
    QGroupBox* loginWidget;
    QGridLayout* mainLayout;

    QLineEdit* loginInput;
    QLineEdit* pwdInput;
    QLabel* loginError;

    QUdpSocket* request;
    void sendRequset();

    QUdpSocket* response;

    Widget(QWidget *parent = nullptr);
    ~Widget();
    void initLogin();
    void hideLogin();
public slots:
    void loginPressed();
    void getResponse();
private:
    Ui::Widget *ui;
};
#endif // WIDGET_H
