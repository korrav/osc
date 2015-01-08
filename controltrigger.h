#ifndef CONTROLTRIGGER_H
#define CONTROLTRIGGER_H
#include <QtGlobal>
#include <QString>
#include <set>
#include <memory>
#include "unit.h"
using std::set;

class AbstractTrigger {
protected:
    qint32 ch_;
private:
    QString name_;
public:
    AbstractTrigger(qint32 ch, QString name): ch_(ch), name_(name) {}
    int getChannel(void) const {
        return ch_;
    }
    QString getName(void) const;    //возвращает имя триггера
    virtual bool check(const PUnit&) { return false;} //провести проверку на выполнение условия триггера
    virtual AbstractTrigger* clone(void) const { return new AbstractTrigger(*this);}   //возвращает копию объекта
    virtual void reset(void){}   //сброс состояния триггера
    friend bool compare(const std::shared_ptr<AbstractTrigger>& l, const std::shared_ptr<AbstractTrigger>& r); //функция сравнения
};

bool compare(const std::shared_ptr<AbstractTrigger> &l, const std::shared_ptr<AbstractTrigger> &r);

class RiseTrigger: public AbstractTrigger {
    bool isInit_ = false;
    PUnit prev_;
    qint64 lev_;
public:
    RiseTrigger(const int& ch, const qint64& level);
    bool check(const PUnit& cur);
    RiseTrigger* clone(void) const;
    void reset(void);
};

class FallTrigger: public AbstractTrigger {
    bool isInit_ = false;
    PUnit prev_;
    qint64 lev_;
public:
    FallTrigger(const int& ch, const qint64& level);
    bool check(const PUnit& cur);
    FallTrigger* clone(void) const;
    void reset(void);
};

class ControlTrigger
{
    set<std::shared_ptr<AbstractTrigger>, decltype(compare)*> listTask_;
public:
    ControlTrigger();
    void add(const AbstractTrigger& t); //активация триггера
    void clear(qint32 ch, const QString &name); //деактивация триггера
    void clearAll(void); //деактивация всех триггеров
    bool check(const PUnit &cur);   //проверка на соблюдения условия триггера
    void reset(void);   //сброс состояния всех активированных триггеров
};

#endif // CONTROLTRIGGER_H
