#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QUdpSocket>
#include <QGridLayout>
#include <QScrollArea>
#include <QLabel>
#include <QNetworkDatagram>
#include <QListWidget>
#include <QGroupBox>
#include <QDateTime>
#include <QLineEdit>
#include <QLabel>
#include <QPushButton>

QT_BEGIN_NAMESPACE
namespace Ui { class Widget; }
QT_END_NAMESPACE

class Widget : public QWidget
{
    Q_OBJECT

public:
    QListWidget* mainWidget;
    QListWidgetItem *item;

    QLineEdit* requestInput;
    QLineEdit* portInput;

    QUdpSocket* socket;
    void sendRequest();

    Widget(QWidget *parent = nullptr);
    ~Widget();

    void initFrame();

public slots:
    void buttonPressed();
    void getResponse();
private:
    Ui::Widget *ui;
};
#endif // WIDGET_H

