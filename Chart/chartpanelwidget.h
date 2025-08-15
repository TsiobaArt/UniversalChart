#pragma once

#include <QWidget>
#include <QMap>
#include <QStringList>
#include <QElapsedTimer>
#include "parametrs.h"
#include <QQuickWidget>
#include <QTimer>
#include <QRandomGenerator>
#include <QtMath>
#include "parametrs.h"
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QPushButton>
#include <QRadioButton>
#include <QScrollArea>
#include <QLabel>
#include <QCheckBox>
#include <QSizePolicy>
#include <QTimer>
#include <QQmlContext>
#include <QQuickItem>

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
    void generateTestData();

private slots:
    void onCheckboxChanged();
    void updatePlot();
    void clearSelection();
    void toggleTheme();
    void autoZoom();

private:
    void scheduleReplot();
    QQuickWidget* m_qmlTopBar = nullptr;

    FlightChart *flightChart = nullptr;
    QMap<QString, QCheckBox*> checkboxes; // key -> checkbox
    bool darkTheme = true;

    // throttle перемальовки
    bool mReplotScheduled = false;
    QElapsedTimer mLastReplot;
    int mMinReplotIntervalMs = 16; //  16 мс ~60 FPS    20 мс ~50 FPS

    // Live кнопка
    QPushButton *liveBtn = nullptr;
    QTimer *testTimer = nullptr;

};
