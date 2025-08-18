#include "chartpanelwidget.h"
#include "flightchart.h"
#include "Fields_parametrs.h"
#include "qcustomplot.h"
#include <QQmlContext>
#include <QQuickWidget>

ChartPanelWidget::ChartPanelWidget(QWidget *parent)
    : QWidget(parent)
{
    // ===== ГОЛОВНИЙ ГОРИЗОНТАЛЬНИЙ ЛЕЙАУТ (ЛІВО: чекбокси, ПРАВО: панель+графік) =====
    auto *rootRow = new QHBoxLayout(this);
    rootRow->setContentsMargins(0, 0, 0, 0);
    rootRow->setSpacing(5);

    // ------------------ ЛІВА ПАНЕЛЬ (СКРОЛ ЧЕКБОКСІВ) ------------------
    QWidget *checkboxContent = new QWidget;
    QVBoxLayout *checkboxLayout = new QVBoxLayout(checkboxContent);
    checkboxLayout->setAlignment(Qt::AlignTop);
    checkboxLayout->setSpacing(2);
    checkboxLayout->setContentsMargins(6, 6, 6, 6);

    for (const auto &f : PARAM_FIELDS()) {
        QCheckBox *cb = new QCheckBox(f.label);
        cb->setChecked(f.defaultChecked);

        // робимо чекбокси "у всю довжину" (тобто на всю доступну ширину панелі)
        cb->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
        // cb->setWordWrap(true); // якщо підписи довгі — переносимо

        checkboxes[f.key] = cb;
        checkboxLayout->addWidget(cb);
        connect(cb, &QCheckBox::stateChanged, this, &ChartPanelWidget::onCheckboxChanged);
    }
    checkboxLayout->addStretch(); // щоб зверху було щільно, а низ заповнювався

    QScrollArea *scrollArea = new QScrollArea;
    scrollArea->setWidgetResizable(true);
    scrollArea->setWidget(checkboxContent);
    scrollArea->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);

    QWidget *leftPanel = new QWidget;
    auto *leftPanelLayout = new QVBoxLayout(leftPanel);
    leftPanelLayout->setContentsMargins(0, 0, 0, 0);
    leftPanelLayout->setSpacing(0);
    leftPanelLayout->addWidget(scrollArea, 1);
    leftPanel->setMaximumWidth(300); // або за бажанням: setFixedWidth(280);

    // ------------------ ПРАВА КОЛОНКА: ВЕРХНЯ QML ПАНЕЛЬ + ГРАФІК ------------------
    QWidget *rightCol = new QWidget;
    auto *rightColLayout = new QVBoxLayout(rightCol);
    rightColLayout->setContentsMargins(0, 0, 0, 0);
    rightColLayout->setSpacing(5);

    // ===== ВЕРХНЯ QML ПАНЕЛЬ (належить тільки правій колонці з графіком) =====
    m_qmlTopBar = new QQuickWidget(rightCol);
    m_qmlTopBar->setResizeMode(QQuickWidget::SizeRootObjectToView);
    m_qmlTopBar->setFixedHeight(50);          // <-- фіксована висота 50

    windowColor =   QApplication::palette().color(QPalette::Window);
    m_qmlTopBar->rootContext()->setContextProperty("appWindowColor", windowColor);

    m_qmlTopBar->setSource(QUrl(QStringLiteral("qrc:/Chart_panel.qml")));
    // ...

    // ------------------ ГРАФІК ------------------
    flightChart = new FlightChart;

    rightColLayout->addWidget(m_qmlTopBar);   // панель тільки над графіком (у правій колонці)
    rightColLayout->addWidget(flightChart, 1);

    // ------------------ ЗБІР ВСЬОГО ------------------
    rootRow->addWidget(leftPanel);            // лівий стовпчик: чекбокси
    rootRow->addWidget(rightCol, 1);          // правий стовпчик: панель+графік

    // --------------------------  test Даних
    testTimer = new QTimer(this);
    connect(testTimer, &QTimer::timeout, this, &ChartPanelWidget::generateTestData);
    testTimer->start(1); // 100 Гц
    // --------------------------  test Даних

    // ===== з'єднання сигналів з QML панелі (за потреби розкоментуй) =====
    // QObject *toolbarRoot = m_qmlTopBar->rootObject();
    // if (toolbarRoot) {
    //     connect(toolbarRoot, SIGNAL(themeToggle()),
    //             this, SLOT(onTopBarThemeToggle()), Qt::UniqueConnection);
    //     connect(toolbarRoot, SIGNAL(autoZoom()),
    //             this, SLOT(onTopBarAutoZoom()), Qt::UniqueConnection);
    //     connect(toolbarRoot, SIGNAL(liveToggle()),
    //             this, SLOT(onTopBarLiveToggle()), Qt::UniqueConnection);
    //     connect(toolbarRoot, SIGNAL(modeChanged(QString)),
    //             this, SLOT(onTopBarModeChanged(QString)), Qt::UniqueConnection);
    //     connect(toolbarRoot, SIGNAL(clearRequested()),
    //             this, SLOT(onTopBarClearRequested()), Qt::UniqueConnection);
    // }

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
