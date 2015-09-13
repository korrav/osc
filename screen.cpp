#include "screen.h"
#include <math.h>
#include <exception>
#include <QFontMetrics>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QKeyEvent>
#include <QCursor>
#include <limits>
#include <algorithm>
#include <QLabel>

std::pair<qint64, qint64> getMaxMinValueUnit(PConstUnit& pu) {
    auto compPair = [](const  Unit::Data::value_type& elem1, const Unit::Data::value_type& elem2) {
        return elem1.second < elem2.second;
    };
    if(pu->data.empty())
        throw "Unit data is empty";
    auto position = std::minmax_element(pu->data.begin(), pu->data.end(), compPair);
    qint64 min = position.first->second;
    qint64 max = position.second->second;
    return std::make_pair(min, max);
}

qint64 zeroOutLowerPositions(const qint64 number) {
    if(number <= 0)
        throw "wrong argument number";
    qint64 exponent = static_cast<qint64>(std::log10(number));
    qint64 roundValue = static_cast<qint64>(std::pow(10, exponent));
    //    roundValue = std::round(static_cast<double>(number)/roundValue) * roundValue;
    qint64 interValue = std::round(static_cast<double>(number)/roundValue);
    if(interValue >= std::numeric_limits<qint64>::max()/roundValue)
        roundValue = std::numeric_limits<qint64>::max();
    else
        roundValue = interValue * roundValue;
    return roundValue;
}

std::pair<QString, qint64> getPrefixAndFactorTimeAxis(qint64 stepTime) {
    if(stepTime < 1000)
        return std::pair<QString, qint64>("ns", 1);
    else if(stepTime < 1000000)
        return std::pair<QString, qint64>("us", 1000);
    else if(stepTime < 1000000000)
        return std::pair<QString, qint64>("ms", 1000000);
    else
        return std::pair<QString, qint64>("s", 1000000000);
}

void Screen::updateSignalSnippet()
{
    snippetSignal_ = clipboard_->getSnippetSignal(coordinates_.timeMin, coordinates_.timeMax);
    std::pair<qint64, qint64> spanTimeSignal = clipboard_->getTimeSpanSignal();
    auto fullRangeTime = std::make_pair(std::min(spanTimeSignal.first, coordinates_.timeMin),
                                        std::max(spanTimeSignal.second, coordinates_.timeMax));
    auto currentRangeTime = std::make_pair(coordinates_.timeMin, coordinates_.timeMax);
    emit updateSpanTimeSignal(currentRangeTime, fullRangeTime);
    std::pair<qint64, qint64> spanValueSignal = getSpanValueSnippetSignal();
    auto fullRangeValue = std::make_pair(std::min(spanValueSignal.first, coordinates_.valueMin),
                                         std::max(spanValueSignal.second, coordinates_.valueMax));
    auto currentRangeValue = std::make_pair(coordinates_.valueMin, coordinates_.valueMax);
    emit updateSpanValueSignal(currentRangeValue, fullRangeValue);
}

void Screen::drawRectCoordinate(bool &status)
{
    status = false;
    rectCoordinate_ =  QRect(MARGIN, MARGIN,
                             width() - 2 * MARGIN, height() - 2 * MARGIN);
    if (rectCoordinate_.isValid())
        status = true;
    else
        return;
    QPen pen(colorGrind_);
    pen.setWidth(5);
    painter_.setPen(pen);
    painter_.drawRect(rectCoordinate_);
}

void Screen::drawAxisAndGrid()
{
    //нарисовать прямоугольник рабочей области отображения графиков сигнала
    bool status = true;
    drawRectCoordinate(status);
    if (!status)
        return;
    //нарисовать оси и сетку координат
    drawTimeAxisAndGrind();
    drawValueAxisAndGrind();
}

void Screen::drawTimeAxisAndGrind(void)
{
    //получить величину промежутка времени, соответствующего отображаемой сетки
    qint64 stepGrindTime = 1 + (coordinates_.timeMax - coordinates_.timeMin)/numberColumn_;
    try{
        stepGrindTime = zeroOutLowerPositions(stepGrindTime);
    }
    catch (const char*) {
        return;
    }
    prefixAndFactorTimeAxis_ = getPrefixAndFactorTimeAxis(stepGrindTime);
    QFontMetrics fm(painter_.font());
    qreal heightText = fm.height();
    qreal ratioXToTime = static_cast<qreal>(rectCoordinate_.width())/ (coordinates_.timeMax - coordinates_.timeMin); //соотношение между временными координатами и координатами виджета
    //получить значение первого деления сетки времени
    qint64 indexTimeNotch;
    if(coordinates_.timeMin % stepGrindTime == 0)
        indexTimeNotch = coordinates_.timeMin;
    else if(coordinates_.timeMin > 0)
        indexTimeNotch = (coordinates_.timeMin/stepGrindTime) * stepGrindTime + stepGrindTime;
    else
        indexTimeNotch = (coordinates_.timeMin/stepGrindTime) * stepGrindTime;
    //последовательно визуализировать всю времененную сетку в пределах рабочей области
    for(;indexTimeNotch <= coordinates_.timeMax; indexTimeNotch += stepGrindTime) {
        //получить позицию текущего деления сетки относительно виджета
        qreal positionXNotch = (indexTimeNotch - coordinates_.timeMin) * ratioXToTime + MARGIN;
        //начертить текущее деление временной сетки
        if(isEnableGrid_) {
            painter_.setPen(colorGrind_);
            painter_.drawLine(QPointF(positionXNotch, rectCoordinate_.bottom()),
                              QPointF(positionXNotch, rectCoordinate_.top()));
        }
        //подписать значение текущего деления временной сетки сверху и снизу
        double labelTimeValue = static_cast<double>(indexTimeNotch)/prefixAndFactorTimeAxis_.second;
        QString labelTimeText = QString::number(labelTimeValue, 'g', 5);
        qreal witdhLabelTimeText = fm.width(labelTimeText);
        QRectF rectLabelText(positionXNotch, rectCoordinate_.top() - OFFSET_LABEL - heightText, witdhLabelTimeText, heightText);
        painter_.setPen(colorLegend_);
        painter_.drawText(rectLabelText, labelTimeText, QTextOption(Qt::AlignLeft | Qt::AlignTop));
        rectLabelText.moveTop(rectLabelText.top() + heightText + rectCoordinate_.height() + 2 * OFFSET_LABEL);
        painter_.drawText(rectLabelText, labelTimeText, QTextOption(Qt::AlignLeft | Qt::AlignTop));
    }
    //подписать единицу измерения временной сетки
    QString legendTimeText = QString("time, ") + prefixAndFactorTimeAxis_.first;
    qreal witdhLegendText = fm.width(legendTimeText);
    QRectF rectLegendText(MARGIN + rectCoordinate_.width()/2,
                          rectCoordinate_.bottom() + OFFSET_LEGEND, witdhLegendText, heightText);
    painter_.drawText(rectLegendText, legendTimeText, QTextOption(Qt::AlignLeft | Qt::AlignTop));
}

void Screen::drawValueAxisAndGrind(void)
{
    //получить величину шага отображаемой сетки значений сигнала
    qint64 stepGrindValue = 1 + (coordinates_.valueMax - coordinates_.valueMin)/numberRow_;
    try{
        stepGrindValue = zeroOutLowerPositions(stepGrindValue);
    }
    catch (const char*) {
        return;
    }
    QFontMetrics fm(painter_.font());
    qreal heightText = fm.height();
    qreal ratioYToValue = static_cast<qreal>(rectCoordinate_.height())/ (coordinates_.valueMax - coordinates_.valueMin);
    //получить значение первого деления сетки значений сигнала
    qint64 indexValueNotch;
    if(coordinates_.valueMin % stepGrindValue == 0)
        indexValueNotch = coordinates_.valueMin;
    else if(coordinates_.valueMin > 0)
        indexValueNotch = (coordinates_.valueMin/stepGrindValue) * stepGrindValue + stepGrindValue;
    else
        indexValueNotch = (coordinates_.valueMin/stepGrindValue) * stepGrindValue;
    //последовательно визуализировать всю  сетку значений сигнала в пределах рабочей области
    for(;indexValueNotch <= coordinates_.valueMax; indexValueNotch += stepGrindValue) {
        //получить позицию текущего деления сетки относительно виджета
        qreal positionYNotch = rectCoordinate_.bottom() -
                (indexValueNotch - coordinates_.valueMin) * ratioYToValue;
        //начертить текущее деление сетки значений
        if(isEnableGrid_) {
            painter_.setPen(colorGrind_);
            painter_.drawLine(QPointF(rectCoordinate_.left(), positionYNotch),
                              QPointF(rectCoordinate_.right(), positionYNotch));
        }
        //подписать значение текущего деления сетки значений справа и слева
        QString labelValueText = QString::number(indexValueNotch, 'g', 5);
        qreal witdhLabelValueText = fm.width(labelValueText);
        QRectF rectLabelText(rectCoordinate_.left() - OFFSET_LABEL - witdhLabelValueText,
                             positionYNotch, witdhLabelValueText, heightText);
        painter_.setPen(colorLegend_);
        painter_.drawText(rectLabelText, labelValueText, QTextOption(Qt::AlignLeft | Qt::AlignTop));
        rectLabelText.moveRight(rectLabelText.right() + rectCoordinate_.width() + 2 * OFFSET_LABEL + witdhLabelValueText);
        painter_.drawText(rectLabelText, labelValueText, QTextOption(Qt::AlignLeft | Qt::AlignTop));
    }
}

void Screen::drawSignals(void)
{
    auto curves = getSignalsCurves();
    if(curves.empty())
        return;
    painter_.setClipRect(rectCoordinate_.adjusted(+1, +1, -1, -1));
    for(auto& params : curvePainters_) {
        qint32 idx = params.first;
        if(curves.count(idx) != 0) {
            painter_.setPen(params.second.pen);
            painter_.drawPolyline(curves[idx]);
        }
    }
}

boost::optional<std::pair<qint64, qint64> > Screen::getCoordinatesSignal(QPoint point)
{
    qint64 curTimeCoordinate = 0, curValueCoordinate = 0;
    if(rectCoordinate_.contains(point)) {
        qreal ratioTimeToX = static_cast<qreal>((coordinates_.timeMax - coordinates_.timeMin))/rectCoordinate_.width();
        curTimeCoordinate = coordinates_.timeMin + (point.x() - rectCoordinate_.left()) * ratioTimeToX;
        qreal ratioValueToY = static_cast<qreal>((coordinates_.valueMax - coordinates_.valueMin))/rectCoordinate_.height();
        curValueCoordinate = coordinates_.valueMin + (rectCoordinate_.bottom() - point.y()) * ratioValueToY;
        return std::make_pair(curTimeCoordinate, curValueCoordinate);
    } else
        return boost::none;
}

QPointF Screen::getPointSignal(qint64 time, qint64 value)
{
    qreal ratioXToTime = static_cast<qreal>(rectCoordinate_.width())/ (coordinates_.timeMax - coordinates_.timeMin);
    qreal x = (time - coordinates_.timeMin) * ratioXToTime + MARGIN;
    qreal ratioYToValue = static_cast<qreal>(rectCoordinate_.height())/ (coordinates_.valueMax - coordinates_.valueMin);
    qreal y = rectCoordinate_.bottom() - (value - coordinates_.valueMin) * ratioYToValue;
    return QPointF(x, y);
}

std::pair<qint64, qint64> Screen::getSpanValueSnippetSignal()
{
    if(snippetSignal_.second.empty())
        return std::make_pair(0,0);
    qint64 min,max;
    std::make_pair(std::ref(min), std::ref(max)) = getMaxMinValueUnit(*snippetSignal_.second.begin());
    if(snippetSignal_.second.size() > 1) {
        for(auto& pu : snippetSignal_.second) {
            auto curMinMax = getMaxMinValueUnit(pu);
            min = std::min(min, curMinMax.first);
            max = std::max(max, curMinMax.second);
        }
    }
    return std::make_pair(min, max);
}

std::pair<qint64, qint64> Screen::getCenterRepresentsArea()
{
    qint64 curTimeCoordinate = (coordinates_.timeMax - coordinates_.timeMin)/2 + coordinates_.timeMin;
    qint64 curValueCoordinate = (coordinates_.valueMax - coordinates_.valueMin)/2 + coordinates_.valueMin;
    return std::make_pair(curTimeCoordinate, curValueCoordinate);
}

std::map<qint32, QPolygonF> Screen::getSignalsCurves() const
{
    std::map<qint32, QPolygonF> curves;
    qint64 time = snippetSignal_.first;
    qreal ratioXToTime = static_cast<qreal>(rectCoordinate_.width())/
            (coordinates_.timeMax - coordinates_.timeMin);
    qreal ratioYToValue = static_cast<qreal>(rectCoordinate_.height())/
            (coordinates_.valueMax - coordinates_.valueMin);
    if(snippetSignal_.second.empty())
        return curves;
    for(auto pUnit : snippetSignal_.second) {
        qreal x = (time - coordinates_.timeMin) * ratioXToTime + MARGIN;
        for(auto channelSig : pUnit->data) {
            qint32 channel = channelSig.first;
            qint64 value = channelSig.second;
            qreal y = rectCoordinate_.bottom() - (value - coordinates_.valueMin) * ratioYToValue;
            curves[channel] << QPointF(x, y);
        }
        time += pUnit->dtime;
    }
    return curves;
}

void Screen::shiftInTime(qint64 offset)
{
    if( offset > 0 &&  coordinates_.timeMax >= std::numeric_limits<qint64>::max() - offset)
        offset = 0;
    if(offset < 0 && coordinates_.timeMin <= std::numeric_limits<qint64>::min() - offset)
        offset = 0;
    coordinates_.timeMin += offset;
    coordinates_.timeMax += offset;
    refreshPixmap();
}

void Screen::shiftInValue(qint64 offset)
{
    if( offset > 0 &&  coordinates_.valueMax >= std::numeric_limits<qint64>::max() - offset)
        offset = 0;
    if(offset < 0 && coordinates_.valueMin <= std::numeric_limits<qint64>::min() - offset)
        offset = 0;
    coordinates_.valueMin += offset;
    coordinates_.valueMax+= offset;
    refreshPixmap();
}

void Screen::rescaling(qint64 time, qint64 value, double factor)
{
    if((coordinates_.timeMax - coordinates_.timeMin) >= std::numeric_limits<qint64>::max()/factor)
        factor = 1;
    if((coordinates_.valueMax - coordinates_.valueMin) >= std::numeric_limits<qint64>::max()/factor)
        factor =1;
    qint64 rangeTime = (coordinates_.timeMax - coordinates_.timeMin) * factor;
    if(rangeTime < 2)
        rangeTime = 2;
    qint64 rangeValue = (coordinates_.valueMax - coordinates_.valueMin) * factor;
    if(rangeValue < 2)
        rangeValue = 2;
    coordinates_.timeMin = time - rangeTime/2;
    coordinates_.timeMax = time + rangeTime/2;
    coordinates_.valueMin = value - rangeValue/2;
    coordinates_.valueMax = value + rangeValue/2;
    if(factor != 1)
        pCacheScales_->push(coordinates_);
    refreshPixmap();
}

void Screen::rescalingDown()
{
    auto coordinatesCenter = getCenterRepresentsArea();
    rescaling(coordinatesCenter.first, coordinatesCenter.second, zoomFactor_);
}

void Screen::rescalingUp()
{
    auto coordinatesCenter = getCenterRepresentsArea();
    rescaling(coordinatesCenter.first, coordinatesCenter.second, 1/zoomFactor_);
}

void Screen::enableRubberBand()
{
    isEnableRubberBand_ = true;
}

void Screen::disableRubberBand()
{
    isEnableRubberBand_ = false;
}

void Screen::undo()
{
    auto area = pCacheScales_->next();
    if(area) {
        coordinates_ = area.get();
        refreshPixmap();
    }
}

void Screen::redo()
{
    auto area = pCacheScales_->prev();
    if(area) {
        coordinates_ = area.get();
        refreshPixmap();
    }
}

void Screen::enableGrid(bool enable)
{
    isEnableGrid_ =  enable ? true : false;
    refreshPixmap();
}

void Screen::setPositionVisualizationWindow(const Screen::AreaVisualization &coordinates)
{
    if(coordinates_ == coordinates)
        return;
    else
        coordinates_ = coordinates;
    pCacheScales_->push(coordinates_);
    refreshPixmap();
}

void Screen::setCurvePainters(const std::map<qint32, ParamSignal> &paramSignals)
{
    curvePainters_.clear();
    for(auto& param : paramSignals) {
        if(param.second.enable)
            curvePainters_[param.first].pen = QPen(param.second.color/*, 2*/);
    }
}

QColor Screen::getBackroundColor()
{
    return colorBackround_;
}

void Screen::setBackroundColor(QColor color)
{
    if(colorBackround_ != color) {
        colorBackround_ = color;
        setPalette(QPalette(colorBackround_));
        refreshPixmap();
    }
}

QColor Screen::getGridColor()
{
    return colorGrind_;
}

void Screen::setGridColor(QColor color)
{
    if(colorGrind_ != color) {
        colorGrind_ = color;
        refreshPixmap();
    }
}

QColor Screen::getLegendColor()
{
    return colorLegend_;
}

void Screen::setLegendColor(QColor color)
{
    if(colorLegend_ != color) {
        colorLegend_ = color;
        refreshPixmap();
    }
}

unsigned Screen::getNumberColumn()
{
    return numberColumn_;
}

void Screen::setNumberColumn(unsigned column)
{
    if(numberColumn_ != column) {
        numberColumn_ = column;
        refreshPixmap();
    }
}

unsigned Screen::getNumberRow()
{
    return numberRow_;
}

void Screen::setNumberRow(unsigned row)
{
    if(numberRow_ != row) {
        numberRow_ = row;
        refreshPixmap();
    }
}

QFont Screen::getFontLegend()
{
    return fontLegend_;
}

void Screen::setFontLegend(QFont font)
{
    if(fontLegend_ != font) {
        fontLegend_ = font;
        refreshPixmap();
    }
}

Screen::Screen(ClipBoard *pClipboard, CacheScales* cash, QWidget *parent) : QWidget(parent), fontLegend_("Times",10),
    pixmap_(100, 100), coordinates_{DEFAULT_TIME_MIN, DEFAULT_TIME_MAX,DEFAULT_VALUE_MIN, DEFAULT_VALUE_MAX},
    clipboard_(pClipboard), pCacheScales_(cash)
{
    pCacheScales_->initialize(coordinates_);
    pRubberBand_ = new QRubberBand(QRubberBand::Rectangle, this);
    setPalette(QPalette(colorBackround_));
    setAutoFillBackground(true);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    setFocusPolicy(Qt::StrongFocus);
    setMouseTracking(true);
    connect(clipboard_, SIGNAL(updateVQ()), this,SLOT(refreshPixmap()));
}

QSize Screen::minimumSizeHint() const
{
    return QSize(6 * MARGIN, 4 * MARGIN);
}

void Screen::resizeEvent(QResizeEvent *)
{
    refreshPixmap();
}

void Screen::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    painter.drawPixmap(0, 0, pixmap_);
}

void Screen::keyPressEvent(QKeyEvent *event)
{
    switch (event->key()) {
    case Qt::Key_Right:
        if(event->modifiers() & Qt::ControlModifier)
            shiftInTime(coordinates_.timeMax - coordinates_.timeMin);
        else
            shiftInTime((coordinates_.timeMax - coordinates_.timeMin) / 10 + 1);
        break;
    case Qt::Key_Left:
        if(event->modifiers() & Qt::ControlModifier)
            shiftInTime(-(coordinates_.timeMax - coordinates_.timeMin));
        else
            shiftInTime(-((coordinates_.timeMax - coordinates_.timeMin) / 10 + 1));
        break;
    case Qt::Key_Up :
        if(event->modifiers() & Qt::ControlModifier)
            shiftInValue(coordinates_.valueMax - coordinates_.valueMin);
        else
            shiftInValue((coordinates_.valueMax - coordinates_.valueMin) / 10 + 1);
        break;
    case Qt::Key_Down:
        if(event->modifiers() & Qt::ControlModifier)
            shiftInValue(-(coordinates_.valueMax - coordinates_.valueMin));
        else
            shiftInValue(-((coordinates_.valueMax - coordinates_.valueMin) / 10 + 1));
        break;
    case Qt::Key_Plus:
    case Qt::Key_Minus:
    {
        qint64 curTimeCoordinate = 0;
        qint64 curValueCoordinate = 0;
        QPoint positionMouse = mapFromGlobal(QCursor::pos());
        auto position = getCoordinatesSignal(positionMouse);
        if(position) {
            curTimeCoordinate = position->first;
            curValueCoordinate = position->second;
        } else {
            make_pair(std::ref(curTimeCoordinate), std::ref(curValueCoordinate)) = getCenterRepresentsArea();
        }
        double factor = event->key() == Qt::Key_Plus ? zoomFactor_ : 1/zoomFactor_;
        rescaling(curTimeCoordinate, curValueCoordinate, factor);
    }
        break;
    default:
        QWidget::keyPressEvent(event);
    }
}

void Screen::wheelEvent(QWheelEvent *event)
{
    int numDegrees = event->delta() / 8;
    int numTicks = numDegrees / 15;
    if(event->orientation() == Qt::Vertical && event->modifiers() == Qt::ControlModifier) {
        qint64 step = (coordinates_.timeMax - coordinates_.timeMin) / 10 + 1;
        shiftInValue(step * numTicks);
    }
    if(event->orientation() == Qt::Horizontal || (event->orientation() == Qt::Vertical && event->modifiers() == Qt::ShiftModifier)) {
        qint64 step = (coordinates_.timeMax - coordinates_.timeMin) / 10 + 1;
        shiftInTime(step * numTicks);
    }else if(event->orientation() == Qt::Vertical && event->modifiers() == Qt::NoModifier) {
        double factor;
        if(numTicks < 0)
            factor = -numTicks * zoomFactor_;
        else
            factor = 1 / (numTicks * zoomFactor_);
        if (event->orientation() == Qt::Vertical) {
            auto position = getCoordinatesSignal(event->pos());
            if(position)
                rescaling(position->first, position->second, factor);
        }
    }else
        QWidget::wheelEvent(event);
}

void Screen::mousePressEvent(QMouseEvent* event)
{
    if(isEnableRubberBand_) {
        originRubberBand_ = event->pos();
        if(!rectCoordinate_.contains(originRubberBand_))
            return;
        pRubberBand_->setGeometry(QRect(originRubberBand_, QSize()));
        pRubberBand_->show();
    }
}

void Screen::mouseMoveEvent(QMouseEvent *event)
{
    if(isEnableRubberBand_)
        pRubberBand_->setGeometry(QRect(originRubberBand_, event->pos()).normalized());
    auto positionStatus =  getCoordinatesSignal(event->pos());
    if(positionStatus) {
        auto pos = positionStatus.get();
        QString textTime = QString::number(static_cast<double>(pos.first) / prefixAndFactorTimeAxis_.second)
                + prefixAndFactorTimeAxis_.first;
        QString textValue = QString::number(pos.second);
        emit position(textTime,textValue);
    }
}

void Screen::mouseReleaseEvent(QMouseEvent* event)
{
    pRubberBand_->hide();
    if(isEnableRubberBand_) {
        QPoint delta = event->pos() - originRubberBand_;
        if(std::abs(delta.x()) < 4 || std::abs(delta.y()) < 4)
            return;
        QRect selectArea = (rectCoordinate_.intersected(QRect(originRubberBand_, event->pos()))).normalized();
        if(selectArea.isValid()) {
            auto positionTopLeft = getCoordinatesSignal(selectArea.topLeft());
            auto positionBottomRight = getCoordinatesSignal(selectArea.bottomRight());
            if(positionTopLeft && positionBottomRight) {
                coordinates_.timeMin = positionTopLeft->first;
                coordinates_.valueMax = positionTopLeft->second;
                coordinates_.timeMax = positionBottomRight->first;
                coordinates_.valueMin = positionBottomRight->second;
                pCacheScales_->push(coordinates_);
                refreshPixmap();
            }
        }
    }
}

void Screen::refreshPixmap()
{
    if(coordinates_.timeMax <= coordinates_.timeMin || coordinates_.valueMax <= coordinates_.valueMin)
        return;
    pixmap_ = QPixmap(size());
    pixmap_.fill(colorBackround_);
    painter_.begin(&pixmap_);
    painter_.setFont(fontLegend_);
    updateSignalSnippet();
    drawAxisAndGrid();
    drawSignals();
    painter_.end();
    update();
}


ScreenWidget::ScreenWidget(ClipBoard *pClipboard,
                           QWidget *parent): QWidget(parent)
{
    pCache_ = new CacheScales;
    connect(pCache_, SIGNAL(enableRedo(bool)), this, SIGNAL(enableRedo(bool)));
    connect(pCache_, SIGNAL(enableUndo(bool)), this, SIGNAL(enableUndo(bool)));
    pScreen_ = new Screen(pClipboard, pCache_, this);
    connect(pScreen_, SIGNAL(position(QString,QString)), this, SIGNAL(position(QString,QString)));
    QVBoxLayout* pVLayout = new QVBoxLayout;
    pVLayout->addWidget(pScreen_);
    QHBoxLayout* pHLayout = new QHBoxLayout;
    pHLayout->addLayout(pVLayout);
    setLayout(pHLayout);
}

QColor ScreenWidget::getBackroundColor()
{
    return pScreen_->getBackroundColor();
}

QColor ScreenWidget::getGridColor()
{
    return pScreen_->getGridColor();
}

QColor ScreenWidget::getLegendColor()
{
    return pScreen_->getLegendColor();
}

unsigned ScreenWidget::getNumberColumn()
{
    return pScreen_->getNumberColumn();
}

unsigned ScreenWidget::getNumberRow()
{
    return pScreen_->getNumberRow();
}

QFont ScreenWidget::getFontLegend()
{
    return pScreen_->getFontLegend();
}

void ScreenWidget::changePosVisualizationWindow(std::array<qint64,4> area)
{
    Screen::AreaVisualization location{area[0], area[1], area[2], area[3]};
    pScreen_->setPositionVisualizationWindow(location);
}

void ScreenWidget::setParamSignals(std::map<qint32, ParamSignal> setSignals)
{
    pScreen_->setCurvePainters(setSignals);
}

void ScreenWidget::setBackroundColor(QColor color)
{
    pScreen_->setBackroundColor(color);
}

void ScreenWidget::setGridColor(QColor color)
{
    pScreen_->setGridColor(color);
}

void ScreenWidget::setLegendColor(QColor color)
{
    pScreen_->setLegendColor(color);
}

void ScreenWidget::setNumberColumn(unsigned column)
{
    pScreen_->setNumberColumn(column);
}

void ScreenWidget::setNumberRow(unsigned row)
{
    pScreen_->setNumberRow(row);
}

void ScreenWidget::setFontLegend(QFont font)
{
    pScreen_->setFontLegend(font);
}

void ScreenWidget::zoom()
{
    pScreen_->rescalingUp();
}

void ScreenWidget::zoomOut()
{
    pScreen_->rescalingDown();
}

void ScreenWidget::enableRubberBand(bool status)
{
    if(status)
        pScreen_->enableRubberBand();
    else
        pScreen_->disableRubberBand();
}

void ScreenWidget::undo()
{
    pScreen_->undo();
}

void ScreenWidget::redo()
{
    pScreen_->redo();
}

void ScreenWidget::enableGrid(bool enable)
{
    pScreen_->enableGrid(enable);
}

boost::optional<Screen::AreaVisualization> CacheScales::prev()
{
    if(queueScales_.empty() || positionInCash_ == 0) {
        emit enableRedo(false);
        return boost::none;
    }
    emit enableUndo(true);
    if(--positionInCash_ == 0)
        emit enableRedo(false);
    return queueScales_[positionInCash_];
}

boost::optional<Screen::AreaVisualization> CacheScales::next()
{
    if(queueScales_.empty() || positionInCash_ >= queueScales_.size() - 1) {
        emit enableUndo(false);
        return boost::none;
    }
    emit enableRedo(true);
    if(++positionInCash_ >= queueScales_.size() - 1)
        emit enableUndo(false);
    return queueScales_[positionInCash_];
}

void CacheScales::push(const Screen::AreaVisualization &area)
{   if(positionInCash_ != 0) {
        queueScales_.erase(queueScales_.begin(), queueScales_.begin() + positionInCash_);
    }
    positionInCash_ = 0;
    queueScales_.push_front(area);
    if(queueScales_.size() > MAXIMUM_ITEM_CASH)
        queueScales_.pop_front();
    emit enableRedo(false);
    enableUndo(true);
}

void CacheScales::initialize(const Screen::AreaVisualization &area)
{
    queueScales_.clear();
    queueScales_.push_back(area);
}



QColor Cursor::getColor() const
{
    return color_;
}
QPen Cursor::getPen() const
{
    QPen pen(getColor());
    pen.setWidth(5);
    return pen;
}

void Cursor::draw(QPainter &painter, const QRectF &rect, qreal coord, const QString &mes)
{
    bool coordIsChange = false;
    rect_ = rect;
    if(coord_ != coord) {
        coord_ = coord;
        coordIsChange = true;
    }
    //изобразить курсор
    painter.save();
    painter.setPen(getPen());
    drawCursor(painter);
    //изобразить надпись к курсору
    painter.rotate(getAngleLegend());
    painter.drawText(getPositionLegend(), mes);
    painter.restore();
    if(coordIsChange)
        emit changeCoordinate(coord_);
}

bool Cursor::is_containts(const QPointF &point) const
{
    return getSpaceUsed().contains(point);
}

void HorizontalCursor::drawCursor(QPainter &painter)
{
    if(!rect_.contains(rect_.left(),coord_))
        return;
    painter.translate(rect_.left(), coord_);
    painter.scale(rect_.width()/SIDE_ORIG_RECT, rect_.height()/SIDE_ORIG_RECT);
    painter.drawPath(arrows_);
}

HorizontalCursor::HorizontalCursor(const QColor &color, QObject *pobj): Cursor(color, pobj)
{
    //рисование стрелки
    arrows_.moveTo(0, 0);
    arrows_.lineTo(5, -5);
    arrows_.lineTo(5, 5);
    arrows_.lineTo(0,0);
    arrows_.moveTo(5,0);
    arrows_.lineTo(SIDE_ORIG_RECT - 5, 0);
    arrows_.moveTo(SIDE_ORIG_RECT, 0);
    arrows_.lineTo(SIDE_ORIG_RECT - 5,5);
    arrows_.lineTo(SIDE_ORIG_RECT - 5, -5);
    arrows_.lineTo(SIDE_ORIG_RECT,0);
}

QPointF HorizontalCursor::getPositionLegend() const
{
    return QPointF(5, 0);
}

QRectF HorizontalCursor::getSpaceUsed()
{
    return QRect(rect_.left(), coord_ - 5 *(rect_.height()/SIDE_ORIG_RECT), rect_.width(),
                 10 * (rect_.height()/SIDE_ORIG_RECT));
}
