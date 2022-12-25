#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QUdpSocket>
#include <QtSql>
#include <QGroupBox>
#include <QBoxLayout>
#include <QListWidget>
#include <QNetworkDatagram>
#include <QRandomGenerator>
#include <QChar>

QT_BEGIN_NAMESPACE
namespace Ui { class Widget; }
QT_END_NAMESPACE

class Widget : public QWidget
{
    Q_OBJECT

public:
    Widget(QWidget *parent = nullptr);
    ~Widget();
    QListWidget* mainWidget;
    QListWidgetItem *item;

    QUdpSocket* socket;

    QSqlDatabase db;

    void initFrame();
    void processRequest(QByteArray data);
    void login(QByteArray data);
    void authRequest(QByteArray data);
    void logOut(QByteArray data);

    //QByteArray parse(QByteArray &data, int index);
public slots:
    void request();
private:
    Ui::Widget *ui;
};
#endif // WIDGET_H
