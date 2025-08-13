#pragma once

#include <QWidget>
#include <QVector>
#include <QHash>
#include <QStringList>
#include "parametrs.h"

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
    void setLiveModeEnabled(bool on);
    bool liveModeEnabled() const { return m_liveModeEnabled; }
    void setLiveCount(int n) { m_liveCount = n > 0 ? n : 1; }
    int liveCount() const { return m_liveCount; }

    // (Не обов'язково) часове вікно, якщо захочеш повернутись до секунд
    void setTimeWindow(double seconds) { m_timeWindowSec = seconds; }

private:
    void updateLiveWindow(double currentX);

    QCustomPlot* customPlot = nullptr;
    std::vector<parametrs> dataPtr;
    QHash<QCPGraph*, QString> graphKeyByPtr; // graph -> field key

    // налаштування
    double m_timeWindowSec = 15.0;   // не використовується для видалення
    bool   m_liveModeEnabled = false;
    int    m_liveCount = 100;
};
