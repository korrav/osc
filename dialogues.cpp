#include "dialogues.h"
#include <QGridLayout>
#include <QVBoxLayout>
#include <QLabel>
#include <QEvent>
#include <algorithm>
#include <random>
#include <QFocusEvent>
#include <QIntValidator>
#include <QDoubleValidator>
#include <QSizePolicy>
#include <QGroupBox>
#include <QtAlgorithms>
#include <QBrush>
#include <QList>
#include <QPalette>
#include <QColorDialog>
#include <QFontDialog>
qint64 SetupNewLocationWindowVisualization::getFactor(QString str)
{
    qint64 factor = 0;
    if(str == TEXT_NANOSECOND)
        factor = 1;
    else if(str == TEXT_MICROSECOND)
        factor = 1000;
    else if(str == TEXT_MILISECOND)
        factor = 1e6;
    else if(str == TEXT_SECOND)
        factor = 1e9;
    return factor;
}

QStringList SetupNewLocationWindowVisualization::getListUnitMinTime(qint64 factorUpperBound)
{
    QStringList list(TEXT_NANOSECOND);
    if(factorUpperBound >= 1000)
        list << TEXT_MICROSECOND;
    if(factorUpperBound >= 1e6)
        list << TEXT_MILISECOND;
    if(factorUpperBound >= 1e9)
        list << TEXT_SECOND;
    return list;
}

QStringList SetupNewLocationWindowVisualization::getListUnitMaxTime(qint64 factorUnderBound)
{
    QStringList list(TEXT_UNITS);
    if(factorUnderBound > 1000)
        list.removeAt(0);
    if(factorUnderBound > 1e6)
        list.removeAt(1);
    if(factorUnderBound > 1e9)
        list.removeAt(2);
    return list;
}

bool SetupNewLocationWindowVisualization::readInputs(std::array<qint64,4>& location)
{
    location[0] = pSpinBoxMinTime_->value() * getFactor(pComboMinTime_->currentText());
    location[1] = pSpinBoxMaxTime_->value() * getFactor(pComboMaxTime_->currentText());
    if(location[0] >= location[1]) {
        pSpinBoxMinTime_->setStyleSheet("color: red");
        pSpinBoxMaxTime_->setStyleSheet("color: red");
        return false;
    } else {
        pSpinBoxMinTime_->setStyleSheet("color");
        pSpinBoxMaxTime_->setStyleSheet("color");
    }
    location[2] = pSpinBoxMinValue_->value();
    location[3] = pSpinBoxMaxValue_->value();
    if(location[2] >= location[3]) {
        pSpinBoxMinValue_->setStyleSheet("color: red");
        pSpinBoxMaxValue_->setStyleSheet("color: red");
        return false;
    } else {
        pSpinBoxMinValue_->setStyleSheet("color");
        pSpinBoxMaxValue_->setStyleSheet("color");
    }
    return true;
}

SetupNewLocationWindowVisualization::SetupNewLocationWindowVisualization(const ScreenWidget &screen, QWidget *parent): QDialog(parent),
    TEXT_UNITS(QStringList() << TEXT_NANOSECOND << TEXT_MICROSECOND << TEXT_MILISECOND << TEXT_SECOND)
{
    pSpinBoxMinTime_ = new QDoubleSpinBox;
    pSpinBoxMinTime_->setDecimals(3);
    pSpinBoxMinTime_->setRange(-1e9, 1e9);

    pSpinBoxMaxTime_ = new QDoubleSpinBox;
    pSpinBoxMaxTime_->setDecimals(3);
    pSpinBoxMaxTime_->setRange(-1e9, 1e9);


    pSpinBoxMinValue_ = new QSpinBox;
    pSpinBoxMinValue_->setRange(std::numeric_limits<int>::lowest(), std::numeric_limits<int>::max());
    pSpinBoxMaxValue_ = new QSpinBox;
    pSpinBoxMaxValue_->setRange(std::numeric_limits<int>::lowest(), std::numeric_limits<int>::max());

    pComboMinTime_ = new QComboBox(this);
    pComboMinTime_->addItems(TEXT_UNITS);
    pComboMaxTime_ = new QComboBox(this);
    pComboMaxTime_->addItems(TEXT_UNITS);

    QLabel* plMinTime = new QLabel("&Start time");
    plMinTime->setBuddy(pSpinBoxMinTime_);
    QLabel* plMaxTime = new QLabel("&End time");
    plMaxTime->setBuddy(pSpinBoxMaxTime_);
    QLabel* plMinValue = new QLabel("S&tart value");
    plMinValue->setBuddy(pSpinBoxMinValue_);
    QLabel* plMaxValue = new QLabel("E&nd value");
    plMaxValue->setBuddy(pSpinBoxMaxValue_);
    pButtonOk_ = new QPushButton("&Ok");
    connect(pButtonOk_, SIGNAL(clicked()), this, SLOT(getOk()));
    pButtonCancel_ = new QPushButton("&Cancel");
    connect(pButtonCancel_, SIGNAL(clicked()), this, SLOT(getCancel()));
    pButtonApply_ = new QPushButton("&Apply");
    connect(pButtonApply_, SIGNAL(clicked()), this, SLOT(getApply()));

    QGridLayout *pglEditLocation = new QGridLayout;
    pglEditLocation->addWidget(plMinTime, 0, 0);
    pglEditLocation->addWidget(pSpinBoxMinTime_, 0, 1);
    pglEditLocation->addWidget(pComboMinTime_, 0, 2);
    pglEditLocation->addWidget(plMaxTime, 1, 0);
    pglEditLocation->addWidget(pSpinBoxMaxTime_, 1, 1);
    pglEditLocation->addWidget(pComboMaxTime_, 1, 2);
    pglEditLocation->addWidget(plMinValue, 2, 0);
    pglEditLocation->addWidget(pSpinBoxMinValue_, 2, 1);
    pglEditLocation->addWidget(plMaxValue, 3, 0);
    pglEditLocation->addWidget(pSpinBoxMaxValue_, 3, 1);
    pglEditLocation->addWidget(pButtonOk_, 4, 0);
    pglEditLocation->addWidget(pButtonCancel_, 4, 1);
    pglEditLocation->addWidget(pButtonApply_, 4, 2);
    setLayout(pglEditLocation);
    setWindowTitle("Select the location");
    setFixedSize(minimumSizeHint());
    connect(this, &SetupNewLocationWindowVisualization::changeLocation, &screen, &ScreenWidget::changePosVisualizationWindow);
}

void SetupNewLocationWindowVisualization::getOk()
{
    std::array<qint64, 4> location;
    if(readInputs(location)) {
        emit changeLocation(location);
        hide();
    }
}

void SetupNewLocationWindowVisualization::getCancel()
{
    hide();
}

void SetupNewLocationWindowVisualization::getApply()
{
    std::array<qint64, 4> location;
    if(readInputs(location))
        emit changeLocation(location);
}


QInt64Validator::QInt64Validator(QObject* parent, qint64 minimum, qint64 maximum): QValidator(parent), min_(minimum), max_(maximum)
{

}

void QInt64Validator::setTop(qint64 value)
{
    max_ = value;
}

void QInt64Validator::setBottom(qint64 value)
{
    min_ = value;
}

void QInt64Validator::setRange(qint64 bottom, qint64 top)
{
    min_ = bottom;
    max_ = top;
}

qint64 QInt64Validator::top() const
{
    return max_;
}

qint64 QInt64Validator::bottom() const
{
    return min_;
}

QValidator::State QInt64Validator::validate(QString &input, int&) const
{
    bool status;
    if(input.isEmpty() || input == "-" || input == "+")
        return  Intermediate;
    qint64 value = input.toLongLong(&status);
    if(!status)
        return Invalid;
    //    if(value < min_ || value > max_)
    //        return Invalid;
    if(value < min_)
        return Intermediate;
    else if(value > max_)
        return Invalid;
    else
        return Acceptable;
}

void QInt64Validator::fixup(QString &input) const
{
    qint64 value = input.toLongLong();
    if(value < min_) {
        value = min_;
        input = QString::number(value);
    }
}


SettingBufferVisualization::SettingBufferVisualization(const ClipBoard& clip, QWidget *parent = 0): QDialog(parent)
{
    pSpinBoxSize_ = new QSpinBox;
    pSpinBoxSize_->setRange(0, std::numeric_limits<int>::max());
    pSpinBoxSize_->setValue(clip.getSize());
    pSpinBoxFocus_ = new QSpinBox;
    pSpinBoxFocus_->setRange(0, std::numeric_limits<int>::max());
    pSpinBoxFocus_->setValue(clip.getFocus());

    QLabel* plSize = new QLabel("&Size");
    plSize->setBuddy(pSpinBoxSize_);
    QLabel* plFocus = new QLabel("&Focus");
    plFocus->setBuddy(pSpinBoxFocus_);

    pButtonOk_ = new QPushButton("&Ok");
    connect(pButtonOk_, SIGNAL(clicked()), this, SLOT(getOk()));
    pButtonCancel_ = new QPushButton("&Cancel");
    connect(pButtonCancel_, SIGNAL(clicked()), this, SLOT(getCancel()));

    QGridLayout *pglEditLocation = new QGridLayout;
    pglEditLocation->addWidget(plSize, 0, 0);
    pglEditLocation->addWidget(pSpinBoxSize_, 0, 1);
    pglEditLocation->addWidget(plFocus, 1, 0);
    pglEditLocation->addWidget(pSpinBoxFocus_, 1, 1);
    pglEditLocation->addWidget(pButtonOk_, 2, 0);
    pglEditLocation->addWidget(pButtonCancel_, 2, 1);
    setLayout(pglEditLocation);

    setWindowTitle("Setting buffer visualization");
    setFixedSize(minimumSizeHint());
    connect(this, SIGNAL(changeBufferVisualization(int,int)), &clip, SLOT(setSizeAndFocus(int,int)));
}

void SettingBufferVisualization::getOk()
{
    int size = pSpinBoxSize_->value();
    int focus = pSpinBoxFocus_->value();
    if(size == 0)
        pSpinBoxSize_->setStyleSheet("color: red");
    else if(focus >= size) {
        pSpinBoxSize_->setStyleSheet("color: red");
        pSpinBoxFocus_->setStyleSheet("color: red");
    } else {
        pSpinBoxSize_->setStyleSheet("color");
        pSpinBoxFocus_->setStyleSheet("color");
        emit changeBufferVisualization(size, focus);
        hide();
    }
}

void SettingBufferVisualization::getCancel()
{
    hide();
}


SettingTrigger::SettingTrigger(const QVector<qint32> &chs, ClipBoard* clip, QWidget *parent): QDialog(parent), chs_(chs), clip_(clip)
{
    pStackChannelTriggers_ = new QStackedWidget;
    pListChannels_ = new QListWidget;
    pListChannels_->setSelectionMode(QAbstractItemView::ExtendedSelection);
    qSort(chs_);
    QPixmap pixRise(":/rise_front");
    pixRise = pixRise.scaled(pixRise.size()/4, Qt::KeepAspectRatio);
    QPixmap pixFall(":/fall_front");
    pixFall = pixFall.scaled(pixFall.size()/4, Qt::KeepAspectRatio);
    QPixmap pixUpper(":/upper_level");
    pixUpper = pixUpper.scaled(pixUpper.size()/4, Qt::KeepAspectRatio);
    QPixmap pixUnder(":/under_level");
    pixUnder = pixUnder.scaled(pixUnder.size()/4, Qt::KeepAspectRatio);
    foreach (qint32 channel,chs_) {
        SettingTriggerChannel* pTriggerChannel = new SettingTriggerChannel(channel, pixRise, pixFall, pixUpper, pixUnder);
        pStackChannelTriggers_->addWidget(pTriggerChannel);
        ChannelListWidgetItem* pChannelItem= new ChannelListWidgetItem(QString::number(channel), pListChannels_);
        connect(pTriggerChannel, &SettingTriggerChannel::activate,
                [pChannelItem](){pChannelItem->activate();});
        connect(pTriggerChannel, &SettingTriggerChannel::deactivate, [pChannelItem](){pChannelItem->deactivate();});
    }
    connect(pListChannels_, SIGNAL(currentRowChanged(int)), pStackChannelTriggers_, SLOT(setCurrentIndex(int)));

    pButtonReset_ = new QPushButton("R&eset");
    connect(pButtonReset_, SIGNAL(clicked()), this, SLOT(handReset()));
    pButtonResetAll_ = new QPushButton("Reset &all");
    connect(pButtonResetAll_, SIGNAL(clicked()), this, SLOT(handResetAll()));

    QGridLayout *pglChannels = new QGridLayout;
    pglChannels->addWidget(pListChannels_,0,0,3,2);
    pglChannels->addWidget(pButtonReset_, 3, 0);
    pglChannels->addWidget(pButtonResetAll_, 3, 1);
    QGroupBox* pGroupChannels = new QGroupBox("Channels");
    pGroupChannels->setLayout(pglChannels);

    pButtonOk_ = new QPushButton("&Ok");
    connect(pButtonOk_, SIGNAL(clicked()), this, SLOT(handOk()));
    pButtonCancel_ = new QPushButton("&Cancel");
    connect(pButtonCancel_, SIGNAL(clicked()), this, SLOT(handCancel()));

    QGridLayout *pglEditLocation = new QGridLayout;
    pglEditLocation->addWidget(pGroupChannels, 0,0, 2, 1);
    pglEditLocation->addWidget(pStackChannelTriggers_, 0,1,2,1);
    pglEditLocation->addWidget(pButtonCancel_, 2,0,1,1);
    pglEditLocation->addWidget(pButtonOk_, 2,1,1,1);
    setLayout(pglEditLocation);
    setWindowTitle("Setting trigger");
    setFixedSize(minimumSizeHint());
}

void SettingTrigger::handOk()
{
    clip_->removeAllTrigger();

    for(int i = 0; i < pStackChannelTriggers_->count(); i++) {
        SettingTriggerChannel* channel =   static_cast<SettingTriggerChannel*>(pStackChannelTriggers_->widget(i));
        qint32 id = channel->idx();
        SettingTriggerChannel::TriggerSet triggersChannel = channel->getTriggers();
        for(auto& trigger : triggersChannel) {
            if(trigger.first == SettingTriggerChannel::RISE)
                clip_->setTrigger(RiseTrigger(id, trigger.second));
            else if(trigger.first == SettingTriggerChannel::FALL)
                clip_->setTrigger(FallTrigger(id, trigger.second));

        }
        hide();
    }
}

void SettingTrigger::handCancel()
{
    hide();
}

void SettingTrigger::handReset()
{
    QList<QListWidgetItem *> listSelectChannel = pListChannels_->selectedItems();
    foreach (QListWidgetItem* item, listSelectChannel) {
        int index = pListChannels_->row(item);
        static_cast<SettingTriggerChannel*>(pStackChannelTriggers_->widget(index))->reset();
    }
}

void SettingTrigger::handResetAll()
{
    for(int i = 0; i < pStackChannelTriggers_->count(); i++)
        static_cast<SettingTriggerChannel*>(pStackChannelTriggers_->widget(i))->reset();
}



qint32 SettingTriggerChannel::idx() const
{
    return idx_;
}

SettingTriggerChannel::TriggerSet SettingTriggerChannel::getTriggers()
{
    TriggerSet triggers;
    if(pSpinBoxRise_->isEnabled())
        triggers[RISE] = pSpinBoxRise_->value();
    if(pSpinBoxFall_->isEnabled())
        triggers[FALL] = pSpinBoxFall_->value();
    if(pSpinBoxUpper_->isEnabled())
        triggers[UPPER] = pSpinBoxUpper_->value();
    if(pSpinBoxUnder_->isEnabled())
        triggers[UNDER] = pSpinBoxUnder_->value();
    return triggers;
}

std::shared_ptr<MementoSettingTriggerChannel> SettingTriggerChannel::createMemento()
{
    State curState;
    curState.Rise = std::make_pair(pCheckRise_->isChecked(), pSpinBoxRise_->value());
    curState.Fall = std::make_pair(pCheckFall_->isChecked(), pSpinBoxFall_->value());
    curState.Upper = std::make_pair(pCheckUpper_->isChecked(), pSpinBoxUpper_->value());
    curState.Under = std::make_pair(pCheckUnder_->isChecked(), pSpinBoxUnder_->value());
    return std::shared_ptr<MementoSettingTriggerChannel>(new MementoSettingTriggerChannel(curState));
}

void SettingTriggerChannel::setMemento(std::shared_ptr<MementoSettingTriggerChannel> pMemento)
{
    State state = pMemento->getState();
    if(state.Rise.first) {
        pCheckRise_->setChecked(true);
        pSpinBoxRise_->setValue(state.Rise.second);
    } else
        pCheckRise_->setChecked(false);

    if(state.Fall.first) {
        pCheckFall_->setChecked(true);
        pSpinBoxFall_->setValue(state.Fall.second);
    } else
        pCheckFall_->setChecked(false);
    if(state.Upper.first) {
        pCheckUpper_->setChecked(true);
        pSpinBoxUpper_->setValue(state.Upper.second);
    } else
        pCheckUpper_->setChecked(false);
    if(state.Under.first) {
        pCheckUnder_->setChecked(true);
        pSpinBoxUnder_->setValue(state.Under.second);
    } else
        pCheckUnder_->setChecked(false);
}

void SettingTriggerChannel::changeStateTriggers(int state)
{
    if(state == Qt::Checked)
        emit activate();
    else if(state == Qt::Unchecked) {
        if(pCheckRise_->checkState() == Qt::Unchecked && pCheckFall_->checkState() == Qt::Unchecked
                && pCheckUpper_->checkState() == Qt::Unchecked && pCheckUnder_->checkState() == Qt::Unchecked)
            emit deactivate();
    }
}

void SettingTriggerChannel::reset()
{
    pCheckSelectAll_->setChecked(false);
    pCheckRise_->setChecked(false);
    pCheckFall_->setChecked(false);
    pCheckUnder_->setChecked(false);
    pCheckUpper_->setChecked(false);
}

SettingTriggerChannel::SettingTriggerChannel(int idx, QPixmap& pixRiseFront,QPixmap& pixFallFront,
                                             QPixmap& pixUpperLevel, QPixmap& pixUnderLevel, QWidget* parent):
    QWidget(parent), idx_(idx)
{
    pLabelRise_ = new QLabel;
    pLabelRise_->setPixmap(pixRiseFront);
    pCheckRise_ = new QCheckBox("&Rise");
    pSpinBoxRise_ = new QSpinBox;
    pSpinBoxRise_->setRange(std::numeric_limits<int>::lowest(), std::numeric_limits<int>::max());
    pSpinBoxRise_->setEnabled(false);
    connect(pCheckRise_, SIGNAL(toggled(bool)), pSpinBoxRise_, SLOT(setEnabled(bool)));
    connect(pCheckRise_, SIGNAL(stateChanged(int)), this, SLOT(changeStateTriggers(int)));

    pLabelFall_ = new QLabel;
    pLabelFall_->setPixmap(pixFallFront);
    pCheckFall_ = new QCheckBox("&Fall");
    pSpinBoxFall_ = new QSpinBox;
    pSpinBoxFall_->setRange(std::numeric_limits<int>::lowest(), std::numeric_limits<int>::max());
    pSpinBoxFall_->setEnabled(false);
    connect(pCheckFall_, SIGNAL(toggled(bool)), pSpinBoxFall_, SLOT(setEnabled(bool)));
    connect(pCheckFall_, SIGNAL(stateChanged(int)), this, SLOT(changeStateTriggers(int)));

    pLabelUpper_ = new QLabel;
    pLabelUpper_->setPixmap(pixUpperLevel);
    pCheckUpper_ = new QCheckBox("&Upper");
    pSpinBoxUpper_ = new QSpinBox;
    pSpinBoxUpper_->setRange(std::numeric_limits<int>::lowest(), std::numeric_limits<int>::max());
    pSpinBoxUpper_->setEnabled(false);
    connect(pCheckUpper_, SIGNAL(toggled(bool)), pSpinBoxUpper_, SLOT(setEnabled(bool)));
    connect(pCheckUpper_, SIGNAL(stateChanged(int)), this, SLOT(changeStateTriggers(int)));

    pLabelUnder_ = new QLabel;
    pLabelUnder_->setPixmap(pixUnderLevel);
    pCheckUnder_ = new QCheckBox("U&nder");
    pSpinBoxUnder_ = new QSpinBox;
    pSpinBoxUnder_->setRange(std::numeric_limits<int>::lowest(), std::numeric_limits<int>::max());
    pSpinBoxUnder_->setEnabled(false);
    connect(pCheckUnder_, SIGNAL(toggled(bool)), pSpinBoxUnder_, SLOT(setEnabled(bool)));
    connect(pCheckUnder_, SIGNAL(stateChanged(int)), this, SLOT(changeStateTriggers(int)));

    pCheckSelectAll_= new QCheckBox("&Select");
    connect(pCheckSelectAll_, SIGNAL(toggled(bool)), pCheckRise_, SLOT(setChecked(bool)));
    connect(pCheckSelectAll_, SIGNAL(toggled(bool)), pCheckFall_, SLOT(setChecked(bool)));
    connect(pCheckSelectAll_, SIGNAL(toggled(bool)), pCheckUpper_, SLOT(setChecked(bool)));
    connect(pCheckSelectAll_, SIGNAL(toggled(bool)), pCheckUnder_, SLOT(setChecked(bool)));

    QGridLayout *pglEditLocation = new QGridLayout;
    pglEditLocation->addWidget(pLabelRise_, 0, 0);
    pglEditLocation->addWidget(pCheckRise_, 0, 1);
    pglEditLocation->addWidget(pSpinBoxRise_, 0, 2);

    pglEditLocation->addWidget(pLabelFall_, 1, 0);
    pglEditLocation->addWidget(pCheckFall_, 1, 1);
    pglEditLocation->addWidget(pSpinBoxFall_, 1, 2);

    pglEditLocation->addWidget(pLabelUpper_, 2, 0);
    pglEditLocation->addWidget(pCheckUpper_, 2, 1);
    pglEditLocation->addWidget(pSpinBoxUpper_, 2, 2);

    pglEditLocation->addWidget(pLabelUnder_, 3, 0);
    pglEditLocation->addWidget(pCheckUnder_, 3, 1);
    pglEditLocation->addWidget(pSpinBoxUnder_, 3, 2);

    pglEditLocation->addWidget(pCheckSelectAll_, 4, 1);

    setLayout(pglEditLocation);
}


ChannelListWidgetItem::ChannelListWidgetItem(const QString &text, QListWidget *parent): QListWidgetItem(text, parent) {
    deactBrush_ = background();
    actBrush_ = QBrush(QColor(0,255,0));
}

void ChannelListWidgetItem::activate()
{
    setBackground(actBrush_);
}

void ChannelListWidgetItem::deactivate()
{
    setBackground(deactBrush_);
}



qint32 SettingSignalChannel::idx() const
{
    return idx_;
}

ParamSignal SettingSignalChannel::getParameter()
{
    return param_;
}

SettingSignalChannel::SettingSignalChannel(qint32 idx, const ParamSignal &param, QWidget* parent):
    QWidget(parent), idx_(idx), param_(param)
{
    pCheckEnable_ = new QCheckBox("&Enable signal");
    if(param_.enable)
        pCheckEnable_->setChecked(true);
    connect(pCheckEnable_, SIGNAL(toggled(bool)), this, SLOT(changePermitSignal(bool)));

    pButtonSelectColor_ = new QPushButton("&Set color...");

    if(param_.enable)
        pButtonSelectColor_->setEnabled(true);
    else
        pButtonSelectColor_->setEnabled(false);
    connect(pButtonSelectColor_, SIGNAL(clicked()), this, SLOT(handSelectColor()));
    connect(pCheckEnable_, SIGNAL(toggled(bool)), pButtonSelectColor_, SLOT(setEnabled(bool)));
    connect(pCheckEnable_, &QCheckBox::toggled,[this](bool allow) {
        param_.enable = allow;
    });
    pLabelColor_ = new QLabel;
    pLabelColor_->setFixedSize(50,50);
    pLabelColor_->setAutoFillBackground(true);
    QPalette palette;
    palette.setColor(QPalette::Window, param_.color);
    pLabelColor_->setPalette(palette);

    QGridLayout* pglSetting = new QGridLayout;
    pglSetting->addWidget(pCheckEnable_, 0, 0);
    pglSetting->addWidget(pLabelColor_, 2,0);
    pglSetting->addWidget(pButtonSelectColor_, 2,1);
    setLayout(pglSetting);
}

void SettingSignalChannel::handSelectColor()
{
    QColor color = QColorDialog::getColor(param_.color, this);
    if(color.isValid())
        param_.color = color;
    QPalette palette;
    palette.setColor(QPalette::Window, param_.color);
    pLabelColor_->setPalette(palette);
}

void SettingSignalChannel::changePermitSignal(bool status)
{
    if(status)
        emit enableSignal();
    else
        emit disableSignal();
}

void SettingSignalChannel::enable()
{
    param_.enable = true;
    pCheckEnable_->setChecked(true);
}

void SettingSignalChannel::disable()
{
    param_.enable = false;
    pCheckEnable_->setChecked(false);
}


SettingSignal::SettingSignal(const QVector<qint32> &chs, ScreenWidget& screen, QWidget *parent):QDialog(parent)
{
    std::default_random_engine dre(3060);
    std::uniform_int_distribution<unsigned int> di;
    std::map<qint32, ParamSignal> signalsParam;
    foreach (qint32 idx, chs) {
        ParamSignal param{true, QColor(di(dre))};
        signalsParam[idx] = param;
    }
    pStackChannelSignals_ = new QStackedWidget;
    pListChannels_ = new QListWidget;
    pListChannels_->setSelectionMode(QAbstractItemView::ExtendedSelection);
    for(std::pair<qint32, ParamSignal> channel : signalsParam) {
        SettingSignalChannel* pSignalChannel = new SettingSignalChannel(channel.first, channel.second);
        pStackChannelSignals_->addWidget(pSignalChannel);
        ChannelListWidgetItem* pChannelItem = new ChannelListWidgetItem(QString::number(channel.first), pListChannels_);
        pChannelItem->activate();
        connect(pSignalChannel, &SettingSignalChannel::enableSignal, [pChannelItem](){pChannelItem->activate();});
        connect(pSignalChannel, &SettingSignalChannel::disableSignal, [pChannelItem](){pChannelItem->deactivate();});
    }
    connect(pListChannels_, SIGNAL(currentRowChanged(int)), pStackChannelSignals_, SLOT(setCurrentIndex(int)));

    pButtonEnable_ = new QPushButton("&Enable");
    connect(pButtonEnable_, SIGNAL(clicked()), this, SLOT(handEnable()));
    pButtonEnableAll_ = new QPushButton("Enable &all");
    connect(pButtonEnableAll_, SIGNAL(clicked()), this, SLOT(handEnableAll()));
    pButtonDisable_ = new QPushButton("&Disable");
    connect(pButtonDisable_, SIGNAL(clicked()), this, SLOT(handDisable()));
    pButtonDisableAll_ = new QPushButton("Disable a&ll");
    connect(pButtonDisableAll_, SIGNAL(clicked()), this, SLOT(handDisableAll()));

    QGridLayout *pglChannels = new QGridLayout;
    pglChannels->addWidget(pListChannels_,0,0,2,2);
    pglChannels->addWidget(pButtonEnable_, 2, 0);
    pglChannels->addWidget(pButtonEnableAll_, 3, 0);
    pglChannels->addWidget(pButtonDisable_, 2, 1);
    pglChannels->addWidget(pButtonDisableAll_, 3, 1);
    QGroupBox* pGroupChannels = new QGroupBox("Channels");
    pGroupChannels->setLayout(pglChannels);

    pButtonOk_ = new QPushButton("&Ok");
    connect(pButtonOk_, SIGNAL(clicked()), this, SLOT(handOk()));
    pButtonCancel_ = new QPushButton("&Cancel");
    connect(pButtonCancel_, SIGNAL(clicked()), this, SLOT(handCancel()));

    QGridLayout *pglEditLocation = new QGridLayout;
    pglEditLocation->addWidget(pGroupChannels, 0,0, 2, 1);
    pglEditLocation->addWidget(pStackChannelSignals_, 0,1,2,1);
    pglEditLocation->addWidget(pButtonCancel_, 2,0,1,1);
    pglEditLocation->addWidget(pButtonOk_, 2,1,1,1);
    setLayout(pglEditLocation);
    setWindowTitle("Setting signal");
    setFixedSize(minimumSizeHint());

    screen.setParamSignals(signalsParam);
    connect(this, SIGNAL(setParamSignal(std::map<qint32,ParamSignal>)),
            &screen, SLOT(setParamSignals(std::map<qint32,ParamSignal>)));
}

void SettingSignal::handOk()
{
    std::map<qint32, ParamSignal> signalsParam;
    for(int i = 0; i < pStackChannelSignals_->count(); i++) {
        SettingSignalChannel* channel =   static_cast<SettingSignalChannel*>(pStackChannelSignals_->widget(i));
        qint32 id = channel->idx();
        signalsParam[id] = channel->getParameter();
    }
    emit setParamSignal(signalsParam);
    hide();
}

void SettingSignal::handCancel()
{
    hide();
}

void SettingSignal::handDisable()
{
    QList<QListWidgetItem *> listSelectChannel = pListChannels_->selectedItems();
    foreach (QListWidgetItem* item, listSelectChannel) {
        int index = pListChannels_->row(item);
        static_cast<SettingSignalChannel*>(pStackChannelSignals_->widget(index))->disable();
    }
}

void SettingSignal::handDisableAll()
{
    for(int i = 0; i < pStackChannelSignals_->count(); i++)
        static_cast<SettingSignalChannel*>(pStackChannelSignals_->widget(i))->disable();
}

void SettingSignal::handEnable()
{
    QList<QListWidgetItem *> listSelectChannel = pListChannels_->selectedItems();
    foreach (QListWidgetItem* item, listSelectChannel) {
        int index = pListChannels_->row(item);
        static_cast<SettingSignalChannel*>(pStackChannelSignals_->widget(index))->enable();
    }
}

void SettingSignal::handEnableAll()
{
    for(int i = 0; i < pStackChannelSignals_->count(); i++)
        static_cast<SettingSignalChannel*>(pStackChannelSignals_->widget(i))->enable();
}


SettingView::SettingView(ScreenWidget &screen, QWidget *parent): QDialog(parent), pScreen_(&screen)
{
    QHBoxLayout* phlBackround = new QHBoxLayout;
    pLabelBackroundColor_ = new QLabel;
    pLabelBackroundColor_->setFixedSize(10,10);
    pLabelBackroundColor_->setAutoFillBackground(true);
    QPalette palette;
    palette.setColor(QPalette::Window, screen.getBackroundColor());
    pLabelBackroundColor_->setPalette(palette);
    pButtonBackroundColor_ = new QPushButton("&Backround color...");
    pButtonBackroundColor_->setFixedWidth(150);
    connect(pButtonBackroundColor_, SIGNAL(clicked()), this, SLOT(setBackroundColor()));
    phlBackround->addWidget(pLabelBackroundColor_);
    phlBackround->addStretch();
    phlBackround->addWidget(pButtonBackroundColor_);
    QGroupBox* pgrBackround = new QGroupBox("Backround");
    pgrBackround->setLayout(phlBackround);

    QGridLayout* pglGrid = new QGridLayout;
    pLabelGridColor_ = new QLabel;
    pLabelGridColor_->setFixedSize(10,10);
    pLabelGridColor_->setAutoFillBackground(true);
    palette.setColor(QPalette::Window, screen.getGridColor());
    pLabelGridColor_->setPalette(palette);
    pButtonGridColor_ = new QPushButton("&Grid color...");
    pButtonGridColor_->setFixedWidth(150);
    connect(pButtonGridColor_, SIGNAL(clicked()), this, SLOT(setGridColor()));
    pSpinBoxNumberColumn_ = new QSpinBox;
    pSpinBoxNumberColumn_->setRange(1, 100);
    pSpinBoxNumberColumn_->setValue(screen.getNumberColumn());
    QLabel* pLabelNumberColumn = new  QLabel("Number co&lumn");
    pLabelNumberColumn->setBuddy(pSpinBoxNumberColumn_);
    pSpinBoxNumberRow_ = new QSpinBox;
    pSpinBoxNumberRow_->setRange(1, 100);
    pSpinBoxNumberRow_->setValue(screen.getNumberRow());
    QLabel* pLabelNumberRow = new  QLabel("Number &row");
    pLabelNumberRow->setBuddy(pSpinBoxNumberRow_);
    pglGrid->addWidget(pLabelGridColor_, 0, 0);
    pglGrid->addWidget(pButtonGridColor_, 0, 1);
    pglGrid->addWidget(pLabelNumberColumn, 1, 0);
    pglGrid->addWidget(pSpinBoxNumberColumn_, 1, 1);
    pglGrid->addWidget(pLabelNumberRow, 2, 0);
    pglGrid->addWidget(pSpinBoxNumberRow_, 2, 1);
    QGroupBox* pgrGrid = new QGroupBox("Grid");
    pgrGrid->setLayout(pglGrid);

    QGridLayout *pglLegend = new QGridLayout;
    pLabelLegendColor_ = new QLabel;
    pLabelLegendColor_->setFixedSize(10,10);
    pLabelLegendColor_->setAutoFillBackground(true);
    palette.setColor(QPalette::Window, screen.getLegendColor());
    pLabelLegendColor_->setPalette(palette);
    pButtonLegendColor_ = new QPushButton("Legen&d color...");
    pButtonLegendColor_->setFixedWidth(150);
    connect(pButtonLegendColor_, SIGNAL(clicked()), this, SLOT(setLegendColor()));
    pLabelLegendFont_ = new QLabel;
    pLabelLegendFont_->setFixedSize(80,30);
    pLabelLegendFont_->setWordWrap(true);
    pLabelLegendFont_->setFont(screen.getFontLegend());
    pLabelLegendFont_->setText(pLabelLegendFont_->fontInfo().family());
    pButtonLegendFont_ = new QPushButton("Legend &font...");
    pButtonLegendFont_->setFixedWidth(150);
    connect(pButtonLegendFont_, SIGNAL(clicked()), this, SLOT(setLegendFont()));
    pglLegend->addWidget(pLabelLegendColor_, 0, 0);
    pglLegend->addWidget(pButtonLegendColor_, 0, 2);
    pglLegend->addWidget(pLabelLegendFont_, 1, 0);
    pglLegend->addWidget(pButtonLegendFont_, 1, 2);
    QGroupBox* pgrLegend = new QGroupBox("Legend");
    pgrLegend->setLayout(pglLegend);

    QHBoxLayout* phlControl = new QHBoxLayout;
    pButtonOk_ = new QPushButton("&Ok");
    connect(pButtonOk_, SIGNAL(clicked()), this, SLOT(handOk()));
    pButtonCancel_ = new QPushButton("&Cancel");
    connect(pButtonCancel_, SIGNAL(clicked()), this, SLOT(handCancel()));
    pButtonApply_ = new QPushButton("&Apply");
    connect(pButtonApply_, SIGNAL(clicked()), this, SLOT(handApply()));
    phlControl->addWidget(pButtonOk_);
    phlControl->addWidget(pButtonCancel_);
    phlControl->addWidget(pButtonApply_);

    QVBoxLayout* pvLayout = new QVBoxLayout;
    pvLayout->addWidget(pgrBackround);
    pvLayout->addWidget(pgrGrid);
    pvLayout->addWidget(pgrLegend);
    pvLayout->addLayout(phlControl);
    setLayout(pvLayout);
    setWindowTitle("Setting view");
    setFixedSize(minimumSizeHint());
}

void SettingView::handOk()
{
    handApply();
    hide();
}

void SettingView::handCancel()
{
    hide();
}

void SettingView::handApply()
{
    pScreen_->setBackroundColor(pLabelBackroundColor_->palette().color(QPalette::Window));
    pScreen_->setGridColor(pLabelGridColor_->palette().color(QPalette::Window));
    pScreen_->setNumberColumn(pSpinBoxNumberColumn_->value());
    pScreen_->setNumberRow(pSpinBoxNumberRow_->value());
    pScreen_->setLegendColor(pLabelLegendColor_->palette().color(QPalette::Window));
    pScreen_->setFontLegend(pLabelLegendFont_->font());
}

void SettingView::setBackroundColor()
{
    QColor color = QColorDialog::getColor(pLabelBackroundColor_->palette().color(QPalette::Window), this);
    if(color.isValid()) {
        QPalette palette;
        palette.setColor(QPalette::Window, color);
        pLabelBackroundColor_->setPalette(palette);
    }
}

void SettingView::setGridColor()
{
    QColor color = QColorDialog::getColor(pLabelGridColor_->palette().color(QPalette::Window), this);
    if(color.isValid()) {
        QPalette palette;
        palette.setColor(QPalette::Window, color);
        pLabelGridColor_->setPalette(palette);
    }
}

void SettingView::setLegendColor()
{
    QColor color = QColorDialog::getColor(pLabelLegendColor_->palette().color(QPalette::Window), this);
    if(color.isValid()) {
        QPalette palette;
        palette.setColor(QPalette::Window, color);
        pLabelLegendColor_->setPalette(palette);
    }
}

void SettingView::setLegendFont()
{
    bool ok;
    QFont font = QFontDialog::getFont(&ok, pLabelLegendFont_->font() , this);
    if (ok) {
        pLabelLegendFont_->setFont(font);
        pLabelLegendFont_->setText(pLabelLegendFont_->fontInfo().family());
    }
}
