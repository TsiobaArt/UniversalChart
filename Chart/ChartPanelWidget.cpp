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
#include <QQmlContext>
#include <QQuickItem>
ChartPanelWidget::ChartPanelWidget(QWidget *parent)
    : QWidget(parent)
{
    // ===== ГОЛОВНИЙ ВЕРТИКАЛЬНИЙ ЛЕЙАУТ =====
    auto *rootLayout = new QVBoxLayout(this);
    rootLayout->setContentsMargins(0, 0, 0, 0);
    rootLayout->setSpacing(5);

    // ===== ВЕРХНЯ QML ПАНЕЛЬ =====
    m_qmlTopBar = new QQuickWidget(this);
    m_qmlTopBar->setResizeMode(QQuickWidget::SizeRootObjectToView);
    m_qmlTopBar->setFixedHeight(50);          // <-- фіксована висота 50
    rootLayout->addWidget(m_qmlTopBar);       // займає всю ширину віджета

    // ===== НИЖНІЙ РЯД (ТВІЙ ІСНУЮЧИЙ КОД) =====
    QHBoxLayout *mainLayout = new QHBoxLayout();
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(5);
    rootLayout->addLayout(mainLayout, 1);     // тягнеться на весь залишок

    // ------------------ ГРАФІК ------------------
    flightChart = new FlightChart;

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
    leftPanelLayout->addWidget(scrollArea, 1);

    QWidget *leftPanel = new QWidget;
    leftPanel->setLayout(leftPanelLayout);
    leftPanel->setMaximumWidth(300);

    // ------------------ ЗБІР ВСЬОГО ------------------
    mainLayout->addWidget(leftPanel);
    mainLayout->addWidget(flightChart, 1);
    // 1) Встановлюємо джерело QML
    m_qmlTopBar->setSource(QUrl(QStringLiteral("qrc:/Chart_panel.qml")));



    // QObject *toolbarRoot = m_qmlTopBar->rootObject();
    // if (!toolbarRoot)
    //     return;

    // connect(toolbarRoot, SIGNAL(themeToggle()),
    //         this, SLOT(onTopBarThemeToggle()), Qt::UniqueConnection);
    // connect(toolbarRoot, SIGNAL(autoZoom()),
    //         this, SLOT(onTopBarAutoZoom()), Qt::UniqueConnection);
    // connect(toolbarRoot, SIGNAL(liveToggle()),
    //         this, SLOT(onTopBarLiveToggle()), Qt::UniqueConnection);
    // connect(toolbarRoot, SIGNAL(modeChanged(QString)),
    //         this, SLOT(onTopBarModeChanged(QString)), Qt::UniqueConnection);
    // connect(toolbarRoot, SIGNAL(clearRequested()),
    //         this, SLOT(onTopBarClearRequested()), Qt::UniqueConnection);
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
    flightChart->getPlot()->replot(QCustomPlot::rpQueuedReplot);;
}

void ChartPanelWidget::autoZoom()
{
    flightChart->getPlot()->rescaleAxes();
    flightChart->getPlot()->replot(QCustomPlot::rpQueuedReplot);
}
