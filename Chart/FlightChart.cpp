#include "flightchart.h"
#include <QVBoxLayout>
#include <QPen>
#include <QBrush>
#include "Fields_parametrs.h"
#include "Fields_lookup.h"
#include <QTimer>
#include <algorithm>
FlightChart::FlightChart(QWidget *parent)
    : QWidget{parent}
{
    QVBoxLayout *layout = new QVBoxLayout(this);
    customPlot = new QCustomPlot(this);
    layout->addWidget(customPlot);
    setLayout(layout);

    // Оптимізації рендера
    customPlot->setPlottingHints(QCP::phFastPolylines | QCP::phCacheLabels);
    customPlot->setNotAntialiasedElements(QCP::aePlottables | QCP::aeScatters | QCP::aeFills);    
    customPlot->setNoAntialiasingOnDrag(true);
    customPlot->xAxis->grid()->setSubGridVisible(false);
    customPlot->yAxis->grid()->setSubGridVisible(false);
    // customPlot->xAxis->grid()->setVisible(false);
    // customPlot->yAxis->grid()->setVisible(false);


    // [OPTIM] Прибрати хіту-тести по легенді (менше навантаження)
    // customPlot->legend->setSelectableParts(QCPLegend::spNone);
    // [OPTIM] Зробити легенду прозорою (менше малювання фону)
    customPlot->legend->setBrush(Qt::NoBrush);

    customPlot->legend->setVisible(true);
    customPlot->legend->setBrush(QBrush(Qt::white));
    customPlot->legend->setBorderPen(QPen(Qt::black));

    darkTheme();

    customPlot->setInteractions(
        QCP::iRangeDrag |
        QCP::iRangeZoom |
        QCP::iSelectAxes |
        QCP::iRangeZoom
        );


    connect(customPlot, &QCustomPlot::mousePress, this, [this](QMouseEvent* ev){ // права кнопка мишки скинути зум
        if (ev->button() == Qt::RightButton && customPlot->selectionRectMode() == QCP::srmZoom) // якщо режим виділення
        {
            customPlot->rescaleAxes();
            customPlot->replot(QCustomPlot::rpQueuedReplot);
        }
        else if (ev->button() == Qt::RightButton && customPlot->selectionRectMode() == QCP::srmNone) {
            // режим drag: шлемо сигнал
            double x = customPlot->xAxis->pixelToCoord(ev->pos().x());
            double y = customPlot->yAxis->pixelToCoord(ev->pos().y());
            emit rightClickInDragMode();
        }
    });

    // ------------------------------------------------------------------------ Основна частина

    connect(customPlot, &QCustomPlot::mousePress, this, [this](QMouseEvent* ev){
        if (ev->button() == Qt::RightButton &&
            customPlot->selectionRectMode() == QCP::srmZoom)
        {
            customPlot->rescaleAxes();
            customPlot->replot(QCustomPlot::rpQueuedReplot);
        }
    });

    // КООРДИНАТИ
    connect(customPlot, &QCustomPlot::mouseMove, this, [this](QMouseEvent* e){
        const bool inside = customPlot->axisRect()->rect().contains(e->pos());
        emit cursorPosChanged(std::numeric_limits<double>::quiet_NaN(),
                              std::numeric_limits<double>::quiet_NaN(), false);
        const double x = customPlot->xAxis->pixelToCoord(e->pos().x());
        const double y = customPlot->yAxis->pixelToCoord(e->pos().y());
        emit cursorPosChanged(x, y, true);
    });


    //  дозвіл на вибір елементів легенди
    customPlot->legend->setSelectableParts(QCPLegend::spItems);

    // взаємодії: додаємо iSelectLegend
    customPlot->setInteractions(
        QCP::iRangeDrag |
        QCP::iRangeZoom |
        QCP::iSelectAxes |
        QCP::iSelectPlottables |
        QCP::iSelectLegend
        );

    // підписка на клік по легенді
    connect(customPlot, &QCustomPlot::legendClick,
            this, &FlightChart::onLegendClick);




    // ====== ruler // статичний підпис влівому куті
    // Рух миші: оновлюємо лінійку/табличку
    connect(customPlot, &QCustomPlot::mouseMove, this, [this](QMouseEvent* ev){
        if (!m_rulerMode) return;
        if (!customPlot->axisRect()->rect().contains(ev->pos())) return;

        ensureRulerItems();

        const double x0 = customPlot->xAxis->pixelToCoord(ev->pos().x());

        // Вертикальна лінія x = x0 (нескінченна, але обрізається прямокутником осей)
        m_rulerLine->point1->setType(QCPItemPosition::ptPlotCoords);
        m_rulerLine->point2->setType(QCPItemPosition::ptPlotCoords);
        m_rulerLine->point1->setCoords(x0, 0.0);
        m_rulerLine->point2->setCoords(x0, 1.0);
        m_rulerLine->setVisible(true);

        // Текст у 2 колонки
        // 1) X — першим рядком
        QString txt = QString("x = %1\n").arg(x0, 0, 'f', 5);

        // 2) Збираємо ім'я та значення, обчислюємо макс. довжину імені для вирівнювання
        int maxLabelLen = 0;
        struct Row { QString name; QString val; };
        QVector<Row> rows;

        for (int i = 0; i < customPlot->graphCount(); ++i) {
            QCPGraph* g = customPlot->graph(i);
            if (!g || !g->visible()) continue;

            const QString name = g->name().isEmpty()
                                     ? QString("Series %1").arg(i+1)
                                     : g->name();

            const double y = valueAtX(g, x0);
            if (std::isnan(y)) continue;

            const QString val = QString::number(y, 'f', 5);
            rows.push_back({name, val});
            maxLabelLen = qMax(maxLabelLen, name.length());
        }

        // 3) Формуємо рівні колонки (моношрифт!):
        //    .arg(<str>, -width) — ліве вирівнювання по фіксованій ширині
        for (const auto& r : rows) {
            txt += QString("%1 : %2\n")
            .arg(r.name, -maxLabelLen)
                .arg(r.val);
        }

        m_rulerInfo->setText(txt.trimmed());
        m_rulerInfo->setVisible(true);

        customPlot->replot(QCustomPlot::rpQueuedReplot);
    });



    // ПКМ у режимі лінійки — прибрати підказку/лінію
    connect(customPlot, &QCustomPlot::mousePress, this, [this](QMouseEvent* ev){
        if (!m_rulerMode) return;
        if (ev->button() == Qt::RightButton)
            clearRuler();
    });

    connect(customPlot, &QCustomPlot::mouseRelease,
            this, [this](QMouseEvent* ev) {
                if (ev->button() == Qt::LeftButton && m_lodEnabled && !m_liveModeEnabled) {
                    scheduleLodRebuild();
                }
            });

    connect(customPlot, &QCustomPlot::mouseWheel,
            this, [this](QWheelEvent*) {
                scheduleLodRebuild();
            });
}

FlightChart::~FlightChart() {}

QCustomPlot* FlightChart::getPlot()
{
    return customPlot;
}

void FlightChart::darkTheme()
{
    customPlot->setBackground(QColor("#2b2b2b"));
    customPlot->axisRect()->setBackground(QColor("#1e1e1e"));
    // customPlot->axisRect()->setBackground(QColor("#ed0d1216"));

    customPlot->xAxis->setTickLabelColor(Qt::white);
    customPlot->yAxis->setTickLabelColor(Qt::white);
    customPlot->xAxis->setLabelColor(Qt::white);
    customPlot->yAxis->setLabelColor(Qt::white);

    customPlot->xAxis->setBasePen(QPen(Qt::gray));
    customPlot->yAxis->setBasePen(QPen(Qt::gray));
    customPlot->xAxis->setTickPen(QPen(Qt::gray));
    customPlot->yAxis->setTickPen(QPen(Qt::gray));
    customPlot->xAxis->setSubTickPen(QPen(Qt::darkGray));
    customPlot->yAxis->setSubTickPen(QPen(Qt::darkGray));

    customPlot->xAxis->grid()->setPen(QPen(QColor(80, 80, 80), 1, Qt::DotLine));
    customPlot->yAxis->grid()->setPen(QPen(QColor(80, 80, 80), 1, Qt::DotLine));

    customPlot->legend->setVisible(true);
    customPlot->legend->setBrush(QColor(30, 30, 30));
    customPlot->legend->setBorderPen(QPen(Qt::gray));
    customPlot->legend->setTextColor(Qt::white);
}

void FlightChart::lightTheme()
{
    customPlot->setBackground(Qt::white);
    customPlot->axisRect()->setBackground(Qt::white);

    customPlot->xAxis->setTickLabelColor(Qt::black);
    customPlot->yAxis->setTickLabelColor(Qt::black);
    customPlot->xAxis->setLabelColor(Qt::black);
    customPlot->yAxis->setLabelColor(Qt::black);

    customPlot->xAxis->setBasePen(QPen(Qt::black));
    customPlot->yAxis->setBasePen(QPen(Qt::black));
    customPlot->xAxis->setTickPen(QPen(Qt::black));
    customPlot->yAxis->setTickPen(QPen(Qt::black));
    customPlot->xAxis->setSubTickPen(QPen(Qt::gray));
    customPlot->yAxis->setSubTickPen(QPen(Qt::gray));

    customPlot->xAxis->grid()->setPen(QPen(QColor(180, 180, 180), 1, Qt::DotLine));
    customPlot->yAxis->grid()->setPen(QPen(QColor(180, 180, 180), 1, Qt::DotLine));

    customPlot->legend->setVisible(true);
    customPlot->legend->setBrush(QColor(240, 240, 240));
    customPlot->legend->setBorderPen(QPen(Qt::black));
    customPlot->legend->setTextColor(Qt::black);
}

void FlightChart::clearPlot()
{
    customPlot->clearGraphs();
    graphKeyByPtr.clear();
    customPlot->replot();
}

void FlightChart::clearData()
{
    clearPlot();
    dataPtr.clear();
    customPlot->replot(QCustomPlot::rpQueuedReplot);
}

void FlightChart::setDataChart(std::vector<parametrs> &data)
{
    dataPtr = data;

    if (!dataPtr.empty()) {
        customPlot->xAxis->setRange(dataPtr.front().time,
                                    dataPtr.back().time);
    }

    rebuildVisibleLod();
}

void FlightChart::appendDataChart(parametrs &data)
{
    dataPtr.push_back(data);
    const double tx = data.time;

    for (int i = 0; i < customPlot->graphCount(); ++i) {
        QCPGraph* g = customPlot->graph(i);
        const auto key = graphKeyByPtr.value(g);
        if (key.isEmpty()) continue;

        auto spec = findFieldByKey(key);
        if (!spec) continue;

        g->addData(tx, spec->getter(data));
        // qDebug() << "Graph" << key << "points:" << g->dataCount();
    }

    if (m_liveModeEnabled) {
        updateLiveWindow(tx); // показуємо останні N точок
    } else {
        // повна історія — можна не рухати діапазон, або плавно тягнути праву межу
        // customPlot->xAxis->setRange(tx, customPlot->xAxis->range().size(), Qt::AlignRight);
    }
    // replot робить панель (throttle)
}

void FlightChart::plotSelectedFields(const QStringList &keys)
{
    customPlot->clearGraphs();
    graphKeyByPtr.clear();

    for (const QString &key : keys)
    {
        auto spec = findFieldByKey(key);
        if (!spec) continue;

        QCPGraph *graph = customPlot->addGraph();
        graphKeyByPtr[graph] = spec->key;

        QString title = spec->label;
        if (!spec->unit.isEmpty()) title += " [" + spec->unit + "]";
        graph->setName(title);

        // колір з override, якщо є
        const QColor baseColor = spec->color;
        const QColor useColor  = m_userColors.value(spec->key, baseColor);

        QPen pen(useColor);
        pen.setWidth(1);
        graph->setPen(pen);

        graph->setAdaptiveSampling(true);

        // 🔹 ЗРОБИТИ ПЛОТ ВИБИРАНИМ (цілий об’єкт)
        graph->setSelectable(QCP::stWhole); // як і було

        auto onSelBool = [this, graph](bool selected){
            if (auto *li = customPlot->legend->itemWithPlottable(graph))
                li->setSelected(selected);

            QPen p = graph->pen();
            p.setWidth(selected ? 2 : 1);
            graph->setPen(p);

            customPlot->replot(QCustomPlot::rpQueuedReplot);
        };

        QObject::connect(
            graph,
            static_cast<void (QCPAbstractPlottable::*)(bool)>(&QCPAbstractPlottable::selectionChanged),
            this,
            onSelBool
            );

        fillGraphLod(graph,
                     spec->key,
                     customPlot->xAxis->range().lower,
                     customPlot->xAxis->range().upper);
    }

    customPlot->legend->setVisible(true);
    customPlot->replot(QCustomPlot::rpQueuedReplot);
}

void FlightChart::updateLiveWindow(double currentX)
{
    if (customPlot->graphCount() == 0) return;

    double left = currentX;

    for (int i = 0; i < customPlot->graphCount(); ++i) {
        QCPGraph* g = customPlot->graph(i);
        const auto &container = *g->data();
        if (container.isEmpty()) continue;

        int n = 0;
        for (auto it = container.constEnd(); it != container.constBegin();) {
            --it; // рух назад
            left = std::min(left, it->key);
            if (++n >= m_liveCount) break;
        }
    }
    customPlot->xAxis->setRange(QCPRange(left, currentX));
}

void FlightChart::setLiveModeEnabled(bool on, int step)
{
    m_liveCount = step;
    m_liveModeEnabled = on;
    // перерахувати діапазон відразу (використаємо поточну праву межу або верхній X даних)
    double rightX = customPlot->xAxis->range().upper;
    if (customPlot->graphCount() > 0) {
        // якщо є дані — беремо останній ключ першого графіка як праву межу
        const auto &container = *customPlot->graph(0)->data();
        if (!container.isEmpty())
            rightX = container.constEnd()[-1].key;
    }
    updateLiveWindow(rightX);
}


void FlightChart::onLegendClick(QCPLegend* /*legend*/,
                                QCPAbstractLegendItem* item,
                                QMouseEvent* ev)
{
    if (!item || ev->button() != Qt::LeftButton) return;

    auto *pli   = qobject_cast<QCPPlottableLegendItem*>(item);
    if (!pli) return;
    auto *graph = qobject_cast<QCPGraph*>(pli->plottable());
    if (!graph) return;

    const QString key = graphKeyByPtr.value(graph);
    if (key.isEmpty()) return;

    const QColor cur = graph->pen().color();

    const QColor chosen = QColorDialog::getColor(cur, this,
                                                 tr("Обрати колір для: %1").arg(graph->name()));
    if (!chosen.isValid()) return;

    // 1) Міняємо колір лише у графіка зараз
    QPen p = graph->pen();
    p.setColor(chosen);
    graph->setPen(p);

    // 2) ⬇️ ПЕРСИСТИМО override, щоб не злітав при перебудові
    m_userColors[key] = chosen;

    // 3) (необов’язково) Оновити дефолт у реєстрі полів, якщо є єдиний контейнер
    if (auto spec = findFieldByKey(key)) {
        spec->color = chosen;
    }

    customPlot->replot(QCustomPlot::rpQueuedReplot);
}


void FlightChart::setSeriesColorByKey(const QString& key, const QColor& c)
{
    // оновити контейнер
    if (auto spec = findFieldByKey(key)) {
        spec->color = c;
    }
    // оновити існуючий графік, якщо намальований
    for (int i = 0; i < customPlot->graphCount(); ++i) {
        auto *g = customPlot->graph(i);
        if (graphKeyByPtr.value(g) == key) {
            QPen p = g->pen();
            p.setColor(c);
            g->setPen(p);
        }
    }
}



// =========================ruler====================

void FlightChart::setRulerMode(bool on)
{
    m_rulerMode = on;
    ensureRulerItems();
    if (!on) clearRuler();
}

void FlightChart::clearRuler()
{
    if (m_rulerLine) m_rulerLine->setVisible(false);
    if (m_rulerInfo) m_rulerInfo->setVisible(false);
    customPlot->replot(QCustomPlot::rpQueuedReplot);
}

void FlightChart::setFieldVisible(const QString& key, bool visible)
{
    if (visible) {
        for (int i = 0; i < customPlot->graphCount(); ++i) {
            QCPGraph* g = customPlot->graph(i);
            if (graphKeyByPtr.value(g) == key)
                return;
        }

        auto spec = findFieldByKey(key);
        if (!spec) return;

        QCPGraph *graph = customPlot->addGraph();
        graphKeyByPtr[graph] = spec->key;

        QString title = spec->label;
        if (!spec->unit.isEmpty())
            title += " [" + spec->unit + "]";

        graph->setName(title);

        QColor useColor = m_userColors.value(spec->key, spec->color);

        QPen pen(useColor);
        pen.setWidth(1);
        graph->setPen(pen);

        graph->setAdaptiveSampling(true);
        graph->setSelectable(QCP::stWhole);

        fillGraphLod(graph,
                     spec->key,
                     customPlot->xAxis->range().lower,
                     customPlot->xAxis->range().upper);
    }
    else {
        for (int i = customPlot->graphCount() - 1; i >= 0; --i) {
            QCPGraph* g = customPlot->graph(i);

            if (graphKeyByPtr.value(g) == key) {
                graphKeyByPtr.remove(g);
                customPlot->removeGraph(g);
                break;
            }
        }
    }

    customPlot->legend->setVisible(true);
    rebuildVisibleLod();
}
void FlightChart::ensureRulerItems() // статичний підпис влівому куті
{
    // Вертикальна лінія: нескінченна пряма, відсічена прямокутником осей
    if (!m_rulerLine) {
        m_rulerLine = new QCPItemStraightLine(customPlot);
        m_rulerLine->setClipAxisRect(customPlot->axisRect());
        m_rulerLine->setPen(QPen(QColor(220,220,220), 1, Qt::DashLine));
        m_rulerLine->setVisible(false);
    }

    // Плашка з текстом: закріплена в лівому верхньому куті прямокутника осей
    if (!m_rulerInfo) {
        m_rulerInfo = new QCPItemText(customPlot);
        m_rulerInfo->setClipAxisRect(customPlot->axisRect());

        // Якір — у лівому верхньому куті (0..1 — відносні координати прямокутника осей)
        m_rulerInfo->position->setType(QCPItemPosition::ptAxisRectRatio);
        m_rulerInfo->setPositionAlignment(Qt::AlignLeft | Qt::AlignTop);
        m_rulerInfo->position->setCoords(0.01, 0.02); // невеликий відступ

        // Стиль
        m_rulerInfo->setPadding(QMargins(6,4,6,4));
        m_rulerInfo->setBrush(QColor(0,0,0));        // напівпрозорий фон
        m_rulerInfo->setPen(QPen(QColor(220,220,220)));  // рамка
        m_rulerInfo->setFont(QFont("DejaVu Sans Mono", 9)); // моношрифт → рівні колонки
        m_rulerInfo->setColor(Qt::white);                // білий текст
        m_rulerInfo->setVisible(false);
        m_rulerInfo->setSelectable(false);
    }
}
double FlightChart::valueAtX(QCPGraph *g, double x) const
{
    if (!g)
        return qQNaN();

    auto data = g->data();
    if (!data || data->isEmpty())
        return qQNaN();

    auto it = data->constBegin();
    auto end = data->constEnd();

    // Якщо x лівіше першої точки
    if (x < it->key)
        return qQNaN();

    auto prev = it;
    ++it;

    // Якщо в графіку лише одна точка
    if (it == end) {
        if (qAbs(prev->key - x) < 1e-9)
            return prev->value;

        return qQNaN();
    }

    for (; it != end; ++it) {
        const double x1 = prev->key;
        const double y1 = prev->value;

        const double x2 = it->key;
        const double y2 = it->value;

        if (x >= x1 && x <= x2) {
            if (qAbs(x2 - x1) < 1e-12)
                return y1;

            const double k = (x - x1) / (x2 - x1);
            return y1 + k * (y2 - y1);
        }

        prev = it;
    }

    return qQNaN();
}
// double FlightChart::valueAtX(QCPGraph *g, double x) const
// {
//     if (!g) return qQNaN();

//     const auto &cont = *g->data();
//     if (cont.isEmpty()) return qQNaN();

//     auto first = cont.constBegin();
//     auto last  = cont.constEnd();
//     --last;

//     // x поза областю даних
//     if (x < first->key || x > last->key)
//         return qQNaN();

//     auto itUpper = cont.findBegin(x, true);

//     if (itUpper == cont.constBegin())
//         return itUpper->value;

//     auto itLower = itUpper;
//     --itLower;

//     const double x1 = itLower->key;
//     const double y1 = itLower->value;

//     const double x2 = itUpper->key;
//     const double y2 = itUpper->value;

//     if (qFuzzyCompare(x1, x2))
//         return y1;

//     const double t = (x - x1) / (x2 - x1);
//     return y1 + t * (y2 - y1);
// }

void FlightChart::setLodEnabled(bool on)
{
    m_lodEnabled = on;
    rebuildVisibleLod();
}

void FlightChart::setLodPointLimitPerGraph(int points)
{
    m_lodPointLimitPerGraph = qMax(100, points);
    rebuildVisibleLod();
}

void FlightChart::scheduleLodRebuild()
{
    if (m_lodRebuildScheduled)
        return;

    m_lodRebuildScheduled = true;

    QTimer::singleShot(50, this, [this]() {
        m_lodRebuildScheduled = false;
        rebuildVisibleLod();
    });
}
void FlightChart::rebuildVisibleLod()
{
    if (!customPlot || dataPtr.empty())
        return;

    const double xMin = customPlot->xAxis->range().lower;
    const double xMax = customPlot->xAxis->range().upper;

    for (int i = 0; i < customPlot->graphCount(); ++i) {
        QCPGraph* graph = customPlot->graph(i);
        if (!graph)
            continue;

        const QString key = graphKeyByPtr.value(graph);
        if (key.isEmpty())
            continue;

        fillGraphLod(graph, key, xMin, xMax);
    }

    customPlot->replot(QCustomPlot::rpQueuedReplot);
}

void FlightChart::fillGraphLod(QCPGraph* graph,
                               const QString& key,
                               double xMin,
                               double xMax)
{
    auto spec = findFieldByKey(key);
    if (!spec || !graph)
        return;

    QVector<double> x;
    QVector<double> y;

    if (dataPtr.empty()) {
        graph->setData(x, y);
        return;
    }

    auto firstIt = std::lower_bound(
        dataPtr.begin(),
        dataPtr.end(),
        xMin,
        [](const parametrs& p, double value) {
            return p.time < value;
        });

    auto lastIt = std::upper_bound(
        dataPtr.begin(),
        dataPtr.end(),
        xMax,
        [](double value, const parametrs& p) {
            return value < p.time;
        });

    if (firstIt == dataPtr.end() || firstIt >= lastIt) {
        graph->setData(x, y);
        return;
    }

    const int first = int(std::distance(dataPtr.begin(), firstIt));
    const int lastExclusive = int(std::distance(dataPtr.begin(), lastIt));
    const int visibleCount = lastExclusive - first;

    if (visibleCount <= 0) {
        graph->setData(x, y);
        return;
    }

    // if (!m_lodEnabled || visibleCount <= m_lodPointLimitPerGraph) { // Варіант алгоритму min max
    //     x.reserve(visibleCount);
    //     y.reserve(visibleCount);

    //     for (int i = first; i < lastExclusive; ++i) {
    //         x.append(dataPtr[i].time);
    //         y.append(spec->getter(dataPtr[i]));
    //     }

    //     graph->setData(x, y);
    //     return;
    // }

    // const int bucketCount = qMax(1, m_lodPointLimitPerGraph / 2);
    // const double bucketSize = double(visibleCount) / double(bucketCount);

    // x.reserve(bucketCount * 2);
    // y.reserve(bucketCount * 2);

    // for (int b = 0; b < bucketCount; ++b) {
    //     const int i0 = first + int(b * bucketSize);
    //     int i1 = first + int((b + 1) * bucketSize) - 1;

    //     if (i0 >= lastExclusive)
    //         break;

    //     if (i1 >= lastExclusive)
    //         i1 = lastExclusive - 1;

    //     int minIndex = i0;
    //     int maxIndex = i0;

    //     double minValue = spec->getter(dataPtr[i0]);
    //     double maxValue = minValue;

    //     for (int i = i0 + 1; i <= i1; ++i) {
    //         const double v = spec->getter(dataPtr[i]);

    //         if (v < minValue) {
    //             minValue = v;
    //             minIndex = i;
    //         }

    //         if (v > maxValue) {
    //             maxValue = v;
    //             maxIndex = i;
    //         }
    //     }

    //     if (minIndex <= maxIndex) {
    //         x.append(dataPtr[minIndex].time);
    //         y.append(minValue);

    //         if (maxIndex != minIndex) {
    //             x.append(dataPtr[maxIndex].time);
    //             y.append(maxValue);
    //         }
    //     } else {
    //         x.append(dataPtr[maxIndex].time);
    //         y.append(maxValue);

    //         x.append(dataPtr[minIndex].time);
    //         y.append(minValue);
    //     }
    // }

    // graph->setData(x, y);

    if (!m_lodEnabled || visibleCount <= m_lodPointLimitPerGraph) { // ==== Варіант тупої фітраціх N
        x.reserve(visibleCount);
        y.reserve(visibleCount);

        for (int i = first; i < lastExclusive; ++i) {
            x.append(dataPtr[i].time);
            y.append(spec->getter(dataPtr[i]));
        }

        graph->setData(x, y);
        return;
    }

    // Просте швидке прорідження: кожна N-та точка
    const int step = qMax(1, visibleCount / m_lodPointLimitPerGraph);

    const int approxCount = visibleCount / step + 1;
    x.reserve(approxCount);
    y.reserve(approxCount);

    for (int i = first; i < lastExclusive; i += step) {
        x.append(dataPtr[i].time);
        y.append(spec->getter(dataPtr[i]));
    }

    // Обов'язково додати останню точку видимого діапазону
    const int lastIndex = lastExclusive - 1;
    if (!x.isEmpty() && x.last() != dataPtr[lastIndex].time) {
        x.append(dataPtr[lastIndex].time);
        y.append(spec->getter(dataPtr[lastIndex]));
    }

    graph->setData(x, y);

}
