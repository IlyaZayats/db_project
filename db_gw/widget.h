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

QT_BEGIN_NAMESPACE
namespace Ui { class Widget; }
QT_END_NAMESPACE

struct session{
    int port;
    QString token;
};

class Widget : public QWidget
{
    Q_OBJECT

public:
    QVector<session> activeSessions ;

    Widget(QWidget *parent = nullptr);
    ~Widget();

    QListWidget* mainWidget;
    QListWidgetItem *item;

    QUdpSocket* socket;

    void initFrame();
    void processRequest(QNetworkDatagram datagram);

public slots:
    void request();

private:
    Ui::Widget *ui;
};
#endif // WIDGET_H
