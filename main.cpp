#include "oscilloscope.h"
#include "receiver.h"
#include <QApplication>
#include <QPalette>

const qint64 DEFAULT_PERIOD_SAMMPLE = 1000000000;

Receiver* init(QVector<qint32>& chs, QString& title) {
    Receiver* prec = nullptr;
    bool abs, rel;  //используется ли в потоке данных абсолютное и/или относительное время
    qint64 period = DEFAULT_PERIOD_SAMMPLE; //период дискретизации
    QCoreApplication::setApplicationName("osc");
    QCoreApplication::setApplicationVersion("1.0");
    QCommandLineParser parser;
    //Установка опций
    parser.setApplicationDescription("Данная программа используется для отладки интерфейса программы Осциллятор.\n"
                                     "Также её наработки будут использоваться в виджете osc.");
    parser.addHelpOption();
    parser.addVersionOption();
    QCommandLineOption optType(QStringList() << "t" << "type",
                               "Определяет тип отсчёта. Возможные значения: 8, u8, 16, u16, 32, u32, 64.", "type",
                               "32");
    parser.addOption(optType);
    QCommandLineOption optChannels (QStringList() << "c" << "channels",
                                    "Определяются идентификаторы каналов, используемых в потоке данных.  Каналы "
                                    "отделяются пробелами (всё это заключено в кавычки).", "\"id1 id2...\"", "1 2 3");
    parser.addOption(optChannels);
    QCommandLineOption optNumber(QStringList() << "n" << "number",
                                 "Определятся количество каналов, используемых в потоке данных. ", "number");
    parser.addOption(optNumber);
    QCommandLineOption optAbs(QStringList() << "a" << "absolute",
                              "Устанавливаетя, если в потоке данных присутствует абсолютное время измерительной системы.");
    parser.addOption(optAbs);
    QCommandLineOption optRel(QStringList() << "r" << "relative",
                              "Устанавливаетя, если в потоке данных присутствует значение периода выборки измерительной системы.");
    parser.addOption(optRel);
    QCommandLineOption optPeriod(QStringList() << "p" << "period",
                                 "Определяет фиксированное значение периода выборки в нс.", "period",
                                 "1000000000");
    parser.addOption(optPeriod);
    QCommandLineOption optBanner(QStringList() << "b" << "banner",
                                 "Определяет заголовок окна приложения.", "name",
                                 "osc");
    parser.addOption(optBanner);

    parser.process(*qApp);
    //Проверка введённых опций
    abs = parser.isSet(optAbs);
    if(!(rel = parser.isSet(optRel)))
        period = parser.value(optPeriod).toLongLong();
    if(parser.isSet(optNumber)) {
        bool status;
        QString str = parser.value(optNumber);
        int n = str.toInt(&status);
        if(!status) {
            qDebug() << "Неверно задан параметр n: " << str;
            exit(1);
        } else
            for(int i = 1; i < n + 1; i++)
                chs.push_back(i);
    } else {
        QStringList strCh = parser.value(optChannels).simplified().split(" ");
        if(!strCh.isEmpty()){
            bool status;
            for(auto s : strCh) {
                chs.push_back(s.toInt(&status));
                if(!status) {
                    qDebug() << "Не верно задан идентификатор канала: " << s << endl;
                    exit(1);
                }
            }
        }
    }
    QString strT = parser.value(optType);
    if(strT == "8")
        prec = new Rec<qint8>(chs, rel, abs, period);
    else if(strT == "u8")
        prec = new Rec<quint8>(chs, rel, abs, period);
    else if(strT == "16")
        prec = new Rec<qint16>(chs, rel, abs, period);
    else if(strT == "u16")
        prec = new Rec<quint16>(chs, rel, abs, period);
    else if(strT == "32")
        prec = new Rec<qint32>(chs, rel, abs, period);
    else if(strT == "u32")
        prec = new Rec<quint32>(chs, rel, abs, period);
    else if(strT == "64")
        prec = new Rec<qint64>(chs, rel, abs, period);
    else {
        qDebug() << "Не верно задан тип отсчёта: " << strT << endl;
        exit(1);
    }

    title = parser.value(optBanner);
    return prec;
}

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QVector<qint32> chs;    //идентификаторы каналов в программе
    QString title;  //заголовок окна приложения
    Receiver* pReceiver = init(chs, title);    //приёмник данных
    Oscilloscope osc(chs, title);
    osc.show();
    InputFifoThread ft(pReceiver, osc);
    ft.start();
    return a.exec();
}
