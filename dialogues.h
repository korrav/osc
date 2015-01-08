#ifndef DIALOGUES_H
#define DIALOGUES_H
#include <QLineEdit>
#include <QPushButton>
#include <QDialog>
#include <QValidator>
#include <limits>
#include <QComboBox>
#include <QCheckBox>
#include <QSpinBox>
#include <QLabel>
#include <QStackedWidget>
#include <QListWidget>
#include <QListWidgetItem>
#include <array>
#include <map>
#include <memory>
#include "screen.h"

class QInt64Validator: public QValidator {
    qint64 min_;
    qint64 max_;
public:
    QInt64Validator(QObject* parent, qint64 minimum = std::numeric_limits<qint64>::min(), qint64 maximum = std::numeric_limits<qint64>::max());
    void setTop(qint64 value);
    void setBottom(qint64 value);
    void setRange(qint64 bottom, qint64 top);
    qint64 top(void) const;
    qint64 bottom(void) const;
    virtual QValidator::State validate(QString& input, int&) const;
    virtual void fixup(QString &input) const;
};

class ChannelListWidgetItem: public QListWidgetItem {
    QBrush deactBrush_;
    QBrush actBrush_;
public:
    ChannelListWidgetItem(const QString & text, QListWidget * parent = 0);
    void activate(void);
    void deactivate(void);
};

class SetupNewLocationWindowVisualization : public QDialog {    //диалоговое окно установки положения и размера окна визуализации
    Q_OBJECT
    QDoubleSpinBox* pSpinBoxMinTime_;
    QDoubleSpinBox* pSpinBoxMaxTime_;
    QSpinBox* pSpinBoxMinValue_;
    QSpinBox* pSpinBoxMaxValue_;
    QPushButton* pButtonOk_;
    QPushButton* pButtonApply_;
    QPushButton* pButtonCancel_;
    QComboBox* pComboMinTime_;
    QComboBox* pComboMaxTime_;
    const QString TEXT_NANOSECOND = "ns";
    const QString TEXT_MICROSECOND = "us";
    const QString TEXT_MILISECOND = "ms";
    const QString TEXT_SECOND = "s";
    const QStringList TEXT_UNITS;

    qint64 getFactor(QString str);
    QStringList getListUnitMinTime(qint64 factorUpperBound);
    QStringList getListUnitMaxTime(qint64 factorUnderBound);
    bool readInputs(std::array<qint64,4>& location);
public:
    SetupNewLocationWindowVisualization(const ScreenWidget &screen,QWidget *parent = 0);
signals:
    void changeLocation(std::array<qint64, 4> location);   //передаёт новые координаты окна визуализации
private slots:
    void getOk(void);
    void getCancel(void);
    void getApply(void);
};

class SettingBufferVisualization : public QDialog {    //диалоговое окно установки параметров буфера визуализации
    Q_OBJECT
    QSpinBox *pSpinBoxSize_;
    QSpinBox *pSpinBoxFocus_;
    QPushButton* pButtonOk_;
    QPushButton* pButtonCancel_;
public:
    SettingBufferVisualization(const ClipBoard& clip, QWidget *parent);
private slots:
    void getOk(void);
    void getCancel(void);
signals:
    void changeBufferVisualization(int size, int focus);
};

class MementoSettingTriggerChannel;

class SettingTriggerChannel : public QWidget {
    Q_OBJECT
private:
    QLabel *pLabelRise_;
    QCheckBox *pCheckRise_;
    QSpinBox *pSpinBoxRise_;

    QLabel *pLabelFall_;
    QCheckBox *pCheckFall_;
    QSpinBox *pSpinBoxFall_;

    QLabel *pLabelUpper_;
    QCheckBox *pCheckUpper_;
    QSpinBox *pSpinBoxUpper_;

    QLabel *pLabelUnder_;
    QCheckBox *pCheckUnder_;
    QSpinBox *pSpinBoxUnder_;

    QCheckBox *pCheckSelectAll_;

    qint32 idx_;   //индекс канала
public:
    struct State {
        std::pair<bool, int> Rise;
        std::pair<bool, int> Fall;
        std::pair<bool, int> Upper;
        std::pair<bool, int> Under;
    };
    enum type_trigger {RISE, FALL, UPPER, UNDER};
    typedef std::map<type_trigger, qint64> TriggerSet;
    SettingTriggerChannel(int idx, QPixmap& pixRiseFront,QPixmap& pixFallFront, QPixmap& pixUpperLevel,
                          QPixmap& pixUnderLevel, QWidget* parent = 0);
    qint32 idx() const;
    TriggerSet getTriggers(void);  //возвращает список активированных триггеров
    std::shared_ptr<MementoSettingTriggerChannel> createMemento(void);  //создать хранитель состояния
    void setMemento(std::shared_ptr<MementoSettingTriggerChannel> pMemento);    //установить новое сотояние
public slots:
    void changeStateTriggers(int state);    //реакция на активирование/деактивирования триггера в канале
    void reset(void);   //сбросить все триггерные условия канала
signals:
    void activate(void);    //сообщение об активации триггера в канале
    void deactivate(void);  //сообщение об деактивации всех триггеров в канале
};

class MementoSettingTriggerChannel {    //хранитель состояния SettingTriggerChannel
    friend class SettingTriggerChannel;
    SettingTriggerChannel::State& state_;

    MementoSettingTriggerChannel(SettingTriggerChannel::State& state): state_(state) {}
    SettingTriggerChannel::State getState(void) {
        return state_;
    }
};

class SettingTrigger : public QDialog {    //диалоговое окно установки триггерных условий
    Q_OBJECT
    QStackedWidget* pStackChannelTriggers_;
    QListWidget* pListChannels_;
    QPushButton* pButtonReset_;
    QPushButton* pButtonResetAll_;
    QPushButton* pButtonOk_;
    QPushButton* pButtonCancel_;
    QVector<qint32> chs_;
    ClipBoard* clip_;
public:
    typedef std::map<qint32, SettingTriggerChannel::TriggerSet> Triggers;
    SettingTrigger(const QVector<qint32>& chs, ClipBoard *clip, QWidget* parent);
public slots:
    void handOk(void);
    void handCancel(void);
    void handReset(void);
    void handResetAll(void);
signals:
    void reset(void);
};

class SettingSignalChannel : public QWidget{
    Q_OBJECT
    QCheckBox* pCheckEnable_;
    QLabel* pLabelColor_;
    QPushButton* pButtonSelectColor_;
    qint32 idx_;
    ParamSignal param_;
public:
    SettingSignalChannel(qint32 idx, const ParamSignal& param, QWidget *parent = 0);
    void enable(void);  //разрешить сигнал
    void disable(void); //запретить сигнал
    qint32 idx() const;
    ParamSignal getParameter(void);
private slots:
    void handSelectColor(void);
    void changePermitSignal(bool status);   //реагирование на разрешение/запрещения сигнала

signals:
    void changeColorSignal(QColor newColor, qint32 index);
    void enableSignal(void);  //разрешение сигнала
    void disableSignal(void); //запрещение сигнала
};

class SettingSignal : public QDialog {    //диалоговое окно установки параметров отображения сигналов
    Q_OBJECT
    QStackedWidget* pStackChannelSignals_;
    QListWidget* pListChannels_;
    QPushButton* pButtonDisable_;
    QPushButton* pButtonDisableAll_;
    QPushButton* pButtonEnable_;
    QPushButton* pButtonEnableAll_;
    QPushButton* pButtonOk_;
    QPushButton* pButtonCancel_;
public:
    SettingSignal(const QVector<qint32>& chs, ScreenWidget &screen, QWidget* parent);
public slots:
    void handOk(void);
    void handCancel(void);
    void handDisable(void);
    void handDisableAll(void);
    void handEnable(void);
    void handEnableAll(void);
signals:
    void disable(void);
    void setParamSignal(std::map<qint32, ParamSignal>);   //уведомление о новых параметрах отображения сигнала
};

class SettingView : public QDialog {    //диалоговое окно настройки изображения экрана осциллографа
    Q_OBJECT
    QPushButton* pButtonBackroundColor_;
    QLabel* pLabelBackroundColor_;
    QPushButton* pButtonGridColor_;
    QLabel* pLabelGridColor_;
    QSpinBox* pSpinBoxNumberColumn_;
    QSpinBox* pSpinBoxNumberRow_;
    QPushButton* pButtonLegendColor_;
    QLabel* pLabelLegendColor_;
    QPushButton* pButtonLegendFont_;
    QLabel* pLabelLegendFont_;
    QPushButton* pButtonOk_;
    QPushButton* pButtonCancel_;
    QPushButton* pButtonApply_;
    ScreenWidget* pScreen_;
public:
    SettingView(ScreenWidget &screen, QWidget* parent);
public slots:
    void handOk(void);
    void handCancel(void);
    void handApply(void);
    void setBackroundColor(void);
    void setGridColor(void);
    void setLegendColor(void);
    void setLegendFont(void);
};

#endif // DIALOGUES_H
