#ifndef WRITETASK_H
#define WRITETASK_H
#include <QtGlobal>
#include <QString>
#include <memory>
#include <QVariant>
#include <QDir>
#include <QDateTime>
#include <QDataStream>
#include <QTimer>
#include "unit.h"
#include <deque>

class WriteTask: public QObject
{
    Q_OBJECT
public:
    enum Error {    //ошибки
        OK,	//нет ошибок
        WRONG_DIR,	//неправильно задана директория, куда будет производиться запись файлов данных
        WRONG_TIME_END,	//неверно задано время окончания записи
        INVALIDATE, //неверно оформленное Задание на запись
        WRONG_FILE_WRITE,	//невозможно открыть файл на запись

    };
private:
    enum Stat {     //состояние Задания записи
        DEACTIVATED,        //Задание деактивированно (из этого состояния Задание уже перейти в другое не способно)
        STOP,   //Задание ещё не запущено
        RUN,    //Выполнение Задания запущено
        COMPLETE,   //Выполнение задания завершено
    };
    QVariant beg_; //начало записи
    QVariant end_; //окончание записи
    QString nameFile_; //префикс названия файла. Если ="", то в качестве него выступает дата записи
    QString patch_; //каталог, в котором будут записиваться файлы данных
    qint64 curNumber_; //текущий порядковый номер триггера
    Error error_;   //текущая ошибка
    Stat status_;  //текущее состояние
    QTimer tim_; //таймер, который используется для деактивации триггерного условия, в случае, если установлена дата его окончания
public:
    WriteTask(QVariant begin = QVariant(), QVariant end = QVariant(), QString name = "", QString patch = "");
    void start(void);    //активация Задания на запись
    inline qint64 getNumDetectTrigger(void) const { //возвращает количество детектированных триггеров
        return curNumber_;
    }
    inline WriteTask::Error getError(void) {
        if(status_ == COMPLETE)
            deactivate();
        return error_;
    }

    bool write(const std::deque<PUnit>& data, int focus);	/*запись данных в файл. Возвращает true, если Задание на запись завершено,
                                                                 false -если оно всё ещё действует или деактивировано*/
private slots:
    void activate(void);     //активация триггерного условия
    void complete(void);    //переход в состояние Задание на запись завершено
public slots:
    void deactivate(void);  //деактивация триггерного условия
signals:
    void writeTrigger(qint64 number);   //сообщает количество записанных триггеров
};

#endif // WRITETASK_H
