#ifndef CLIPBOARD_H
#define CLIPBOARD_H

#include <QObject>
#include <QThread>
#include <QWaitCondition>
#include <QMutex>
#include <QMutexLocker>
#include <deque>
#include <queue>
#include <memory>
#include <mutex>
#include <vector>
#include "unit.h"
#include "writetask.h"
#include "controltrigger.h"
#include <utility>
using std::deque;
using std::shared_ptr;
using std::queue;
using std::vector;

class ControlTrigger;
class WriteTask;

class ClipBoard : public QThread    /*Производит обработку входного потока данных*/
{
    Q_OBJECT
    typedef deque<PUnit> QueuePUnit;
    typedef queue<PUnit> InQueue;

public:
    enum mode_screen {
        STOP, //обновление экрана остановлено
        RUN,  //экран постоянно обновляется
        SINGLE_RUN    //режим единичного обновления
    } ;        //режим работы экрана осциллографа
    explicit ClipBoard(QObject* parent = 0);
    virtual ~ClipBoard();
    void pass(const PUnit& pu);   //передача блока данных во входную очередь
    bool dataOW(vector<PUnit>& v, int& begin, bool& isChangeFreq);  //получение содержимого окна визуализации
    std::pair<qint64, vector<PConstUnit>> getSnippetSignal(qint64 timeBegin, qint64 timeEnd);    /*получение
        фрагмента сигнала в диапазоне временных значений [timeBegin,timeEnd], при этом также возвращается
        точное время первого значения*/
    void setFocus(int x);    //команда установить фокус на новое положение
    void setSize(int s);    //команда изменить размер буфера визуализации
    int getFocus(void) const;
    int getSize(void) const;
    std::pair<qint64, qint64> getTimeSpanSignal(void);    //получить временной интервал детектированного сигнала
    qint64 getSamplingPeriod();
private:
    bool receipt(void); //приём данных из входной очереди. Возвращает false, если в течение ожидания не были получены данные из входной очереди
    void run(void); //основная функция потока
    void rewriteRQWQ(void);   //обновление содержимого буфера визуализации, а также рабочей очереди
    void write(int f);   //запись данных в файл. В качестве параметра передаётся текущий фокус

    const static int DEFAULT_NUM_ELEM_RQ = 1000;
    const static int PERIOD_UPDATE_VEIW = 50;   //период обновления буфера визуализации (мс)
    const static ulong PERIOD_WAIT_RECEIV_DATA = 500;   //период ожидания приёма данных из входной очереди (мс)
    QueuePUnit rQ_;    //рабочая очередь
    struct {
        int focus = DEFAULT_NUM_ELEM_RQ/2; //фокус
        QueuePUnit data;  //буфер визуализации
        QMutex mut;  //мьютекс, регулирующий доступ к структуре
    } vQ_;
    int focus_ = DEFAULT_NUM_ELEM_RQ/2; //расположение фокуса в рабочей области
    int size_ = DEFAULT_NUM_ELEM_RQ;    //размер рабочей области
    bool isUpdate_ = true;  //если true, то разрешено обновлять буфер визуализации
    ControlTrigger cTrigger_;    //оперирует алгоритмами детектирования триггеров
    WriteTask* wrTask_;   //указывает на текущее Задание на запись данных
    QMutex mutInQ_;   //регулирует доступ к элементам входной очереди
    InQueue inQ_;    //входная очередь
    QWaitCondition waitInQ_;    //условная переменная заполнения входной очереди
    mode_screen mScreen_ = STOP; //текущий режим обновления экрана
protected:
    virtual void timerEvent(QTimerEvent *) {
        isUpdate_ = true;
    }

signals:
    void issueDebug(QString str);   //испускает отладочное сообщение
    void updateVQ(void);    //сообщает о том, что содержимое буфера визуализации было изменено
    void completeWrite(WriteTask::Error);   //сообщение, о том, что задание на запись завершено, сопровождаемое кодом возможной ошибки
    void transitionModeScreen(mode_screen m);   //сообщает о переходе в новый режим работы отображения экрана осциллографа
    void openFileDataComplete(QString, bool);   //сообщает о завершении процесса загрузки файла данных. Если false - процесс завершился неудачно
    void writeTrigger(qint64 number);  //количество записанных триггеров
    void endSingleRun(void);    //окончание режима SINGLE_RUN
public slots:
    void passWriteTask(WriteTask *pw); //передача Задачи на запись
    void deactivWriteTask(void);    //деактивация Задания на запись
    void setTrigger(const AbstractTrigger& t);  //добавить триггерное условие
    void removeTrigger(qint32 ch, const QString& name); //удалить триггерное условие
    void removeAllTrigger(void); //удалить все триггерные условия
    void resetAllTrigger(void); //сбросить состояния всех триггерных устройств
    void openFileData(QString name);    //открыть файл данных
    mode_screen getModeScreen(void) {
        return mScreen_;
    }
    void setModeScreenRun(void);    //переход в режим непрерывной работы обновления экрана осциллографа
    void setModeScreenStop(void);   //переход в режим остановки обновления экрана осциллографа
    void setModeScreenSingleRun(void);  //переход в режим непрерывной работы единичного обновления экрана осциллографа
    void setSizeAndFocus(int size, int focus);   //изменить размер и фокус буфера визуализации
};

#endif // CLIPBOARD_H
