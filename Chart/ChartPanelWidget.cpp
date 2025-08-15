#include "chartpanelwidget.h"
#include "flightchart.h"
#include "Fields_parametrs.h"
#include "qcustomplot.h"


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

    // --------------------------  test Даних
    testTimer = new QTimer(this);
    connect(testTimer, &QTimer::timeout, this, &ChartPanelWidget::generateTestData);
    testTimer->start(1); // 100 Гц
    // --------------------------  test Даних


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
     mLastReplot.start();  // Timer який оновлює сам графік всередині в незалежності приходсять дані чи ні
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

void ChartPanelWidget::generateTestData()
{
    static double t = 0.0;
    static const double dt = 0.1;

    auto rnd = [](double a, double b){
        return a + (b - a) * QRandomGenerator::global()->generateDouble();
    };

    parametrs p{};
    p.time = t;

    // --- приклад правдоподібних значень ---
    p.v_ground = rnd(200.0, 250.0) + 5.0 * qSin(t * 0.1);
    p.vx = p.v_ground + 2.0 * qSin(t * 0.15);
    p.vy = rnd(-5.0, 5.0) + 1.0 * qCos(t * 0.07);
    p.vz = rnd(-2.0, 2.0);

    p.wx = qDegreesToRadians(rnd(-2.0, 2.0));
    p.wy = qDegreesToRadians(rnd(-2.0, 2.0));
    p.wz = qDegreesToRadians(rnd(-2.0, 2.0));

    static double aWx = 0.0, aWy = 0.0, aWz = 0.0;
    aWx += p.wx * dt;  aWy += p.wy * dt;  aWz += p.wz * dt;
    p.angleWx = aWx;   p.angleWy = aWy;   p.angleWz = aWz;

    p.teta  = qDegreesToRadians(rnd(-10.0, 10.0));
    p.gamma = qDegreesToRadians(rnd(-30.0, 30.0));
    p.psi   = qDegreesToRadians(std::fmod(3.6 * t, 360.0)); // 0.01*360*t

    p.alfa = qDegreesToRadians(rnd(-2.0, 8.0));
    p.beta = qDegreesToRadians(rnd(-3.0, 3.0));

    p.machNumber   = rnd(0.6, 0.9);
    p.altitude_wgs = rnd(100.0, 200.0) + 2.0 * qCos(t * 0.05);

    p.nx = rnd(-0.2, 0.2);
    p.ny = rnd(-0.2, 0.2);
    p.nz = 1.0 + rnd(-0.05, 0.05);

    p.mx = rnd(-100.0, 100.0);
    p.my = rnd(-100.0, 100.0);
    p.mz = rnd(-100.0, 100.0);

    p.deltaChannel1 = rnd(-5.0, 5.0);
    p.deltaChannel2 = rnd(-5.0, 5.0);
    p.deltaElerons  = rnd(-5.0, 5.0);

    p.xg += p.vx * dt; p.yg += p.vy * dt; p.zg += p.vz * dt;
    p.mass  = rnd(500.0, 600.0);
    p.trust = rnd(2000.0, 3000.0);
    // --- кінець генерації ---

    this->appendDataStep(p);   // ✅ саме панель, не FlightChart

    t += dt;
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
