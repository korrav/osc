#ifndef SCREEN_H
#define SCREEN_H

#include <QWidget>
#include <QColor>
#include <QPixmap>
#include "clipboard.h"
#include <boost/optional.hpp>
#include <QRubberBand>
#include <deque>
#include <QLineEdit>
#include <QPushButton>
#include <QDialog>
#include <QPainter>
#include <QPolygonF>
#include <array>

class CacheScales;

struct ParamSignal{
    bool enable;
    QColor color;
};

class Screen : public QWidget
{
    Q_OBJECT
public:
    struct AreaVisualization {
        qint64 timeMin;
        qint64 timeMax;
        qint64 valueMin;
        qint64 valueMax;
        bool operator ==(const AreaVisualization& area) {
            if(timeMin == area.timeMin && timeMax == area.timeMax && valueMin == area.valueMin && valueMax == area.valueMax)
                return true;
            else
                return false;
        }
    };
private:
    struct ParamPainter {   //параметры рисовальщика для канала
        QPen pen;
        int expansion = 1;
        int offset = 0;
    };

    const qint64 DEFAULT_TIME_MIN =  -1000;
    const qint64 DEFAULT_TIME_MAX = 1000;
    const qint64 DEFAULT_VALUE_MIN = -1000;
    const qint64 DEFAULT_VALUE_MAX = 1000;
    const qint64 DEFAULT_ZOOM_FACTOR = 2;
    const unsigned NUMBER_STEP_TIME_GRIND = 5;
    const unsigned NUMBER_STEP_VALUE_GRIND = 5;
    const int MARGIN = 60;  //отступ сетки от края виджета
    const int OFFSET_LABEL = 10;    //отступ метки от сетки
    const int OFFSET_LEGEND = 30;    //отступ легенды от сетки

    QColor colorBackround_ = Qt::black;  //цвет фона экрана
    QColor colorGrind_ = Qt::yellow;
    QColor colorLegend_ = Qt::yellow;
    QFont fontLegend_;
    unsigned numberColumn_ = NUMBER_STEP_TIME_GRIND;
    unsigned numberRow_ = NUMBER_STEP_VALUE_GRIND;
    std::map<qint32, ParamPainter> curvePainters_; //рисовальщики кривых

    QPixmap pixmap_; /*Пиксельная карта, выступающая в качестве промежуточного буфера
        визуализации содержимого окна визуализации*/
    AreaVisualization coordinates_;  //координаты сигнала в виджете
    QRect rectCoordinate_;  //прямоугольник рабочей области отображения графиков
    ClipBoard* clipboard_;  //обработчик входного потока данных
    CacheScales* pCacheScales_;   //кэш масштабов
    double zoomFactor_ = DEFAULT_ZOOM_FACTOR; //коэффициент изменения масштаба
    QRubberBand* pRubberBand_;
    QPoint originRubberBand_;
    bool isEnableRubberBand_ = false;
    bool isEnableGrid_ = true;
    std::pair<qint64, std::vector<PConstUnit>> snippetSignal_;    //Содержит фрагмент сигнала, соответствующего окну визуализации и время его первого отсчёта
    std::pair<QString, qint64> prefixAndFactorTimeAxis_;    //префикс и коэффициент умножения для оси времени виджета
    QPainter painter_;

    void updateSignalSnippet(void); //Обновление фрагмента сигнала, соответствующего окну визуализации
    void drawRectCoordinate(bool& status); //Рисование прямоугольника рабочей области отображения графиков
    void drawAxisAndGrid(void); //Рисование осей и сетки  на пиксельной карте окна визуализации
    void drawTimeAxisAndGrind(void); //Рисование осей и сетки времени  на пиксельной карте окна визуализации
    void drawValueAxisAndGrind(void); //Рисование осей и сетки значения сигнала  на пиксельной карте окна визуализации
    void drawSignals(void);  //Рисование сигналов на пиксельной карте окна визуализации
    boost::optional<std::pair<qint64, qint64>> getCoordinatesSignal(QPoint point);   //получить координаты точки виджета в пространстве (time, value)
    QPointF getPointSignal(qint64 time, qint64 value);  //получить точку на виджете, соответсвующую координате точки в пространстве (time, value)
    std::pair<qint64, qint64> getSpanValueSnippetSignal(void);  //получить диапазон значений текущего фрагмента сигнала
    std::pair<qint64, qint64> getCenterRepresentsArea(void);    //возвращает центр отображаемой области
    std::map<qint32, QPolygonF> getSignalsCurves(void) const; //получить координаты относительно виджета графиков сигналов
public:
    explicit Screen(ClipBoard* pClipboard,CacheScales* cash, QWidget *parent = 0);
    QSize minimumSizeHint() const;
    void rescalingUp();  //увеличение масштаба изображения с центром
    void rescalingDown();  //уменьшение масштаба изображения с центром
    void enableRubberBand(void);    //разрешение резиновой линии изменения масштаба
    void disableRubberBand(void);   //запрещение резиновой линии изменения масштаба
    void undo(void);    //продвижение вперёд по истории масштабов изображения
    void redo(void);    //продвижение назад по истории масштабов изображения
    void enableGrid(bool enable);  //разрешить/запретить визуализацию сетки
    void setPositionVisualizationWindow(const AreaVisualization& coordinates);  //установить новое положение окна визуализации
    void setCurvePainters(const std::map<qint32, ParamSignal> &paramSignals);
    QColor getBackroundColor(void);
    void setBackroundColor(QColor color);
    QColor getGridColor(void);
    void setGridColor(QColor color);
    QColor getLegendColor(void);
    void setLegendColor(QColor color);
    unsigned getNumberColumn(void);
    void setNumberColumn(unsigned column);
    unsigned getNumberRow(void);
    void setNumberRow(unsigned row);
    QFont getFontLegend(void);
    void setFontLegend(QFont font);
protected:
    void resizeEvent(QResizeEvent *);
    void paintEvent(QPaintEvent *);
    void keyPressEvent(QKeyEvent *event);
    void wheelEvent(QWheelEvent *event);
    void mousePressEvent(QMouseEvent*event);
    void mouseMoveEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent*event);
signals:
    void updateSpanTimeSignal(std::pair<qint64, qint64>, std::pair<qint64, qint64>); /*изменение временного промежутка сигнала
        в первом параметре отображается текущая область, во втором полный диапазон временных значений*/
    void updateSpanValueSignal(std::pair<qint64, qint64>, std::pair<qint64, qint64>); /*изменение области значений фрагмента сигнала
        в первом параметре отображается текущая область, во втором полный диапазон значений фрагмента*/
    void position(QString, QString);    //текущая позиция курсора мыши в пространстве время-значение
public slots:
    void refreshPixmap(void); //Обновление изображения виджета
    void shiftInTime(qint64 offset);    //сместить по времени окно обзора
    void shiftInValue(qint64 offset);    //сместить по значению сигнала окно обзора
    void rescaling(qint64 time, qint64 value, double factor);  //изменение масштаба изображения с центром (time, value)
};

class ScreenWidget: public QWidget
{
    Q_OBJECT
public:
    ScreenWidget(ClipBoard* pClipboard, QWidget *parent = 0);
    QColor getBackroundColor(void);
    QColor getGridColor(void);
    QColor getLegendColor(void);
    unsigned getNumberColumn(void);
    unsigned getNumberRow(void);
    QFont getFontLegend(void);
private:
    Screen* pScreen_;
    CacheScales* pCache_;
public slots:
    void zoom(void);    //увеличить масштаб изображения
    void zoomOut(void); //уменьшить масштаб изображения
    void enableRubberBand(bool status); //позволить или запретить резиновую линию изменения масштаба изображения
    void undo(void);
    void redo(void);
    void enableGrid(bool enable);
    void changePosVisualizationWindow(std::array<qint64, 4> area);   //изменить положение окна визуализации
    void setParamSignals(std::map<qint32, ParamSignal> setSignals);
    void setBackroundColor(QColor color);
    void setGridColor(QColor color);
    void setLegendColor(QColor color);
    void setNumberColumn(unsigned column);
    void setNumberRow(unsigned row);
    void setFontLegend(QFont font);
signals:
    void enableUndo(bool);
    void enableRedo(bool);
    void position(QString, QString);
};


class CacheScales : public QObject { //класс, отвечающий за кэширование масштабов изображения (кэширование производится только при изменении масштаба изображения)
    Q_OBJECT
    std::deque<Screen::AreaVisualization> queueScales_;
    unsigned int MAXIMUM_ITEM_CASH = 1000;
    unsigned int positionInCash_ = 0; //позиция в кэше
public:
    boost::optional<Screen::AreaVisualization> prev();  //продвижение назад по кэшу масштабов изображения
    boost::optional<Screen::AreaVisualization> next();  //продвижение вперёд по кэшу масштабов изображения
    void push(const Screen::AreaVisualization& area);   //поместить новый масштаб в кэш
    void initialize(const Screen::AreaVisualization& area); //инициализация кэша. Должна производится перед использованием
signals:
    void enableUndo(bool);
    void enableRedo(bool);
};

class Cursor: public QObject  {
    Q_OBJECT
protected:
    QColor color_;  //цвет курсора
    QRectF rect_;    //прямоугольник, в пределах которого оперирует курсор
    qreal coord_;   //координата, на которую указывает курсор
private:
    virtual void drawCursor(QPainter& painter) = 0;    //нарисовать курсор
public:
    Cursor(const QColor& color, QObject* pobj = 0): QObject(pobj), color_(color) {
    }
    void setColor(const QColor& color){
        color_ = color;
    }
    QPen getPen(void) const;
    virtual QRectF getSpaceUsed(void) const = 0;    //возвратить координаты пространства, занимаемого курсором
    virtual bool is_containts(const QPointF& point) const; //принадлежит ли точка области курсора
    QColor getColor() const;    //возвращает цвет курсора
    virtual QPointF getPositionLegend(void) const = 0; //возвращает левый край базовой линии подписи к курсору
    virtual qreal getAngleLegend(void) const = 0; //возвращает угол написания подписи к курсору
public:
    void draw(QPainter& painter, const QRectF& rect, qreal coord,
                      const QString& mes = "");  //нарисовать курсор в области rect с указанием на координату coord c надписью mes
signals:
    void changeCoordinate(qreal newCoord);
};

class HorizontalCursor: public Cursor {
    Q_OBJECT
    const qreal SIDE_ORIG_RECT = 100;
    QPainterPath arrows_;   //стрелка курсора
    virtual void drawCursor(QPainter& painter);
public:
    HorizontalCursor(const QColor& color, QObject* pobj = 0);
    virtual QPointF getPositionLegend(void) const;
    virtual QRectF getSpaceUsed(void);
};

#endif // SCREEN_H
