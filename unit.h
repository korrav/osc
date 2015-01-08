#ifndef UNIT_H
#define UNIT_H
#include <map>
#include <memory>
using std::map;

struct Unit {       /*логическая единица входных данных*/
    typedef  map<qint32,qint64> Data;
    qint64 atime;
    qint64 dtime;
    Data data;
};

typedef std::shared_ptr<Unit> PUnit;
typedef std::shared_ptr<const Unit> PConstUnit;

#endif // UNIT_H
