#ifndef MANIPULATORDATA_H
#define MANIPULATORDATA_H
#include <QtGlobal>
#include <QFile>
#include "unit.h"
#include <deque>
#include <QDataStream>
#include <boost/optional.hpp>
#include <utility>

class ManipulatorData
{
protected:
    const qint8 VERSION = 1; //версия хидера, который может создаваться данной программой
    QFile *file_;
    virtual void writeUnit(const PUnit &pbuf, QDataStream &s);  //запись в файл единичного блока данных
    virtual PUnit readUnit(QDataStream &s); //чтение единичного блока данных из файла
public:
    typedef std::pair<int, std::deque<PUnit>> return_type_read;
    ManipulatorData(const QString &n);
    virtual ~ManipulatorData();
    virtual bool write(int focus, const std::deque<PUnit>& de); //сохранение файла данных
    virtual boost::optional<return_type_read> read(void);
};

#endif // MANIPULATORDATA_H
