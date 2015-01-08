#ifndef OSCILLOSCOPE_H
#define OSCILLOSCOPE_H

#include <QMainWindow>
#include <QLabel>
#include <QToolBar>
#include <QColor>
#include <map>
#include "clipboard.h"
#include "unit.h"
#include "dialogues.h"
#include "screen.h"
using std::map;

class Oscilloscope : public QMainWindow /* класс, представляющий виджет Осциллографа*/
{
    Q_OBJECT
public:
    Oscilloscope(QVector<qint32> chs,QString title, QWidget *parent = 0);
    ~Oscilloscope(){}
    void pass(const shared_ptr<Unit>& pu);  //передача очередного блока данных на обработку
private:
    QVector<qint32> chs_;    //хранит идентификаторы каналов
    QToolBar* pToolControls_;
    QToolBar pToolConfig_;
    QLabel debLab_;
    ClipBoard* clip_;
    ScreenWidget* screen_;
    SetupNewLocationWindowVisualization* pDialogNewLocation_;    //диалоговое окно выбора расположения и размера окна визуализации
    SettingBufferVisualization* pDialogChangeBufferVisualization_; //диалоговое окно установки параметров буфера визуализации
    SettingTrigger* pDialogTrigger_;    //диалоговое окно выбора триггерных условий
    SettingSignal* pDialogSignal_;  //диалоговое окно выбора параметров визуализации сигналов
    SettingView* pDialogView_;  //диалоговое окно изменения параметров изображения экрана осциллографа
    struct ActionsOscilloscope{
        QAction* pPlay;
        QAction* pSinglePlay;
        QAction* pStop;
        QAction* pZoomDown;
        QAction* pZoomUp;
        QAction* pZoom;
        QAction* pUndo;
        QAction* pRedo;
        QAction* pGrid;
        QAction* pLocation;
        QAction* pBufferVisualization;
        QAction* pTriggers;
        QAction* pSignals;
        QAction* pView;
    } action_;
    QLabel* plPosTime_;
    QLabel* plPosValue_;

    void createToolBarControls(ActionsOscilloscope* action);
    void createCentralWidget(void);
    void createActions(ScreenWidget* screen);
    void createMenuBar(ActionsOscilloscope* action);
    void callDialog(QDialog* pDialog);  //вызов диалогового окна
private slots:
    void deb_message(const QString& str);
    void setInfoCursorPosition(QString time, QString value);  //сообщить информацию о текущей позиции курсора
    void callLocationDialog(void);  //вызов диалогового окна определения локализации окна визуализации
    void callBufferVisualizationDialog(void);  //вызов диалогового окна изменения буфера визуализации
    void callSettingTriggerDialog(void);    //вызов диалогового окна установки триггерных условий
    void callSettingSignalDialog(void); //вызов диалогового окна установки параметров визуализации сигналов
    void callSettingViewDialog(void); //вызов диалогового окна изменения параметров изображения экрана осциллографа
};

#endif // OSCILLOSCOPE_H
