#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QUdpSocket>
#include <QtSql>
#include <QGroupBox>
#include <QBoxLayout>
#include <QSqlDatabase>
#include <QListWidget>
#include <QNetworkDatagram>
#include <QRandomGenerator>
#include <QChar>
#include <QDate>

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

    QTimer* timer_midnight;
    QTimer* timer_five_min;

    QUdpSocket* socket;

    QSqlDatabase db;

    QString getCurrentWeek();
    QString getCurrentDay();
    void initNewWeek();
    void initFrame();
    void processRequest(QByteArray data);
    void login(QByteArray data);
    void authRequest(QByteArray data);
    void logOut(QByteArray data);
    void sickCheck(QString login, QString employ_id);
    //void activeApprove(QByteArray data);

    //QByteArray parse(QByteArray &data, int index);
public slots:
    void request();
    void notification();
    void force();
private:
    Ui::Widget *ui;
};
#endif // WIDGET_H
