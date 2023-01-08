#include "widget.h"
#include "./ui_widget.h"

QRegularExpression re_login("^[a-zA-Z0-9]+$");
QRegularExpressionValidator v_login(re_login, 0);
QIntValidator v_int;

Widget::Widget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Widget)
{
    ui->setupUi(this);

    socket = new QUdpSocket(this);
    socket->bind(25565);

    timer_midnight = new QTimer(this);
    timer_five_min = new QTimer(this);

    initFrame();
}

void Widget::initFrame(){
    QGridLayout* gl = new QGridLayout(this);
    QGroupBox* w = new QGroupBox("Log: ");
    gl->addWidget(w);
    QGridLayout* glw = new QGridLayout(w);
    mainWidget = new QListWidget();
    mainWidget->setFlow(QListView::TopToBottom);
    glw->addWidget(mainWidget);
    db = QSqlDatabase::addDatabase("QPSQL", "db_auth");
    db.setHostName("localhost");
    db.setDatabaseName("postgres");
    db.setUserName("postgres");
    db.setPassword("12345");
    bool ok = db.open();
    if(!ok){
        item = new QListWidgetItem(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") + " DB connection error", mainWidget);
    }
    connect(timer_midnight, SIGNAL(timeout()), this, SLOT(notification()));
    connect(timer_five_min, SIGNAL(timeout()), this, SLOT(force()));
    timer_five_min->setSingleShot(true);
    timer_midnight->start(1000);
    connect(socket, SIGNAL(readyRead()), SLOT(request()));
}

void Widget::notification(){
    QString cur = QDateTime::currentDateTime().toString("hh:mm:ss");
    if(cur == "00:00:00" /*"17:04:30"*/){
        item = new QListWidgetItem(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") + " Midnight notification", mainWidget);
        QSqlQuery query(db);
        //query.prepare("UPDATE Auth SET approved=false;");
        //query.exec();
        query.prepare("SELECT port FROM Auth WHERE active=true;");
        query.exec();
        query.first();
        if(query.isValid()){
            item = new QListWidgetItem(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") + " There are " + QString::number(query.size()) + " active sessions", mainWidget);
            do{
                QString port = query.value(0).toString();
                socket->writeDatagram(("15/"+port).toUtf8(), QHostAddress::LocalHost,25564);
            }while(query.next());
        } else {
            item = new QListWidgetItem(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") + " There are no active sessions", mainWidget);
        }
        timer_five_min->start(1000*60*5);
        //timer_five_min->start(1000);
    }
}

void Widget::sickCheck(QString login, QString employ_id){
    QSqlQuery q_weeks(db), q_sick(db), q_record(db);
    q_weeks.prepare("SELECT id, start_day, end_day from Weeks");
    q_weeks.exec();
    q_sick.prepare("SELECT id, start_date, end_date, approved FROM Sick WHERE id=(SELECT max(id) FROM Sick WHERE employ_id="+employ_id+") AND approved=false;");
    q_sick.exec();
    q_sick.first();
    if(q_sick.isValid()){
        QString sick_id = q_sick.value(0).toString();
        QString start_date_string = q_sick.value(1).toString();
        QString end_date_string = q_sick.value(2).toString();
        bool approved = q_sick.value(3).toBool();

        QDate start_date = QDate::fromString(start_date_string, "yyyy-MM-dd");
        QDate end_date = QDate::fromString(end_date_string, "yyyy-MM-dd");


        QString today_s = QDate::currentDate().toString("yyyy-MM-dd");
        QDate today = QDate::fromString(today_s, "yyyy-MM-dd");

        //QDate deadline = end_date.addDays(1);
        QDate deadline = end_date.addDays(6);

        if(today>deadline && !approved){
            while(q_weeks.next()){
                QString week_id = q_weeks.value(0).toString();
                QString start_week_string = q_weeks.value(1).toString();
                QString end_week_string = q_weeks.value(2).toString();

                qDebug() << week_id;

                QDate start_week = QDate::fromString(start_week_string, "yyyy-MM-dd");
                QDate end_week = QDate::fromString(end_week_string, "yyyy-MM-dd");

                QDate temp_sick = start_date;
                while(temp_sick<=end_date){
                    QDate temp_week = start_week;
                    while(temp_week<=end_week){
                        if(temp_sick==temp_week){
                            item = new QListWidgetItem(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") + " force: Debug: temp_sick=" + temp_sick.toString("yyyy-MM-dd") + " temp_week=" + temp_week.toString("yyyy-MM-dd") + " week_id=" + week_id, mainWidget);
                            QString day = temp_sick.toString("dddd").toLower();
                            q_record.prepare("SELECT id,"+day+" FROM Schedule WHERE week_id="+week_id+" AND employ_id="+employ_id+";");
                            q_record.exec();
                            item = new QListWidgetItem(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") + " force: Debug: " + q_record.lastQuery(), mainWidget);
                            q_record.first();
                            QString schedule_id = q_record.value(0).toString();
                            int schedule_day_h = q_record.value(1).toInt();
                            item = new QListWidgetItem(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") + " force: Debug: schedule_day_h" + QString::number(schedule_day_h), mainWidget);
                            q_record.prepare("SELECT hours FROM Record WHERE schedule_id="+schedule_id+";");
                            q_record.exec();
                            q_record.first();
                            int sum = q_record.value(0).toInt() - (int)schedule_day_h;
                            q_record.prepare("UPDATE Record SET "+day+"=-"+QString::number(schedule_day_h)+", hours="+QString::number(sum)+" WHERE schedule_id="+schedule_id+";");
                            q_record.exec();
                            item = new QListWidgetItem(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") + " force: Updated record for" + login, mainWidget);
                        }
                        temp_week = temp_week.addDays(1);
                    }
                    temp_sick = temp_sick.addDays(1);
                }
            }
            q_record.prepare("UPDATE Sick SET approved=true WHERE id="+sick_id+";");
            q_record.exec();
        }
    } else {
        item = new QListWidgetItem(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") + " force: sick record doesnt exist for " + login, mainWidget);
    }
}





void Widget::request(){
    while(socket->hasPendingDatagrams()) {
        QNetworkDatagram datagram = socket->receiveDatagram();
        QByteArray replyData = datagram.data();
        item = new QListWidgetItem(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") + " Request: " + datagram.data(), mainWidget);
        processRequest(replyData);
        //socket->writeDatagram(replyData, QHostAddress::LocalHost, 22564);
        //item = new QListWidgetItem(QDateTime::currentDateTime().toString("hh:ss:mm") + " Response: " + replyData, mainWidget);

    }
}

QByteArray parse(QByteArray &data, int index){
    QByteArray r;
    for(int i=0; i<index; i++){
        r.push_back(data[i]);
    }
    data.remove(0, index+1);
    return r;
}

QString getRandomString(){
   const QString possibleCharacters("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789");
   const int randomStringLength = 12;
   QString randomString;
   for(int i=0; i<randomStringLength; ++i)
   {
       int index = rand() % possibleCharacters.length();
       //qDebug() << index;
       QChar nextChar = possibleCharacters.at(index);
       randomString.push_back(nextChar);
   }
   return randomString;
}

//output 16/answer|port
//void Widget::activeApprove(QByteArray data){
//    QString login = parse(data,data.indexOf("|"));
//    QString port = data;
//    QString body = "16/";
//    QSqlQuery query;
//    int pos=0;
//    if(v_login.validate(login,pos) && v_int.validate(port, pos)){
//        query.prepare("SELECT * from Auth WHERE login='"+login+"';");
//        query.exec();
//        query.first();
//        if(query.isValid()){
//            query.prepare("UPDATE Auth SET approved=true WHERE login='"+login+"';");
//            item = new QListWidgetItem(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") + " Approved session for " + login, mainWidget);
//        } else {
//            item = new QListWidgetItem(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") + " Can not approve session for " + login, mainWidget);
//            body+="0|";
//        }
//    } else {
//        item = new QListWidgetItem(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") + " Valid error in activeApprove" , mainWidget);
//        body+="-1|";
//    }
//    socket->writeDatagram((body+port).toUtf8(), QHostAddress::LocalHost,25564);
//}

void Widget::login(QByteArray data){
    int index = data.indexOf("|");
    if(index!=-1){
        QString login = parse(data,data.indexOf("|"));
        QByteArray pwd_raw = parse(data, data.indexOf("|"));
        int pos = 0;
        if(v_login.validate(login,pos)){
            //qDebug() << login;
            QByteArray pwd = QCryptographicHash::hash(pwd_raw, QCryptographicHash::Md5);
            QString token = getRandomString();
//            qDebug() << login << Qt::endl;
//            qDebug() << pwd << Qt::endl;
//            qDebug() << token << Qt::endl;
            QSqlQuery query = QSqlQuery(db);
            query.prepare("SELECT employ_id FROM Auth WHERE login='" + login + "' AND pwd='" + QString::fromUtf8(pwd_raw) +"' AND status=1;");
            //query.prepare();
            //query.bindValue(":login", login);
            //query.bindValue(":pwd", pwd_raw);
            //query.exec("SELECT * FROM Auth");
            query.exec();
            query.first();
            //qDebug() << result << Qt::endl;
            QString body = "11/";
            QString role = "-1";
            if(query.value(0).toString().isEmpty()){
                item = new QListWidgetItem(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") + " User with login " + login
                                           + " does not exist with recieved pwd", mainWidget);
                body+="0|";
            } else {
                QString employ_id = query.value("employ_id").toString();
                query.prepare("SELECT role_id FROM Employ WHERE id='"+employ_id+"';");
                query.exec();
                query.first();
                role = query.value("role_id").toString();
                item = new QListWidgetItem(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") + " User with login " + login
                                           + " authorised", mainWidget);
                body += "1|";
                QString updated_at = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.ms");
                query.prepare("UPDATE Auth SET token='" + token + "', active=true, updated_at='"+updated_at+"', port='"+QString::number(data.toInt())+"' WHERE login='" + login +"'");
                query.exec();
                //qDebug() << query.lastQuery();
            }
            body+=token+"|"+QString::number(data.toInt())+"|"+role;
            socket->writeDatagram(body.toUtf8(),QHostAddress::LocalHost,25564);
        } else {
            item = new QListWidgetItem(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") + " Invalid login", mainWidget);
        }
    } else {
        item = new QListWidgetItem(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") + " Parsing error", mainWidget);
    }
}

//Auth req
void Widget::authRequest(QByteArray data){
    int pos=0;
    QString login = parse(data, data.indexOf("|"));
    QString token = parse(data, data.indexOf("|"));
    QString type = parse(data, data.indexOf("|"));
    QString s_port = parse(data, data.indexOf("?"));
    QString r_body = data;
    QString body = "14/";
    QString role;
    if(v_login.validate(login,pos) && v_login.validate(token,pos)){
        QSqlQuery query(db);
        query.prepare("SELECT employ_id FROM Auth WHERE active=true AND token='"+token+"' AND login='"+login+"';");
        //query.prepare("SELECT role FROM Auth WHERE active=true AND token='YpHmqYwDpO2m' AND login='MylkyDad'");
        query.exec();
        query.first();
        //qDebug() << query.value("role");
        //qDebug() << query.value(0);
        QString employ_id = query.value("employ_id").toString();
        query.prepare("SELECT role_id FROM Employ WHERE id='"+employ_id+"';");
        query.exec();
        query.first();
        role = query.value("role_id").toString();
        if(role.isEmpty()){
            item = new QListWidgetItem(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") + " Auth req error: " + login, mainWidget);
            body+="0|";
            role = "-1";
        } else {
            item = new QListWidgetItem(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") + " Auth req success: " + login, mainWidget);
            body+="1|";
        }
    } else {
        body+="-1|";
        role="-1";
        item = new QListWidgetItem(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") + " Auth req regexp error", mainWidget);
    }
    body+=login+"|"+type+"|"+role+"|"+s_port+"?"+r_body;
    socket->writeDatagram(body.toUtf8(),QHostAddress::LocalHost,25564);
}

//Log out
void Widget::logOut(QByteArray data){
    QString login = parse(data, data.indexOf("|"));
    QString token = parse(data, data.indexOf("|"));
    QString s_port = data;
    int pos = 0;
    QString body = "13/";
    if(v_login.validate(login,pos) && v_login.validate(token,pos)){
        QSqlQuery query(db);
        query.prepare("SELECT updated_at FROM Auth WHERE login='" + login + "' AND token='" + token +"' AND active=true;");
        query.exec();
        query.first();
        if(query.value(0).toString().isEmpty()){
            item = new QListWidgetItem(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") + " Log out non-active error: " + login, mainWidget);
            body += "0|";
        } else {
            QByteArray updated_at=query.value(0).toString().toUtf8();
            QString logout_at=QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.ms");
            QByteArray temp = logout_at.toUtf8();

            parse(updated_at,updated_at.indexOf("T"));
            //qDebug() << updated_at;
            int login_h = parse(updated_at, updated_at.indexOf(":")).toInt();
            int login_m = parse(updated_at, updated_at.indexOf(":")).toInt();

            //qDebug() << login_h;
            //qDebug() << login_m;

            parse(temp, temp.indexOf(" "));
            int logout_h = parse(temp, temp.indexOf(":")).toInt();
            int logout_m = parse(temp, temp.indexOf(":")).toInt();

            int h = logout_h-login_h;
            int m = logout_m-login_m;

            //qDebug() << logout_h;
            //qDebug() << logout_m;

            int time = h*60+m;
            //qDebug() << time;

            QString day = getCurrentDay();
            //qDebug() << day;
            query.prepare("SELECT "+day+" FROM Record WHERE schedule_id=(SELECT id FROM Schedule WHERE employ_id=(SELECT employ_id FROM Auth WHERE login='"+login+"') AND week_id=(SELECT max(id) FROM Weeks));");
            query.exec();
            query.first();
            int h_r = query.value(0).toInt();
            //qDebug() << h_r;
            h_r+=time;
            //qDebug() << h_r;
            query.prepare("UPDATE Record SET "+day+"="+QString::number(h_r)+" WHERE schedule_id=(SELECT id FROM Schedule WHERE employ_id=(SELECT employ_id FROM Auth WHERE login='"+login+"') AND week_id=(SELECT max(id) FROM Weeks));");
            query.exec();

            query.prepare("UPDATE Auth SET token='', active=false WHERE login='" + login +"' AND token='" + token + "'");
            query.exec();
            item = new QListWidgetItem(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") + " Logged out: " + login, mainWidget);
            body += "1|";
        }
    } else {
        item = new QListWidgetItem(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") + " Log out regexp error: " + login, mainWidget);
        body += "-1|";
    }
    body+=s_port;
    socket->writeDatagram(body.toUtf8(),QHostAddress::LocalHost,25564);
}

//QString Widget::getCurrentWeek(){
//    QSqlQuery query(db);
//    query.prepare("SELECT id FROM Weeks WHERE approved=false");
//    query.exec();
//    query.first();
//    QString id;
//    if(query.isValid()){
//        id = query.value(0).toString();
//    } else {
//        id="-1";
//    }
//    return id;
//}

QString Widget::getCurrentDay(){
    QSqlQuery query(db);
    query.prepare("SELECT start_day, end_day FROM Weeks WHERE id=(SELECT max(id) from Weeks);");
    query.exec();
    query.first();
    QByteArray start_day = query.value(0).toString().toUtf8();
    QByteArray end_day = query.value(1).toString().toUtf8();
    QByteArray current_day = QDateTime::currentDateTime().toString("yyyy-MM-dd").toUtf8();
    start_day.lastIndexOf("-");
    parse(start_day, start_day.lastIndexOf("-"));
    parse(end_day, end_day.lastIndexOf("-"));
    parse(current_day, current_day.lastIndexOf("-"));
    int sd=start_day.toInt();
    int ed=end_day.toInt();
    int cd=current_day.toInt();

    //item = new QListWidgetItem(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") + " getCurrentDay: " + QString::number(sd) + "," + QString::number(ed) + "," + QString::number(cd) + " " + "sd,ed,cd" , mainWidget);

    QVector<QString> days = {"monday","tuesday","wednesday","thursday","friday","saturday","sunday"};
    QVector<int> numbers = {sd,ed-5,ed-4,ed-3,ed-2,ed-1,ed};
    int amount = 0;
    int index = 0;
    for(int i=0; i<numbers.size(); i++){
        if(numbers[i]<=0){
            amount++;
            if(numbers[i]==0){
                index = i;
            }
        }
    }
    for(int i=index; i>0; i--){
        numbers[i]=sd+amount;
        amount--;
    }
    //qDebug() << numbers;
    index=-1;
    for(int i=0; i<numbers.size(); i++){
        if(numbers[i]==cd){
            index=i;
        }
    }

    if(index==-1){
        return days[6];
    } else {
        QString h = QDateTime::currentDateTime().toString("hh");
        //qDebug() << h;
        if(h=="00"){
            return days[index-1];
        } else {
            return days[index];
        }
    }
}

void Widget::initNewWeek(){
    QSqlQuery query(db);
    QSqlQuery q(db);
    QDateTime date = QDateTime::currentDateTime();
    QString start_day=date.toString("yyyy-MM-dd");
    date = date.addDays(6);
    QString end_day=date.toString("yyyy-MM-dd");
    query.prepare("INSERT INTO Weeks (start_day,end_day) VALUES('"+start_day+"','"+end_day+"');");
    item = new QListWidgetItem(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") + " initNewWeek: new week created" , mainWidget);
    query.exec();
    QString id = query.lastInsertId().toString();
    query.prepare("SELECT id from Employ WHERE status=1;");
    query.exec();
    while(query.next()){
        q.prepare("SELECT monday, tuesday, wednesday, thursday, friday, saturday, sunday, hours FROM Schedule WHERE employ_id="+query.value(0).toString()+" AND week_id="+QString::number(id.toInt()-1)+";");
        q.exec();
        //qDebug() << q.lastQuery();
        q.first();
        if(q.isValid()){
            QVector<QString> v;
            for(int i=0; i<8; i++){
                v.push_back(q.value(i).toString());
            }
            q.prepare("INSERT INTO Schedule (employ_id, week_id, monday, tuesday, wednesday, thursday, friday, saturday, sunday, hours) VALUES ("+query.value(0).toString()+","+id+","+v[0]+","+v[1]+","+v[2]+","+v[3]+","+v[4]+","+v[5]+","+v[6]+","+v[7]+");");
            q.exec();
            //qDebug() << q.lastQuery();
            QString schedule_id = q.lastInsertId().toString();
            q.prepare("INSERT INTO Record (schedule_id) VALUES ("+schedule_id+");");
            q.exec();
            //qDebug() << q.lastQuery();
            item = new QListWidgetItem(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") + " initNewWeek: init schedule and record for " + query.value(0).toString() + " completed" , mainWidget);
        } else {
            item = new QListWidgetItem(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") + " initNewWeek: schedule for " + query.value(0).toString() + " doenst exist" , mainWidget);
        }
    }
}

//int roundTime(int time){
//    if(time>60){
//        time=-60;
//    }
//    int hours = time/60;
//    int minutes = time%60;
//    if(minutes>30){
//        minutes = 60;
//    } else if (minutes>=-30){
//        minutes = 0;
//    }
//    return (hours*60)+minutes;
//}

int roundTime(int time){
    if(time>60){
        time-=60;
    }
    if(time<=30 && time>=-30){
        time=0;
    }
    return time;
}

void Widget::force(){
    QSqlQuery query(db);
    QSqlQuery q(db);
    QString day = getCurrentDay();
    query.prepare("SELECT login,token,port,active,employ_id FROM Auth");
    query.exec();
    query.first();
    if(query.isValid()){
        do{
            QString login = query.value(0).toString();
            QString token = query.value(1).toString();
            QString port = query.value(2).toString();
            bool active = query.value(3).toBool();
            QString employ_id = query.value(4).toString();
            QString body = login + "|" + token + "|" + port;

            q.prepare("SELECT id FROM Schedule WHERE employ_id="+employ_id+" AND week_id = (SELECT max(id) from Weeks);");
            q.exec();
            q.first();
            if(q.isValid()){
                QString schedule_id = q.value(0).toString();
                //qDebug() << " s_i "<<schedule_id;

                if(active){
                    item = new QListWidgetItem(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") + " force: Force log out for " + login, mainWidget);
                    logOut(body.toUtf8());
                    q.prepare("SELECT "+ day + " from Schedule WHERE id=" + schedule_id + ";");
                    q.exec();
                    q.first();
                    int h_s = q.value(0).toInt();
                    //qDebug() << " q "<<q.lastQuery();
                    //qDebug() << " H_S "<<h_s;
                    q.prepare("SELECT "+ day + ", hours from Record WHERE schedule_id=" + schedule_id + ";");
                    q.exec();
                    q.first();
                    int h_r = q.value(0).toInt();
                    int sum = q.value(1).toInt();
                    //qDebug() << " q "<<q.lastQuery();
                    //qDebug() << " H_R " <<h_r;
                    int time;
                    if(h_r-h_s>0){
                        time = h_s;
                    } else {
                        time = roundTime(h_r-h_s);
                    }
                    //qDebug()<<" TIME " <<time;
                    sum+=time;
                    q.prepare("UPDATE Record SET "+ day +"="+QString::number(time)+", hours = "+QString::number(sum)+" WHERE schedule_id=" + schedule_id + ";");
                    q.exec();
                } else {
                    q.prepare("SELECT status FROM Employ WHERE id="+employ_id+";");
                    q.exec();
                    q.first();
                    int status = q.value(0).toInt();
                    if(status == 2){
                        //QSqlQuery q_weeks(db), q_2(db);

                        q.prepare("SELECT end_date FROM Sick WHERE id=(SELECT max(id) FROM Sick WHERE employ_id="+employ_id+") AND approved=false;");
                        q.exec();
                        q.first();
                        if(q.isValid()){

                            //QString sick_id = q.value(0).toString();
                           //QString start_date_string = q.value(1).toString();

                            QString end_date_string = q.value(0).toString();

                            //QDate start_date = QDate::fromString(start_date_string, "yyyy-MM-dd");

                            QDate end_date = QDate::fromString(end_date_string, "yyyy-MM-dd");

                            //QDate deadline = end_date.addDays(6);

                            QString today_s = QDate::currentDate().toString("yyyy-MM-dd");
                            QDate today = QDate::fromString(today_s, "yyyy-MM-dd");
                            if(today>end_date){
                                q.prepare("UPDATE Employ SET status=1 WHERE id="+employ_id+";");
                                q.exec();
                                item = new QListWidgetItem(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") + " force: User " + login + " must be on work today from sick", mainWidget);
                            } else {
                                item = new QListWidgetItem(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") + " force: User " + login + " on sick", mainWidget);
                            }
                        } else {
                            item = new QListWidgetItem(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") + " force: Status error for " + login + " on sick", mainWidget);
                        }
                        //q_1.prepare("SELECT id, start_date, end_date from Seek WHERE ")
                    }
                    if(status == 3){
                        q.prepare("SELECT end_date FROM Vacation WHERE id=(SELECT max(id) FROM Vacation WHERE employ_id="+employ_id+");");
                        q.exec();
                        q.first();
                        //QString start_date_s = q.value(0).toString();
                        if(q.isValid()){
                            QString end_date_s = q.value(0).toString();
                            QString today_s = QDateTime::currentDateTime().toString("yyyy-MM-dd");
                            QDate today_d = QDate::fromString(today_s, "yyyy-MM-dd");
                            //QDate start_date_d = QDate::fromString(start_date_s, "yyyy-MM-dd");
                            QDate end_date_d = QDate::fromString(end_date_s, "yyyy-MM-dd");
                            if(today_d>end_date_d){
                                item = new QListWidgetItem(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") + " force: Updated status for " + login + " on vacation", mainWidget);
                                q.prepare("UPDATE Employ WHERE id="+employ_id+";");
                                q.exec();
                            } else {
                                item = new QListWidgetItem(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") + " force: User " + login + " on vacation", mainWidget);
                            }
                        } else {
                            item = new QListWidgetItem(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") + " force: Status error for " + login + " on vacation", mainWidget);
                        }
                    }
                    if(status == 1){
                        sickCheck(login, employ_id);
                        item = new QListWidgetItem(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") + " force: Calculating for " + login, mainWidget);
                        //
                        q.prepare("SELECT Schedule."+ day + ", Record."+day+", Record.hours from Schedule JOIN Record ON Record.schedule_id=" + schedule_id + ";");
                        //
                        q.exec();
                        q.first();
                        int h_s = q.value(0).toInt();
                        //qDebug() << " q "<<q.lastQuery();
                        //qDebug() << " H_S "<<h_s;
                        int h_r = q.value(1).toInt();
                        int sum = q.value(2).toInt();
                        //qDebug() << " q "<<q.lastQuery();
                        //qDebug() << " H_R " <<h_r;
                        int time = roundTime(h_r-h_s);
                        sum+=(int)time;
                        //qDebug()<<" TIME " <<time;
                        q.prepare("UPDATE Record SET "+day+"="+QString::number(time)+", hours="+QString::number(sum)+" WHERE schedule_id="+schedule_id+";");
                        q.exec();
                    }

                }
            } else {
                item = new QListWidgetItem(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") + " force: No Schedule for " + login, mainWidget);
            }

        } while(query.next());
    } else {
        item = new QListWidgetItem(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") + " force: There are 0 unapproved sessions ", mainWidget);
    }
    if(day=="sunday"){
        initNewWeek();
    }
}



//
//q.exec();
//q.first();
//if(q.isValid()){
//    QString s_id = q.value(0).toString();
//    QString start_date_s = q.value(1).toString();
//    QString end_date_s = q.value(2).toString();
//    //bool approved = q.value(3).toBool();
//    QString today_s = QDateTime::currentDateTime().toString("yyyy-MM-dd");
//    QDate today_d = QDate::fromString(today_s, "yyyy-MM-dd");
//    QDate start_date_d = QDate::fromString(start_date_s, "yyyy-MM-dd");
//    QDate end_date_d = QDate::fromString(end_date_s, "yyyy-MM-dd");
//    if(today_d>end_date_d){
//        QDate deadline = end_date_d.addDays(6);
//        if(today_d>deadline/* && !approved*/){
//            QDate temp = start_date_d;
//            while(temp<=end_date_d){
//                QString day = temp.toString("dddd").toLower();
//                QSqlQuery q_1(db), q_2(db);
//                q_1.prepare("SELECT id, start_day, end_day from WEEKS");
//                q_1.exec();
//                while(q_1.next()){
//                    QString week_id = q_1.value(0).toString();
//                    QString s_d_s = q_1.value(1).toString();
//                    QDate s_d_d = QDate::fromString(s_d_s, "yyyy-MM-dd");
//                    QString e_d_s = q_1.value(2).toString();
//                    QDate e_d_d = QDate::fromString(e_d_s, "yyyy-MM-dd");
//                    qDebug() << " S_D_D " << s_d_d;
//                    qDebug() << " E_D_D " << e_d_d;
//                    qDebug() << " TEMP " << temp;
//                    if(temp<=e_d_d && temp>=s_d_d){
//                        q_2.prepare("SELECT id, "+day+" from Schedule WHERE week_id="+week_id+", AND employ_id="+employ_id+";");
//                        q_2.exec();
//                        q_2.first();
//                        QString schedule_id = q_2.value(0).toString();
//                        int schedule_day_time = q_2.value(1).toInt();
//                        //double schedule_hours = q_2.value(2).toDouble();
//                        q_2.prepare("SELECT hours from Record WHERE schedule_id="+schedule_id+";");
//                        q_2.exec();
//                        q_2.first();
//                        double record_hours = q_2.value(0).toDouble();
//                        record_hours-=schedule_day_time;
//                        q_2.prepare("UPDATE Record SET "+day+"="+QString::number(-schedule_day_time)+", hours="+QString::number(record_hours)+" WHERE schedule_id="+schedule_id+";");
//                        q_2.exec();
//                        q_2.prepare("UPDATE Sick SET approved=true WHERE id="+s_id+";");
//                        q_2.exec();
//                        item = new QListWidgetItem(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") + " force: Updated hours with Sick for " + login, mainWidget);
//                    }
//                }
//                temp = temp.addDays(1);
//            }
//        } else {
//            item = new QListWidgetItem(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") + " force: Deadline is not today for " + login, mainWidget);
//        }
//    } else {
//        item = new QListWidgetItem(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") + " force: User " + login + " on sick", mainWidget);
//    }
//} else {
//
//}



void Widget::processRequest(QByteArray data){
    if(db.isOpen()){
        int index = data.indexOf("/");
        int type = parse(data, index).toInt();
        switch(type){
        case 0: {
            login(data);
            break;
        }
        case 1: {
            authRequest(data);
            break;
        }
        case 2: {
            logOut(data);
            break;
        }
//        case 3:{
//            activeApprove(data);
//        }
        default: {
            item = new QListWidgetItem(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") + " Unknown request type", mainWidget);
        }
        }
    }
}

Widget::~Widget()
{
    delete ui;
}

