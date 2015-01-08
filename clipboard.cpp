#include "clipboard.h"
#include "debug.h"
#include <QCoreApplication>
#include "manipulatordata.h"
#include <QDebug>
#include <cmath>

ClipBoard::ClipBoard(QObject *parent): QThread(parent), rQ_(DEFAULT_NUM_ELEM_RQ)
{
    wrTask_ = new WriteTask;
    startTimer(PERIOD_UPDATE_VEIW);
}

ClipBoard::~ClipBoard()
{
    delete wrTask_;
}

void ClipBoard::pass(const PUnit &pu) {
    QMutexLocker l(&mutInQ_);
    inQ_.push(pu);
    waitInQ_.wakeAll();
    return;
}

std::pair<qint64, vector<PConstUnit> > ClipBoard::getSnippetSignal(qint64 timeBegin, qint64 timeEnd)
{
    QMutexLocker l(&vQ_.mut);
    qint64 validTimeBegin = timeBegin;
    if(vQ_.data.empty() )
        return std::make_pair(validTimeBegin, vector<PConstUnit>());
    PUnit firstPUnit = *(vQ_.data.begin());
    qint64 period = firstPUnit->dtime;
    if(period == 0)
        return std::make_pair(validTimeBegin, vector<PConstUnit>());
    int offsetFromFocusBeginUnit = std::floor(static_cast<double>(timeBegin)/period);
    int indexBeginUnit = vQ_.focus + offsetFromFocusBeginUnit;
    if(indexBeginUnit < 0)
        indexBeginUnit = 0;
    int offsetFromFocusEndUnit = std::ceil(static_cast<double>(timeEnd)/period);
    int indexEndUnit = offsetFromFocusEndUnit + vQ_.focus + 1;
    if(indexEndUnit > static_cast<int>(vQ_.data.size()))
        indexEndUnit = vQ_.data.size();
    if(indexEndUnit <= indexBeginUnit)
        return std::make_pair(validTimeBegin, vector<PConstUnit>());
    vector<PConstUnit> snippetSignal(vQ_.data.begin() + indexBeginUnit, vQ_.data.begin() + indexEndUnit);
    validTimeBegin = (indexBeginUnit - vQ_.focus) * period;
    return std::make_pair(validTimeBegin, std::move(snippetSignal));
}

void ClipBoard::setFocus(int x)
{
    if(x < 0)
        focus_ = 0;
    else if (x >= size_)
        focus_ = size_ - 1;
    else
        focus_ = x;
}

void ClipBoard::setSize(int s)
{
    if(s < 1)
        s = 1;
    size_ = s;
}

int ClipBoard::getFocus() const
{
    return focus_;
}

int ClipBoard::getSize() const
{
    return size_;
}

std::pair<qint64, qint64> ClipBoard::getTimeSpanSignal()
{
    QMutexLocker l(&vQ_.mut);
    if(vQ_.data.empty())
        return std::pair<qint64, qint64>(0,0);
    PUnit firstPUnit = *(vQ_.data.begin());
    qint64 period = firstPUnit->dtime;
    return std::make_pair( period * (-vQ_.focus), period * (vQ_.data.size())- 1 - vQ_.focus);
}

bool ClipBoard::receipt()
{
    static qint64 dtime = 0;
    PUnit pu;
    //mutInQ_.lock();
    QMutexLocker l(&mutInQ_);
    while(inQ_.empty()){
        if(!waitInQ_.wait(l.mutex(), PERIOD_WAIT_RECEIV_DATA))
            return false;
    }
    pu = inQ_.front();
    inQ_.pop();
    l.unlock();
    //issueDebug(deb_describeUnit(*pu));
    if(pu->dtime != dtime) {
        dtime = pu->dtime;
        rQ_.clear();
    }
    rQ_.push_back(pu);
    int sizeExtra  = rQ_.size() - size_;

    if (sizeExtra > 0) {
        rQ_.erase(rQ_.begin(), rQ_.begin() + sizeExtra);
        return true;
    } else
        return false;
}

void ClipBoard::run() {
    for(;;){
        //обработка всех событий
        QCoreApplication::processEvents();
        //приём отсчётов от входной очереди
        if(!receipt())
            continue;
        //проверка на выполнения триггерного условия
        if(cTrigger_.check(rQ_[focus_])) {
            //            emit issueDebug(" Сработало триггерное условие");
            //запись данных
            write(focus_);
            //обновление изображения экрана осциллографа
            if(isUpdate_ && mScreen_ != STOP) {
                rewriteRQWQ();
                if(mScreen_ == SINGLE_RUN) {
                    setModeScreenStop();
                    emit endSingleRun();
                }
            }
        }

    }
}

void ClipBoard::rewriteRQWQ()
{
    QMutexLocker l(&vQ_.mut);
    vQ_.data.swap(rQ_);
    vQ_.focus = focus_;
    rQ_.clear();
    isUpdate_ = false;
    l.unlock();
    emit updateVQ();
}

void ClipBoard::write(int f)
{
    if(wrTask_->write(rQ_, f)) {
        WriteTask::Error error = wrTask_->getError();
        QString messError = "";
        switch(error) {
        case WriteTask::OK:
            messError = "успешно";
            break;
        case WriteTask::WRONG_DIR:
            messError = "с ошибкой неверного заданной директории";
            break;
        case WriteTask::WRONG_TIME_END:
            messError = "с ошибкой неверно заданным временем окончания задания записи";
            break;
        case WriteTask::INVALIDATE:
            messError = "с ошибкой неверно оформленного задания на запись";
            break;
        case WriteTask::WRONG_FILE_WRITE:
            messError = "с ошибкой невозможности открыть файл на запись";
            break;
        default:
            messError = "с неизвестной ошибкой";
        }
        qDebug() << "Задание на запись завершено " << messError;
        emit(completeWrite(error));
    }
}

void ClipBoard::passWriteTask(WriteTask* pw)
{
    wrTask_->deactivate();
    delete wrTask_;
    wrTask_ = pw;
    wrTask_->start();
    connect(wrTask_, SIGNAL(writeTrigger(qint64)), SIGNAL(writeTrigger(qint64)));
}

void ClipBoard::deactivWriteTask()
{
    wrTask_->deactivate();
}

void ClipBoard::setTrigger(const AbstractTrigger &t)
{
    cTrigger_.add(t);
}

void ClipBoard::removeTrigger(qint32 ch, const QString &name)
{
    cTrigger_.clear(ch, name);
}

void ClipBoard::removeAllTrigger()
{
    cTrigger_.clearAll();
}

void ClipBoard::resetAllTrigger()
{
    cTrigger_.reset();
}

void ClipBoard::openFileData(QString name)
{
    boost::optional<ManipulatorData::return_type_read> data = ManipulatorData(name).read();
    QMutexLocker l(&vQ_.mut);
    if(data) {
        vQ_.focus = data->first;
        vQ_.data = std::move(data->second);
        l.unlock();
        rewriteRQWQ();
        setModeScreenStop();
        emit openFileDataComplete(name, true);
    } else
        emit openFileDataComplete(name, false);
}

void ClipBoard::setModeScreenRun()
{
    mScreen_ = RUN;
    emit transitionModeScreen(RUN);
}

void ClipBoard::setModeScreenStop()
{
    mScreen_ = STOP;
    emit transitionModeScreen(STOP);
}

void ClipBoard::setModeScreenSingleRun()
{
    mScreen_ = SINGLE_RUN;
    emit transitionModeScreen(SINGLE_RUN);
}

void ClipBoard::setSizeAndFocus(int size, int focus)
{
    setSize(size);
    setFocus(focus);
}
