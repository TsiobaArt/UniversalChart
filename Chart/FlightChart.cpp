#include "flightchart.h"
#include <QVBoxLayout>
#include <QPen>
#include <QBrush>
#include "qcustomplot.h"
#include "Fields_parametrs.h"
#include "Fields_lookup.h"

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
    customPlot->legend->setSelectableParts(QCPLegend::spNone);
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
    });

    // ------------------------------------------------------------------------ Основна частина
    // customPlot->axisRect()->setAutoMargins(QCP::msNone);
    // customPlot->axisRect()->setMargins(QMargins(40, 0, 0, 30)); // під себе

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
}

void FlightChart::setDataChart(std::vector<parametrs> &data) {
    dataPtr.clear();
    dataPtr = data;
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

        QPen pen(spec->color);
        pen.setWidth(1);
        graph->setPen(pen);

        // [OPTIM] Adaptive sampling завжди вкл
        graph->setAdaptiveSampling(true);

        QVector<double> x, y;
        x.reserve(static_cast<int>(dataPtr.size()));
        y.reserve(static_cast<int>(dataPtr.size()));
        for (const auto &d : dataPtr) {
            x.append(d.time);
            y.append(spec->getter(d));
        }
        graph->setData(x, y);


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
