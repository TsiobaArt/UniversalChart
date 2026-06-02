#pragma once

#include <QWidget>
#include <QVector>
#include <QHash>
#include <QStringList>
#include "parametrs.h"
#include "qcustomplot.h"

class QCustomPlot;
class QCPGraph;

class FlightChart : public QWidget
{
    Q_OBJECT
public:
    explicit FlightChart(QWidget *parent = nullptr);
    ~FlightChart();

    QCustomPlot* getPlot();

    void darkTheme();
    void lightTheme();

    void clearPlot();
    void clearData();

    void setDataChart(std::vector<parametrs> &data);
    void appendDataChart(parametrs &data);
    void plotSelectedFields(const QStringList &fieldKeys);

    // Live режим: показувати лише останні N точок (без видалення старих)
    void setLiveModeEnabled(bool on, int step);
    bool liveModeEnabled() const { return m_liveModeEnabled; }
    void setLiveCount(int n) { m_liveCount = n > 0 ? n : 1; }
    int liveCount() const { return m_liveCount; }

    // (Не обов'язково) часове вікно, якщо захочеш повернутись до секунд
    void setTimeWindow(double seconds) { m_timeWindowSec = seconds; }

    const std::vector<parametrs>& rawData() const { return dataPtr; }



    // ====== ruler ======
    void setRulerMode(bool on);
    void clearRuler();



    // =======скидання одного чекбоксу а не всіх ===============
    void setFieldVisible(const QString& key, bool visible);

public slots:
    void onLegendClick(QCPLegend* legend, QCPAbstractLegendItem* item, QMouseEvent* ev);


private:
    void updateLiveWindow(double currentX);

    QCustomPlot* customPlot = nullptr;
    std::vector<parametrs> dataPtr;
    QHash<QCPGraph*, QString> graphKeyByPtr; // graph -> field key
    QHash<QString, QColor> m_userColors;     // key -> user override color

    // налаштування
    double m_timeWindowSec = 15.0;   // не використовується для видалення
    bool   m_liveModeEnabled = false;
    int    m_liveCount = 1000;

    // допоміжне: встановити колір графіка за ключем
    void setSeriesColorByKey(const QString& key, const QColor& c);


    // ====== ruler ======
    bool m_rulerMode = false;
    QCPItemStraightLine* m_rulerLine = nullptr;
    QCPItemText*         m_rulerInfo = nullptr;
    void ensureRulerItems();
    double valueAtX(QCPGraph* g, double x) const; // лінійна інтерполяція

private:
    bool m_rmbDown = false;   // Права кнопка затиснута?
signals:
    void cursorPosChanged(double x, double y, bool inside);
    void rightClickInDragMode();
};
    // customPlot->axisRect()->setAutoMargins(QCP::msNone);
    // customPlot->axisRect()->setMargins(QMargins(40, 0, 0, 30)); // під себе

//Варіант щоб показувати тільки по парвій кнопці мишки
// // Права кнопка: автозум у режимі прямокутного зуму + прапор rmb
// connect(customPlot, &QCustomPlot::mousePress, this, [this](QMouseEvent* ev){
//     if (ev->button() == Qt::RightButton) {
//         m_rmbDown = true;
//         if (customPlot->selectionRectMode() == QCP::srmZoom) {
//             customPlot->rescaleAxes();
//             customPlot->replot(QCustomPlot::rpQueuedReplot);
//         }
//     }
// });

// connect(customPlot, &QCustomPlot::mouseRelease, this, [this](QMouseEvent* ev){
//     if (ev->button() == Qt::RightButton)
//         m_rmbDown = false;
// });

// // Координати курсора (тільки коли затиснута права кнопка)
// connect(customPlot, &QCustomPlot::mouseMove, this, [this](QMouseEvent* e){
//     // показуємо лише під час RMB
//     if (!m_rmbDown) {
//         emit cursorPosChanged(std::numeric_limits<double>::quiet_NaN(),
//                               std::numeric_limits<double>::quiet_NaN(),
//                               false);
//         return;
//     }
//     const bool inside = customPlot->axisRect()->rect().contains(e->pos());
//     double x = std::numeric_limits<double>::quiet_NaN();
//     double y = std::numeric_limits<double>::quiet_NaN();
//     if (inside) {
//         x = customPlot->xAxis->pixelToCoord(e->pos().x());
//         y = customPlot->yAxis->pixelToCoord(e->pos().y());
//     }
//     emit cursorPosChanged(x, y, inside);
// });
