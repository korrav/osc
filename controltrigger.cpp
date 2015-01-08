#include "controltrigger.h"
#include <QDebug>
RiseTrigger::RiseTrigger(const int &ch, const qint64 &level): AbstractTrigger(ch, "Rise"), lev_(level)
{
}

bool RiseTrigger::check(const PUnit& cur)
{
    auto pcurChanData =  cur->data.find(ch_);
    if(pcurChanData == cur->data.end()) {
        isInit_ = false;
        return false;
    }
    qint64 val = pcurChanData->second;
    if(!isInit_) {
        if(/*pcurChanData->second*/ val >= lev_)
            return false;
        else {
            isInit_ = true;
            return false;
        }
    }
    if(pcurChanData->second >= lev_) {
        isInit_ = false;
        qDebug() << "Сработало триггерное Rise условие";
        return true;
    }
    return false;
}

RiseTrigger *RiseTrigger::clone() const
{
    return new RiseTrigger(*this);
}

void RiseTrigger::reset()
{
    isInit_ = false;
}

bool FallTrigger::check(const PUnit &cur)
{
    auto pcurChanData =  cur->data.find(ch_);
    if(pcurChanData == cur->data.end()) {
        isInit_ = false;
        return false;
    }
    if(!isInit_) {
        if(pcurChanData->second <= lev_)
            return false;
        else {
            isInit_ = true;
            return false;
        }
    }
    if(pcurChanData->second <= lev_) {
        isInit_ = false;
        qDebug() << "Сработало триггреное Fall условие";
        return true;
    }
    return false;
}

FallTrigger *FallTrigger::clone() const
{
    return new FallTrigger(*this);
}

void FallTrigger::reset()
{
    isInit_ = false;
}

void ControlTrigger::add(const AbstractTrigger &t)
{
    qDebug() << "Добавлен триггер" << t.getName() << "для канала " << t.getChannel();
    std::shared_ptr<AbstractTrigger> pTr(t.clone());
    auto res = listTask_.insert(pTr);
    if(!res.second) {
        listTask_.erase(res.first);
        listTask_.insert(pTr);
    }
}

void ControlTrigger::clear(qint32 ch,const QString& name)
{
    listTask_.erase(std::make_shared<AbstractTrigger>(ch, name));
}

void ControlTrigger::clearAll()
{
    qDebug() << "Сброшены все триггеры";
    listTask_.clear();
}

bool ControlTrigger::check(const PUnit &cur)
{
    bool status = false;
    if(listTask_.empty())
        status = true;
    else {
        for(auto& t : listTask_) {
            if(t->check(cur))
                status = true;
        }
    }
    return status;
}

void ControlTrigger::reset()
{
    for(auto& t : listTask_)
        t->reset();
}

ControlTrigger::ControlTrigger(): listTask_(compare)
{
}


FallTrigger::FallTrigger(const int &ch, const qint64 &level): AbstractTrigger(ch, "Fall"), lev_(level)
{
}


QString AbstractTrigger::getName() const
{
    return name_;
}

bool compare(const std::shared_ptr<AbstractTrigger>& l, const std::shared_ptr<AbstractTrigger>& r)
{
    if(l->ch_ == r->ch_)
        return l->name_ < r->name_;
    else
        return l->ch_ < r->ch_;
}
