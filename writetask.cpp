#include "writetask.h"
#include "manipulatordata.h"
#include <limits>
#include <QDebug>

WriteTask::WriteTask(QVariant begin, QVariant end, QString name, QString patch):      //класс регулирующий запись данных в файлы
    beg_(begin), end_(end), nameFile_(name), patch_(patch), curNumber_(0), error_(OK), status_(STOP)
{
    if((beg_.type() != QVariant::DateTime && beg_.type() != QVariant::LongLong) ||
            (end_.type() != QVariant::DateTime && end_.type() != QVariant::LongLong)) {
        qDebug() << "Неверно заданное задание на запись";
        deactivate();
        return;
    }
    if (patch_.isEmpty())
        patch_ = QDir::currentPath();
    else if(!QDir(patch_).exists())
        error_ = WRONG_DIR;
    if(beg_.type()  == QVariant::DateTime && end_.type() == QVariant::DateTime
            && beg_.toDateTime() >= end_.toDateTime())
        error_ = WRONG_TIME_END;
    tim_.setSingleShot(true);
}

bool WriteTask::write(const std::deque<PUnit> &data, int focus)
{
    if(status_ == DEACTIVATED)
        return false;
    else if(status_ == COMPLETE)
        return true;
    else if(status_ == STOP) {
        if(beg_.type() == QVariant::LongLong) {
            beg_ = beg_.toLongLong() - 1;
            if(beg_.toLongLong() <= 0)
                activate();
            else
                return false;
        } else
            return false;
    }
    //запись файла данных
    QString nfile = (nameFile_.isEmpty() ? QDateTime::currentDateTime().toString("ddMMyy_hhmmsszzz") : nameFile_)
            + "_" + QString::number(curNumber_++);
    nfile = QDir(patch_).absoluteFilePath(nfile);
    ManipulatorData manip(nfile);
    if(!manip.write(focus, data)) {
        error_ = WRONG_FILE_WRITE;
        complete();
        return true;
    }
    emit writeTrigger(curNumber_);
    //проверка на окончание Задания на запись
    if (end_.type() == QVariant::LongLong) {
        end_ = end_.toLongLong() - 1;
        if(end_.toLongLong() <= 0) {
            complete();
            return true;
        }
    }
    return false;
}

void WriteTask::start()
{
    if(status_ == DEACTIVATED) {
        qDebug() << "При старте задание на запись уже было деактивированно";
        return;
    }
    if(error_ != OK) {
        qDebug() << "При старте задание на запись уже было завершено";
        complete();
        return;
    }
    if(status_ != STOP || beg_.type() != QVariant::DateTime)
        return;
    connect(&tim_, SIGNAL(timeout()), this, SLOT(activate()));
    qint64 inter64 = QDateTime::currentDateTime().msecsTo(beg_.toDateTime());
    if(inter64 <= 0) {
        activate();
        return;
    } else
        if(std::numeric_limits<int>::max() <= inter64)
            tim_.setInterval(std::numeric_limits<int>::max());
        else
            tim_.setInterval(static_cast<int>(inter64));
}

void WriteTask::deactivate()
{
    tim_.stop();
    tim_.disconnect();
    status_ = DEACTIVATED;
}

void WriteTask::activate()
{
    if(status_ == DEACTIVATED)
        return;
    tim_.disconnect();
    if(end_.type() == QVariant::DateTime) {
        connect(&tim_, SIGNAL(timeout()), this, SLOT(complete()));
        qint64 inter64 = QDateTime::currentDateTime().msecsTo(end_.toDateTime());
        if(inter64 <= 0) {
            complete();
            return;
        } else
            if(std::numeric_limits<int>::max() <= inter64)
                tim_.setInterval(std::numeric_limits<int>::max());
            else
                tim_.setInterval(static_cast<int>(inter64));
    }
    status_ = RUN;
}

void WriteTask::complete()
{
    if(status_ == DEACTIVATED)
        return;
    tim_.stop();
    tim_.disconnect();
    status_ = COMPLETE;
}
