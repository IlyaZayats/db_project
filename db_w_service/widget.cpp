#include "widget.h"
#include "./ui_widget.h"

QRegularExpression re_login("^[a-zA-Z0-9]+$");
QRegularExpression re_int("^[\\d]+$");
QRegularExpression re_varchar("^[a-zA-Z0-9\\s]+$");
QRegularExpression re_name("^[a-zA-Z-\\s]+$");
QRegularExpression re_mail("\\b[A-Z0-9._%+-]+@[A-Z0-9.-]+\\.[A-Z]{2,4}\\b", QRegularExpression::CaseInsensitiveOption);
QRegularExpression re_date("([12]\\d{3}-(0[1-9]|1[0-2])-(0[1-9]|[12]\\d|3[01]))");
QRegularExpressionValidator v_login(re_login, 0);
QRegularExpressionValidator v_int(re_int, 0);
QRegularExpressionValidator v_varchar(re_varchar, 0);
QRegularExpressionValidator v_name(re_name, 0);
QRegularExpressionValidator v_mail(re_mail, 0);
QRegularExpressionValidator v_date(re_date, 0);
QDoubleValidator v_double;

Widget::Widget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Widget)
{
    ui->setupUi(this);

    socket = new QUdpSocket(this);
    socket->bind(25566);

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
    db = QSqlDatabase::addDatabase("QPSQL", "db_w_service");
    db.setHostName("localhost");
    db.setDatabaseName("postgres");
    db.setUserName("postgres");
    db.setPassword("12345");
    bool ok = db.open();
    if(!ok){
        item = new QListWidgetItem(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") + " DB connection error", mainWidget);
    }
    connect(socket, SIGNAL(readyRead()), SLOT(request()));
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
    if(index != -1){
    for(int i=0; i<index; i++){
        r.push_back(data[i]);
    }
    data.remove(0, index+1);
    } else {
        r = data;
    }
    return r;
}

//Загрузка role.csv: output->202/status1|...|statusN|port
void Widget::roleInsertCSV(QByteArray data){
    QString port = parse(data, data.indexOf("?"));
    //item = new QListWidgetItem(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") + QString::fromUtf8(data), mainWidget);
    int index = data.indexOf(";");
    QString body = "202/";
    if(!data.isEmpty()){
        while(index!=-1) {
            int pos = 0;
            QByteArray values = parse(data,index);
            QString id = parse(values, values.indexOf(","));
            QString rate = parse(values, values.indexOf(","));
            QString hours = parse(values, values.indexOf(","));
            QString title = values;
//            item = new QListWidgetItem(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") + " " + id + " " + rate + " " + hours + " " + title, mainWidget);
//            item = new QListWidgetItem(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") + " " + QString::number(v_int.validate(id, pos)) + " " + QString::number(v_int.validate(rate, pos)) + " " + QString::number(v_int.validate(hours, pos)) + " " + QString::number(v_varchar.validate(title, pos)), mainWidget);
            if(v_varchar.validate(title, pos) && v_int.validate(rate, pos) && v_int.validate(hours, pos) && v_int.validate(id, pos)){
                QSqlQuery query(db);
                query.prepare("SELECT * FROM Role WHERE id='"+id+"';");
                query.exec();
                query.first();
                if(query.isValid()){
                    item = new QListWidgetItem(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") + " roleInsertCSV: " +id +"," + rate + "," + hours + "," + title + " updated!", mainWidget);
                    query.prepare("UPDATE Role SET rate="+rate+",hours='"+hours+"',title='"+title+"',updated_at='"+QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.ms")+"' WHERE id="+id+";");
                    body+="2|";
                } else {
                    item = new QListWidgetItem(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") + " roleInsertCSV: " +id +"," + rate + "," + hours + "," + title + " inserted!", mainWidget);
                    query.prepare("INSERT INTO Role (id,rate,hours,title) VALUES ("+id+","+rate+","+hours+",'"+title+"');");
                    body+="1|";
                }
                query.exec();
            } else {
                item = new QListWidgetItem(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") + " roleInsertCSV: Validation error ", mainWidget);
                body+="0|valid";
            }
            index = data.indexOf(";");
        }
    } else {
        body += "-1|error";
    }
    socket->writeDatagram((body+port).toUtf8(), QHostAddress::LocalHost, 25564);
}

//Дописать валидацию Загрузка employ.csv: output->203/status1|...|statusN|port
void Widget::employInsertCSV(QByteArray data){
    QString port = parse(data, data.indexOf("?"));
    //item = new QListWidgetItem(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") + QString::fromUtf8(data), mainWidget);
    int index = data.indexOf(";");
    QString body = "203/";
    if(!data.isEmpty()){
        while(index!=-1) {
            int pos = 0;
            QByteArray values = parse(data,index);
            QString role_id = parse(values, values.indexOf(","));
            QString last_name = parse(values, values.indexOf(","));
            QString first_name = parse(values, values.indexOf(","));
            QString middle_name = parse(values, values.indexOf(","));
            QString birth_date = parse(values, values.indexOf(","));
            QString subdivision = parse(values, values.indexOf(","));
            QString passport_series = parse(values, values.indexOf(","));
            QString passport_number = parse(values, values.indexOf(","));
            QString citizenship = parse(values, values.indexOf(","));
            QString address = parse(values, values.indexOf(","));
            QString phone_number_w = parse(values, values.indexOf(","));
            QString phone_number_p = parse(values, values.indexOf(","));
            QString email = parse(values, values.indexOf(","));//???
            QString login = parse(values, values.indexOf(","));
            QString pwd = parse(values, values.indexOf(","));
            QString status = values;

            bool error = false;
            //int values valid
            if(!v_int.validate(role_id, pos) || !v_int.validate(passport_series,pos) || !v_int.validate(passport_number, pos) || !v_int.validate(phone_number_w, pos) || !v_int.validate(phone_number_p, pos) || !v_int.validate(status, pos)){
                error = true;
            }
            //name values valid
            if(!v_name.validate(last_name, pos) || !v_name.validate(first_name, pos) || !v_name.validate(middle_name, pos) || !v_name.validate(subdivision, pos) || !v_name.validate(citizenship, pos)){
                error = true;
            }
            if(!v_varchar.validate(address, pos) || !v_login.validate(login, pos) || !v_login.validate(pwd, pos)){
                error = true;
            }
            if(!v_mail.validate(email, pos)){
                item = new QListWidgetItem(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") + " employInsertCSV: email valid error " + port, mainWidget);
                error = true;
            }
            if(!v_date.validate(birth_date, pos)){
                item = new QListWidgetItem(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") + " employInsertCSV: birth_date valid error " + port, mainWidget);
                error = true;
            }

            if(!error){
                QSqlQuery query(db);
                query.prepare("SELECT * FROM Auth WHERE login='"+login+"';");
                query.exec();
                query.first();
                //QByteArray pwd_hash = QCryptographicHash::hash(pwd.toUtf8(), QCryptographicHash::Md5);
                if(query.isValid()){
                    item = new QListWidgetItem(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") + " employInsertCSV: " + login + " updated!", mainWidget);
                    QString id = query.value("employ_id").toString();
                    query.prepare("UPDATE Auth SET login='"+login+"', pwd='"+pwd+"' WHERE employ_id="+id+";");
                    query.exec();
                    query.prepare("UPDATE Employ SET last_name='"+last_name+"', first_name='"+first_name+"', middle_name='"+middle_name+"', birth_date='"+birth_date+"', subdivision='"+subdivision+"', passport_series='"+passport_series+"', passport_number='"+passport_number+"', citizenship='"+citizenship+"', address='"+address+"', phone_number_w='"+phone_number_w+"', phone_number_p='"+phone_number_p+"', email='"+email+"', status="+status+",updated_at='"+QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.ms")+"'WHERE id="+id+";");
                    query.exec();
                    body+="2|";
                } else {
                    item = new QListWidgetItem(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") + " employInsertCSV: " + login + " inserted!", mainWidget);
                    query.prepare("INSERT INTO Employ (role_id,last_name,first_name,middle_name,birth_date,subdivision,passport_series,passport_number,citizenship,address,phone_number_w,phone_number_p,email) VALUES ("+role_id+",'"+last_name+"','"+first_name+"','"+middle_name+"','"+birth_date+"','"+subdivision+"','"+passport_series+"','"+passport_number+"','"+citizenship+"','"+address+"','"+phone_number_w+"','"+phone_number_p+"','"+email+"');");
                    //query.prepare("INSERT INTO Eploy (role_id,last_name,first_name,middle_name,birth_date,subdivision,passport_series,passport_number,citizenship,address,phone_number_w,phone_number_p,email) VALUES (:role_id);");
                    query.exec();
                    query.first();
                    QString id = query.lastInsertId().toString();
                    item = new QListWidgetItem(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") + "  employInsertCSV: " + id + " id inserted!", mainWidget);
                    if(!id.isNull()){
                        query.prepare("INSERT INTO Auth (login,pwd,employ_id) VALUES ('"+login+"','"+pwd+"',"+id+");");
                        query.exec();
                        body+="1|";
                    } else {
                        item = new QListWidgetItem(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") + " employInsertCSV: employ_id error " + port, mainWidget);
                        body+="1|";
                    }
                }
            } else {
                body+="0|";
                item = new QListWidgetItem(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") + " employInsertCSV: valid error " + port, mainWidget);
            }
            index = data.indexOf(";");
        }
    } else {
        body += "-1|";
    }
    socket->writeDatagram((body+port).toUtf8(), QHostAddress::LocalHost, 25564);
}

//Проверка часов работяг: output-> 200/port|answer?data1;...;dataN;
//void Widget::checkOutput(QByteArray data){
//    QString port = data;
//    item = new QListWidgetItem(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") + " Check show request " + port, mainWidget);
//    QString body = "200/"+port+"|";
//    QSqlQuery query(db);
//    //тут
//    //query.prepare("SELECT Employ.id, Employ.last_name, Employ.first_name, Employ.middle_name, Employ.role_id, Role.hours, Record.hours FROM Employ JOIN Role ON Role.id = Employ.role_id JOIN Record Record.employ_id = Employ.id WHERE Employ.id NOT IN (SELECT employ_id FROM Fired);");
//    query.prepare("SELECT Employ.id, Employ.last_name, Employ.first_name, Employ.middle_name, Employ.role_id, Role.hours FROM Employ JOIN Role ON Role.id = Employ.role_id WHERE Employ.id NOT IN (SELECT employ_id FROM Fired);");
//    query.exec();
//    //query.first();
//    //item = new QListWidgetItem(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") + " " + query.value(0).toString(), mainWidget);
//    //if(query.isValid()){
//    if(query.size()>0){
//        body+="1?";
//        while(query.next()){
//            item = new QListWidgetItem(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") + " " + query.value(0).toString() + " " + query.value(1).toString() + " " + query.value(2).toString() + " " + query.value(3).toString() + " " + query.value(4).toString() + " " + query.value(5).toString(), mainWidget);
//            //item = new QListWidgetItem(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") + " " + , mainWidget);
//            int i = 0;
//            while(!query.value(i).toString().isNull()){
//                //item = new QListWidgetItem(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") + " " + query.value(i).toString(), mainWidget);
//                body+=query.value(i).toString()+",";
//                i++;
//            }
//            int index = body.lastIndexOf(",");
//            body.replace(index,1,";");
//    //        for(int i=0; i<6; i++){
//    //            body+=query.value(i).toString()+"|";
//    //        }
//        }
//    } else {
//        body+="0?empty";
//    }
//    //item = new QListWidgetItem(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") + " " + body, mainWidget);
//    socket->writeDatagram(body.toUtf8(), QHostAddress::LocalHost, 25564);
//    //}
//}

void Widget::insertQueryIntoBody(QString request, QString &body, int fields){
    QSqlQuery q(db);
    q.prepare(request);
    q.exec();
    q.first();
    if(q.isValid()){
        body+="1|";
        do{
            for(int i=0; i<fields; i++){
                body+=q.value(i).toString()+",";
            }
            int index = body.lastIndexOf(",");
            body.replace(index,1,";");
        }while(q.next());
        int index = body.lastIndexOf(";");
        body.replace(index,1,"*");
    } else {
        body+="0|empty*";
    }
}

void Widget::checkOutput(QByteArray data){
    QString port = parse(data,data.indexOf("|"));
    QString week_id = data;
    QSqlQuery q(db);
    item = new QListWidgetItem(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") + " checkOutput: request " + port, mainWidget);
    QString body = "200/"+port+"?";
    q.prepare("SELECT * FROM Weeks WHERE id="+week_id+";");
    q.exec();
    q.first();
    //item = new QListWidgetItem(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") + " q: " + q.lastQuery(), mainWidget);
    if(q.isValid()){
        insertQueryIntoBody("SELECT Employ.last_name, Employ.first_name, Employ.middle_name, Employ.role_id, Schedule.id, Schedule.employ_id, Schedule.monday, Schedule.tuesday, Schedule.wednesday, Schedule.thursday, Schedule.friday, Schedule.saturday, Schedule.sunday, Schedule.hours, Record.id, Record.monday, Record.tuesday, Record.wednesday, Record.thursday, Record.friday, Record.saturday, Record.sunday, Record.hours FROM Schedule JOIN Employ ON Employ.id=Schedule.employ_id JOIN Record ON Record.schedule_id=Schedule.id WHERE Schedule.week_id = "+week_id+";", body, 23);
    } else {
        item = new QListWidgetItem(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") + " checkOutput: Week id error " + port, mainWidget);
        body += "error";
    }
    socket->writeDatagram(body.toUtf8(), QHostAddress::LocalHost, 25564);
}

void Widget::statusOutput(QByteArray data){
    QString port = data;
    item = new QListWidgetItem(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") + " statusOutput: request " + port, mainWidget);
    QString body = "201/"+port+"?";
    insertQueryIntoBody("SELECT Fired.id, Fired.employ_id, Fired.fired_at, Employ.last_name, Employ.first_name, Employ.middle_name, Employ.role_id FROM Fired JOIN Employ ON Employ.id=Fired.employ_id;", body, 7);
    insertQueryIntoBody("SELECT Vacation.id, Vacation.employ_id, Vacation.start_date, Vacation.end_date, Employ.last_name, Employ.first_name, Employ.middle_name, Employ.role_id FROM Vacation JOIN Employ ON Employ.id=Vacation.employ_id;", body, 8);
    insertQueryIntoBody("SELECT Sick.id, Sick.employ_id, Sick.start_date, Sick.end_date, Sick.approved, Employ.last_name, Employ.first_name, Employ.middle_name, Employ.role_id FROM Sick JOIN Employ ON Employ.id=Sick.employ_id;", body, 9);
    socket->writeDatagram(body.toUtf8(), QHostAddress::LocalHost, 25564);
}

//void Widget::test(){
//    QSqlQuery q;
//}

void Widget::statusInsertChanges(QByteArray data){
    QString port = parse(data,data.indexOf("?"));
    item = new QListWidgetItem(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") + " statusInsertChanges: request " + port, mainWidget);
    int type = parse(data,data.indexOf("|")).toInt();
    QString employ_id = parse(data, data.indexOf(","));
    QString body = "205/"+port+"|";
    QSqlQuery query(db);
    QSqlQuery q(db);
    if (type == 0){
        //fired
        QString fired_at = data;
        query.prepare("SELECT * FROM Fired WHERE employ_id="+employ_id+";");
        query.exec();
        query.first();
        if(query.isValid()){
            item = new QListWidgetItem(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") + " statusInsertChanges: fire already fired error" , mainWidget);
        } else {
            item = new QListWidgetItem(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") + " statusInsertChanges: " + fired_at, mainWidget);
            query.prepare("INSERT INTO Fired (employ_id, fired_at) VALUES ("+employ_id+",'"+fired_at+"');");
            query.exec();
            query.prepare("UPDATE Employ SET status=0 WHERE id="+employ_id+";");
            query.exec();
            body+="1";
            item = new QListWidgetItem(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") + " statusInsertChanges: Fired employ with id=" + employ_id, mainWidget);
        }
    } else {
        //vac or sick
        QString id = parse(data, data.indexOf(","));
        QString start_date = parse(data, data.indexOf(","));
        QString end_date = parse(data, data.indexOf(","));
        QString approved = data;
        QString table = "Vacation";

        //valid

        if(type==2){
            table = "Sick";
        }
        query.prepare("SELECT * FROM "+table+" WHERE id=" + id + ";");
        query.exec();
        query.first();
        if(query.isValid()){
            if(type==2){
                query.prepare("UPDATE "+table+" SET start_date='"+start_date+"',end_date='"+end_date+"',approved="+approved+" WHERE id="+id+";");
                q.prepare("UPDATE Employ SET status=2 WHERE id="+employ_id+";");
            } else {
                query.prepare("UPDATE "+table+" SET start_date='"+start_date+"',end_date='"+end_date+"' WHERE id="+id+";");
                q.prepare("UPDATE Employ SET status=3 WHERE id="+employ_id+";");
            }
            query.exec();
            q.exec();
            body+="2";
            item = new QListWidgetItem(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") + "statusInsertChanges: Updated "+table+" id=" + id, mainWidget);
        } else {
            if(type == 2){
                query.prepare("INSERT INTO "+table+" (employ_id, start_date, end_date, approved) VALUES ("+employ_id+",'"+start_date+"','"+end_date+"',"+approved+");");
                q.prepare("UPDATE Employ SET status=2 WHERE id="+employ_id+";");
            } else {
                query.prepare("INSERT INTO "+table+" (employ_id, start_date, end_date) VALUES ("+employ_id+",'"+start_date+"','"+end_date+"');");
                q.prepare("UPDATE Employ SET status=3 WHERE id="+employ_id+";");
            }
            query.exec();
            q.exec();
            //item = new QListWidgetItem(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") + " statusInsertChanges: " + query.lastQuery(), mainWidget);
            body+="3";
            item = new QListWidgetItem(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") + "statusInsertChanges: Inserted into "+table+" id=" + QString::number(query.lastInsertId().toInt()), mainWidget);
        }
    }
    socket->writeDatagram(body.toUtf8(), QHostAddress::LocalHost, 25564);
    statusOutput(port.toUtf8());
}

void Widget::checkInsertChanges(QByteArray data){
    QString port = parse(data, data.indexOf("?"));
    QString employ_id = parse(data, data.indexOf(","));
    QString last_name = parse(data, data.indexOf(","));
    QString first_name = parse(data, data.indexOf(","));
    QString middle_name = parse(data, data.indexOf(","));
    QString role_id = parse(data, data.indexOf(","));
    QVector<QString> days;
    QString body = "204/";
    bool error = false;
    int pos = 0;
    for(int i=0; i<14; i++){
        days.push_back(parse(data,data.indexOf(",")));
        if(!v_int.validate(days[i], pos)){
            error = true;
            break;
        }
    }
    QString s_h = parse(data, data.indexOf(","));
    QString r_h = data;
    if(error || !v_int.validate(employ_id, pos) || !v_varchar.validate(last_name,pos) || !v_varchar.validate(first_name,pos) || !v_varchar.validate(middle_name, pos) || !v_int.validate(role_id, pos) || !v_int.validate(s_h, pos) || !v_double.validate(r_h, pos)){
        item = new QListWidgetItem(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") + " checkInsertChanges: Validation error" , mainWidget);
        body+="-1";
    } else {
        QSqlQuery q(db);
        q.prepare("SELECT * FROM Employ WHERE id="+employ_id+" AND status<>0;");
        q.exec();
        q.first();
        if(q.isValid()){
            q.prepare("SELECT * FROM Schedule WHERE week_id=(SELECT max(id) from Weeks) AND employ_id="+employ_id+";");
            q.exec();
            q.first();
            if(q.isValid()){
                q.prepare("UPDATE Schedule SET monday="+days[0]+", tuesday="+days[1]+", wednesday="+days[2]+", thursday="+days[3]+", friday="+days[4]+", saturday="+days[5]+", sunday="+days[6]+", hours="+s_h+" WHERE week_id=(SELECT max(id) from Weeks) AND employ_id="+employ_id+";");
                q.exec();
                //item = new QListWidgetItem(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") + " checkInsertChanges: Schedule updated for employ_id=" + employ_id , mainWidget);
                q.prepare("SELECT id from Schedule WHERE employ_id="+employ_id+" AND week_id=(SELECT max(id) from Weeks);");
                q.exec();
                q.first();
                QString schedule_id = q.value(0).toString();
                q.prepare("UPDATE Record SET monday="+days[7]+", tuesday="+days[8]+", wednesday="+days[9]+", thursday="+days[10]+", friday="+days[11]+", saturday="+days[12]+", sunday="+days[13]+", hours="+r_h+" WHERE schedule_id="+schedule_id+";");
                q.exec();
                q.prepare("UPDATE Employ SET last_name='"+last_name+"', first_name='"+first_name+"', middle_name='"+middle_name+"' WHERE id="+employ_id+";");
                q.exec();
                item = new QListWidgetItem(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") + " checkInsertChanges: Schedule, Record and full name updated for employ_id=" + employ_id , mainWidget);
                body+="2|";
            } else {
                q.prepare("INSERT INTO Schedule (monday,tuesday,wednesday,thursday,friday,saturday,sunday,hours,week_id,employ_id) VALUES ("+days[0]+","+days[1]+","+days[2]+","+days[3]+","+days[4]+","+days[5]+","+days[6]+","+s_h+",(SELECT max(id) FROM Weeks),"+employ_id+");");
                q.exec();
                QString schedule_id = q.lastInsertId().toString();
                q.prepare("INSERT INTO Record (monday,tuesday,wednesday,thursday,friday,saturday,sunday,hours,schedule_id) VALUES ("+days[7]+","+days[8]+","+days[9]+","+days[10]+","+days[11]+","+days[12]+","+days[13]+","+r_h+","+schedule_id+");");
                item = new QListWidgetItem(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") + "checkInsertChanges: Schedule and Record inserted for employ_id=" + employ_id , mainWidget);
                body+="1|";
            }
        } else {
            item = new QListWidgetItem(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") + " checkInsertChanges: Worker with id="+employ_id+" is fired or doenst exist" , mainWidget);
            body+="-2|";
        }
    }
    socket->writeDatagram((body+port).toUtf8(), QHostAddress::LocalHost, 25564);
    QSqlQuery q(db);
    q.prepare("SELECT max(id) from weeks;");
    q.exec();
    q.first();
    checkOutput((port+"|"+q.value(0).toString()).toUtf8());

//    QString role_hours = parse(data, data.indexOf(","));
//    QString record_hours = data;

//    if(v_int.validate(employ_id, pos) && v_varchar.validate(last_name,pos) && v_varchar.validate(first_name,pos) && v_varchar.validate(middle_name, pos) && v_int.validate(role_id, pos) && v_int.validate(role_hours, pos) && v_double.validate(record_hours, pos)){

//    }
}

void Widget::insertSchedule(QByteArray data){
    QString port = parse(data, data.indexOf("?"));
    int index = data.indexOf(";");
    QByteArray values;
    QString body = "206/";
    while(index!=-1){
        values = parse(data, index);
        QString employ_id = parse(values, values.indexOf(","));
        //QString week_id = parse(values, values.indexOf(","));
        QVector<QString> days;
        bool error = false;
        int pos = 0;
        for(int i=0; i<7; i++){
            days.push_back(parse(values, values.indexOf(",")));
            if(!v_int.validate(days[i], pos)){
                error = true;
                break;
            }
        }
        QString hours = values;
        if(error || !v_int.validate(employ_id, pos) /*|| !v_int.validate(week_id, pos)*/ || !v_int.validate(hours, pos)){
            item = new QListWidgetItem(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") + " insertSchedule: Validation error" , mainWidget);
            body+="-1|";
        } else {
            QSqlQuery q(db);
            q.prepare("SELECT * FROM Employ WHERE id="+employ_id+" AND status=1;");
            q.exec();
            q.first();
            if(q.isValid()){
                q.prepare("SELECT * FROM Schedule WHERE week_id=(SELECT max(id) from Weeks) AND employ_id="+employ_id+";");
                q.exec();
                q.first();
                if(q.isValid()){
                    QString updated_at = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.ms");
                    q.prepare("UPDATE Schedule SET monday="+days[0]+", tuesday="+days[1]+", wednesday="+days[2]+", thursday="+days[3]+", friday="+days[4]+", saturday="+days[5]+", sunday="+days[6]+", hours="+hours+", updated_at='"+updated_at+"' WHERE week_id=(SELECT max(id) from Weeks) AND employ_id="+employ_id+";");
                    q.exec();
                    item = new QListWidgetItem(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") + " insertSchedule: Schedule updated for employ_id=" + employ_id , mainWidget);
                    body+="2|";
                } else {
                    q.prepare("INSERT INTO Schedule (monday,tuesday,wednesday,thursday,friday,saturday,sunday,hours,week_id,employ_id) VALUES ("+days[0]+","+days[1]+","+days[2]+","+days[3]+","+days[4]+","+days[5]+","+days[6]+","+hours+",(SELECT max(id) FROM Weeks),"+employ_id+");");
                    q.exec();
                    QString schedule_id = q.lastInsertId().toString();
                    q.prepare("INSERT INTO Record (schedule_id) VALUES ("+schedule_id+");");
                    q.exec();
                    item = new QListWidgetItem(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") + " insertSchedule: Schedule and Record inserted for employ_id=" + employ_id , mainWidget);
                    body+="1|";
                }
            } else {
                item = new QListWidgetItem(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") + " insertSchedule: Worker with id="+employ_id+" is fired or doenst exist" , mainWidget);
                body+="-2|";
            }
        }
    index = data.indexOf(";");

    }
    socket->writeDatagram((body+port).toUtf8(), QHostAddress::LocalHost, 25564);
}

void Widget::processRequest(QByteArray data){
    if(db.isOpen()){
        int index = data.indexOf("/");
        int type = parse(data, index).toInt();
        switch(type){
        //output check
        case 0: {
            checkOutput(data);
            break;
        }
        //output status
        case 1: {
            statusOutput(data);
            break;
            //
        }
        //insert csv role 100%
        case 2: {
            roleInsertCSV(data);
            break;
        }
        //insert csv employ 80%
        case 3: {
            employInsertCSV(data);
            break;
        }
        //single changes for change
        case 4: {
            checkInsertChanges(data);
            break;
            //
        }
        //single changes for status
        case 5:{
            statusInsertChanges(data);
            break;
            //
        }
        //insert Schedule
        case 6:{
            insertSchedule(data);
            break;
        }
        default: {
            item = new QListWidgetItem(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") + " Unknown request type", mainWidget);
            break;
        }
        }
    }
}

Widget::~Widget()
{
    delete ui;
}

