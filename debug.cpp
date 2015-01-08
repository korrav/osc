#include "debug.h"
QString deb_describeUnit(const Unit& u) {
    QString str;
    QTextStream stream(&str);
    stream  << "Absolute time  =" << u.atime  << " ns" << endl
            << "Period =" << u.dtime << " ns" << endl;
    for(auto d : u.data)
        stream << "Channel " << d.first << " value = " << d.second << endl;
    return str;
}
