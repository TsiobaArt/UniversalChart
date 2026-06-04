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


    void setLodEnabled(bool on);
    void setLodPointLimitPerGraph(int points);
    void rebuildVisibleLod();

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
    bool m_rmbDown = false;   // Права кнопка затиснута?

    bool m_lodEnabled = true;
    int  m_lodPointLimitPerGraph = 100000;
    bool m_lodRebuildScheduled = false;

    void scheduleLodRebuild();

    void fillGraphLod(QCPGraph* graph,
                      const QString& key,
                      double xMin,
                      double xMax);


signals:
    void cursorPosChanged(double x, double y, bool inside);
    void rightClickInDragMode();
};
