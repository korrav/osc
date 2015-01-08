#ifndef RECEIVER_H
#define RECEIVER_H

#include <QtWidgets>
#include "fcntl.h"
#include "unistd.h"
#include "unit.h"
#include "debug.h"
#include "oscilloscope.h"
#include <utility>
#include <memory>

using std::pair;
using std::shared_ptr;

class Receiver { /*Базовый класс приёмника входных данных*/
public:
    Receiver(const QVector<qint32>& v, size_t sB,  bool isDtime = false,
             bool isATime = false, qint64 d = 1000000000) :
        isD_(isDtime), isA_(isATime),  dtime_(d), chs_(v) {
        buffer_ = createBuffer(sB);
    }
    virtual ~Receiver() {
        delete [] buffer_.first;
        return;
    }

    shared_ptr<Unit> createUnit(void) {
        if(!obtainBuffer())
            return nullptr;
        return fillDataUnit();
    }
protected:
    bool isD_;
    bool isA_;
    qint64 dtime_;
    QVector<qint32> chs_;
    pair<char*, size_t> buffer_;
private:
    pair<char*, size_t> createBuffer(size_t sizeBuf) {
        size_t s = sizeBuf * chs_.size() + (isD_ ? sizeof(qint64) : 0) + (isA_ ? sizeof(qint64) : 0);
        return {new char[s], s};
    }
    bool obtainBuffer(void){
        return !(read(STDIN_FILENO, buffer_.first, buffer_.second) == 0);
    }
    virtual shared_ptr<Unit> fillDataUnit(void) = 0;
};

template <typename T>
class Rec : public Receiver {   /*Реализация приёмника входных данных*/
public:
    Rec(const QVector<qint32>& v, bool isDtime = false,
        bool isATime = false, qint64 d = 1000000000):
        Receiver(v, sizeof(T), isDtime, isATime, d) {

    }

private:
    virtual shared_ptr<Unit> fillDataUnit(void) {
        Unit* u = new Unit;
        qint64* pt = reinterpret_cast<qint64*>(buffer_.first);
        if(isA_)
            u->atime = *pt++;
        else
            u->atime = -1;
        if(isD_)
            u->dtime = *pt++;
        else
            u->dtime = dtime_;
        T* pd = reinterpret_cast<T*>(pt);
        for(auto ch : chs_) {
            u->data[ch] = *(pd++);
            /*qDebug() << "Для канала " << ch << " значение = " << *(pd - 1);*/
        }
        return  shared_ptr<Unit>(u);
    }
};

class InputFifoThread : public QThread {  /*Отдельный поток, ведающий приёмом и передачей на обработку входных данных*/
    Q_OBJECT
    Receiver* rec_;
    Oscilloscope& osc_;
signals:
    void new_value(QString);
    void warning(QString);
public:
    InputFifoThread(Receiver* r, Oscilloscope& osc): rec_(r), osc_(osc){}
    ~InputFifoThread(){}
    void run(void) {
        shared_ptr<Unit> pu;
        for(;;){
            pu = rec_->createUnit();
            if (pu == nullptr) {
                emit warning("Источник данных был отцеплен от входного канала программы!!!");
                return;
            }
            osc_.pass(pu);
        }
    }
};

#endif // RECEIVER_H
