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

    QUdpSocket* socket;

    QSqlDatabase db;

    void insertQueryIntoBody(QString request, QString &body, int fields);

    void test();

    void initFrame();

    void processRequest(QByteArray data);

    void checkOutput(QByteArray data);
    void statusOutput(QByteArray data);

    void checkInsertChanges(QByteArray data);
    void statusInsertChanges(QByteArray data);

    //void checkOutput(QByteArray data);

    void roleInsertCSV(QByteArray data);
    void employInsertCSV(QByteArray data);

    void insertSchedule(QByteArray data);

    //QByteArray parse(QByteArray &data, int index);
public slots:
    void request();
private:
    Ui::Widget *ui;
};
#endif // WIDGET_H
