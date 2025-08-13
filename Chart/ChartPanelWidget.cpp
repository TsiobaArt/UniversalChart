#include "chartpanelwidget.h"
#include "flightchart.h"
#include "Fields_parametrs.h"
#include "qcustomplot.h"

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QPushButton>
#include <QRadioButton>
#include <QScrollArea>
#include <QLabel>
#include <QCheckBox>
#include <QSizePolicy>
#include <QTimer>

ChartPanelWidget::ChartPanelWidget(QWidget *parent)
    : QWidget(parent)
{
    QHBoxLayout *mainLayout = new QHBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(5);

    // ------------------ ГРАФІК ------------------
    flightChart = new FlightChart;

    // ------------------ КНОПКИ ------------------
    QVBoxLayout *toolLayout = new QVBoxLayout;
    toolLayout->setSpacing(4);
    toolLayout->setContentsMargins(4, 4, 4, 4);

    QPushButton *themeBtn = new QPushButton("Тема");
    QPushButton *zoomBtn = new QPushButton("Автозум");
    QRadioButton *dragBtn = new QRadioButton("Рука");
    QRadioButton *rectBtn = new QRadioButton("Виділення");
    QPushButton *clearBtn = new QPushButton("Очистити");
    liveBtn = new QPushButton("Live 100"); // toggle

    dragBtn->setChecked(true);

    toolLayout->addWidget(themeBtn);
    toolLayout->addWidget(zoomBtn);
    toolLayout->addWidget(dragBtn);
    toolLayout->addWidget(rectBtn);
    toolLayout->addSpacing(10);
    toolLayout->addWidget(new QLabel("Поля:"));
    toolLayout->addWidget(liveBtn);
    toolLayout->addWidget(clearBtn);

    QWidget *toolWidget = new QWidget;
    toolWidget->setLayout(toolLayout);

    // ------------------ СКРОЛ ЧЕКБОКСІВ ------------------
    QWidget *checkboxContent = new QWidget;
    QVBoxLayout *checkboxLayout = new QVBoxLayout(checkboxContent);
    checkboxLayout->setAlignment(Qt::AlignTop);
    checkboxLayout->setSpacing(2);
    checkboxLayout->setContentsMargins(2, 2, 2, 2);

    for (const auto &f : PARAM_FIELDS()) {
        QCheckBox *cb = new QCheckBox(f.label);
        cb->setChecked(f.defaultChecked);
        checkboxes[f.key] = cb;
        checkboxLayout->addWidget(cb);
        connect(cb, &QCheckBox::stateChanged, this, &ChartPanelWidget::onCheckboxChanged);
    }

    QScrollArea *scrollArea = new QScrollArea;
    scrollArea->setWidgetResizable(true);
    scrollArea->setWidget(checkboxContent);
    scrollArea->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    // ------------------ ЛІВА ПАНЕЛЬ ------------------
    QVBoxLayout *leftPanelLayout = new QVBoxLayout;
    leftPanelLayout->setContentsMargins(2, 2, 2, 2);
    leftPanelLayout->setSpacing(5);
    leftPanelLayout->addWidget(toolWidget);
    leftPanelLayout->addWidget(scrollArea, 1);

    QWidget *leftPanel = new QWidget;
    leftPanel->setLayout(leftPanelLayout);
    leftPanel->setMaximumWidth(300);

    // ------------------ ЗБІР ВСЬОГО ------------------
    mainLayout->addWidget(leftPanel);
    mainLayout->addWidget(flightChart, 1);

    // ------------------ КОНЕКТИ ------------------
    connect(themeBtn, &QPushButton::clicked, this, &ChartPanelWidget::toggleTheme);
    connect(zoomBtn, &QPushButton::clicked, this, &ChartPanelWidget::autoZoom);
    connect(clearBtn, &QPushButton::clicked, this, &ChartPanelWidget::clearSelection);

    connect(dragBtn, &QRadioButton::toggled, this, [=](bool checked){
        if (checked) {
            flightChart->getPlot()->setSelectionRectMode(QCP::srmNone);
            flightChart->getPlot()->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom | QCP::iSelectAxes);
        }
    });
    connect(rectBtn, &QRadioButton::toggled, this, [=](bool checked){
        if (checked) {
            flightChart->getPlot()->setSelectionRectMode(QCP::srmZoom);
            flightChart->getPlot()->setInteractions(QCP::iRangeZoom | QCP::iSelectPlottables | QCP::iSelectAxes);
        }
    });

    connect(liveBtn, &QPushButton::clicked, this, [this]{
        const bool on = !flightChart->liveModeEnabled();
        flightChart->setLiveModeEnabled(on);
        liveBtn->setText(on ? "Full history" : "Live 100");
        scheduleReplot();
    });

    mLastReplot.start();
}

void ChartPanelWidget::setData(std::vector<parametrs> &data) {
    flightChart->setDataChart(data);
    onCheckboxChanged();
}

void ChartPanelWidget::onCheckboxChanged() {
    QStringList selected;
    for (auto it = checkboxes.begin(); it != checkboxes.end(); ++it) {
        if (it.value()->isChecked())
            selected << it.key();
    }
    flightChart->plotSelectedFields(selected);
}

void ChartPanelWidget::appendDataStep(parametrs data)
{
    flightChart->appendDataChart(data);

    if (flightChart->getPlot()->graphCount() == 0) {
        onCheckboxChanged();
    }

    scheduleReplot();
}

void ChartPanelWidget::scheduleReplot()
{
    if (mReplotScheduled) return;

    const qint64 elapsed = mLastReplot.elapsed();
    if (elapsed >= mMinReplotIntervalMs) {
        mReplotScheduled = true;
        QTimer::singleShot(0, this, [this]{
            flightChart->getPlot()->replot(QCustomPlot::rpQueuedReplot);
            mLastReplot.restart();
            mReplotScheduled = false;
        });
    } else {
        mReplotScheduled = true;
        QTimer::singleShot(int(mMinReplotIntervalMs - elapsed), this, [this]{
            flightChart->getPlot()->replot(QCustomPlot::rpQueuedReplot);
            mLastReplot.restart();
            mReplotScheduled = false;
        });
    }
}

void ChartPanelWidget::clearData()
{
    flightChart->clearData();
}

void ChartPanelWidget::updatePlot() {
    flightChart->getPlot()->replot(QCustomPlot::rpQueuedReplot);
}

void ChartPanelWidget::clearSelection()
{
    for (auto cb : checkboxes)
        cb->setChecked(false);
    flightChart->clearPlot();
}

void ChartPanelWidget::toggleTheme()
{
    if (darkTheme)
        flightChart->lightTheme();
    else
        flightChart->darkTheme();

    darkTheme = !darkTheme;
    flightChart->getPlot()->replot();
}

void ChartPanelWidget::autoZoom()
{
    flightChart->getPlot()->rescaleAxes();
    flightChart->getPlot()->replot();
}
