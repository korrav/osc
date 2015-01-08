#include "manipulatordata.h"
#include <QDataStream>

void ManipulatorData::writeUnit(const PUnit &pbuf, QDataStream &s)
{
    s << pbuf->atime << pbuf->dtime << static_cast<qint64>(pbuf->data.size());
    for(auto e : pbuf->data)
        s << e.first << e.second;
}

PUnit ManipulatorData::readUnit(QDataStream &s)
{

    PUnit pu(new Unit);
    qint64 size;
    s >> pu->atime >> pu->dtime >> size;
    qint32 ch;
    qint64 value;
    for(; size > 0; size--) {
        s >> ch >> value;
        (pu->data)[ch] = value;
    }
    return pu;
}

ManipulatorData::ManipulatorData(const QString &n)
{
    file_ = new QFile(n);
}

ManipulatorData::~ManipulatorData()
{
    delete file_;
}

bool ManipulatorData::write(int focus, const std::deque<PUnit> &de)
{
    if(!file_->open(QIODevice::WriteOnly))
        return false;
    QDataStream s(file_);
    qint64 size = de.size();
    s << VERSION << focus << size;
    for(auto const& e : de)
        writeUnit(e, s);
    file_->close();
    if(s.status() != QDataStream::Ok)
        return false;
    return true;
}

boost::optional<ManipulatorData::return_type_read> ManipulatorData::read()
{
    if(!file_->open(QIODevice::ReadOnly))
        return boost::none;
    QDataStream s(file_);
    qint8 version;
    s   >> version;
    if(version > VERSION)
        return boost::none;
    int focus;
    std::deque<PUnit> v;
    qint64 size;
    s >> focus >> size;
    if(focus < size - 1)
        return boost::none;
    for(; size > 0; size--)
        v.push_back(readUnit(s));
    file_->close();
    if(s.status() != QDataStream::Ok)
        return boost::none;
    return make_pair(focus, std::move(v));
}
