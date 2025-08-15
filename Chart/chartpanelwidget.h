#pragma once

#include <QWidget>
#include <QMap>
#include <QStringList>
#include <QElapsedTimer>
#include "parametrs.h"

class QCheckBox;
class QLabel;
class QPushButton;
class QRadioButton;
class FlightChart;

class ChartPanelWidget : public QWidget
{
    Q_OBJECT
public:
    explicit ChartPanelWidget(QWidget *parent = nullptr);

    void setData(std::vector<parametrs> &data);
    void appendDataStep(parametrs data);
    void clearData();

private slots:
    void onCheckboxChanged();
    void updatePlot();
    void clearSelection();
    void toggleTheme();
    void autoZoom();

private:
    void scheduleReplot();

    FlightChart *flightChart = nullptr;
    QMap<QString, QCheckBox*> checkboxes; // key -> checkbox
    bool darkTheme = true;

    // throttle перемальовки
    bool mReplotScheduled = false;
    QElapsedTimer mLastReplot;
    int mMinReplotIntervalMs = 20; //  16 мс ~60 FPS    20 мс ~50 FPS

    // Live кнопка
    QPushButton *liveBtn = nullptr;
};
