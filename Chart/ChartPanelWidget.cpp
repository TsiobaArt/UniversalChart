#include "chartpanelwidget.h"
#include "flightchart.h"
#include "Fields_parametrs.h"
#include "qcustomplot.h"
#include <QQmlContext>
#include <QQuickWidget>
ChartPanelWidget::ChartPanelWidget(QWidget *parent)
    : QWidget(parent)
{
    setMinimumSize(840, 600);   // мінімальний розмір вікна (ширина × висота)
    setupUi();                  // вся побудова GUI тут
    mLastReplot.start();
    for (auto it = checkboxes.begin(); it != checkboxes.end(); ++it) {
        const QString key = it.key();
        QCheckBox *cb = it.value();

        connect(cb, &QCheckBox::toggled, this, [this, key](bool checked) {
            flightChart->setFieldVisible(key, checked);
        });
    }
    // ------------------------------  test Даних на кіклькість
    generateTestDataCount(1000000);
    // ------------------------------ test Даних на кількість

    QObject *toolbarRoot = m_qmlTopBar->rootObject();
    if (toolbarRoot) {
        // connect(toolbarRoot, SIGNAL(themeToggle()),this, SLOT(onTopBarThemeToggle()), Qt::UniqueConnection);
        connect(toolbarRoot, SIGNAL(autoZoom()),this, SLOT(autoZoom()), Qt::UniqueConnection);
        connect(toolbarRoot, SIGNAL(liveButt(bool, int)),this, SLOT(liveButt(bool,int)), Qt::UniqueConnection);
        connect(toolbarRoot, SIGNAL(modeChanged(QString)), this, SLOT(modeChange(QString)), Qt::UniqueConnection);
        connect(toolbarRoot, SIGNAL(clearSelection()), this, SLOT(clearSelection()), Qt::UniqueConnection);
        connect(toolbarRoot, SIGNAL(exportCsv(QString)), this, SLOT(exportToCsv(QString)), Qt::UniqueConnection);
        connect(toolbarRoot, SIGNAL(exportImage(QString)), this, SLOT(exportImage(QString)),  Qt::UniqueConnection);
        connect(toolbarRoot, SIGNAL(deleteDataChart()), this, SLOT(deleteData()),  Qt::UniqueConnection);
        connect(toolbarRoot, SIGNAL(autoZoomX()), this, SLOT(autoZoomX()),  Qt::UniqueConnection);
        connect(toolbarRoot, SIGNAL(importCSV()), this, SLOT(importCSV()),  Qt::UniqueConnection);

    }


    connect(flightChart, &FlightChart::cursorPosChanged, // показник координат
            this, [this](double x, double y, bool inside){
                if (!inside || std::isnan(x) || std::isnan(y)) {
                    coordLabel->setText("x: —    y: —");
                    return;
                }
                coordLabel->setText(QString("x: %1    y: %2")
                                        .arg(x, 0, 'f', 2)
                                        .arg(y, 0, 'f', 2));
            });

    connect(flightChart, &FlightChart::rightClickInDragMode, // настикання правою кнопкою мишкой
            this, &ChartPanelWidget::rightClickDrag);
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
void ChartPanelWidget::generateTestDataCount(int count)
{
    if (count <= 0) return;

    const bool timerWasRunning = (testTimer && testTimer->isActive());
    if (timerWasRunning) testTimer->stop();

    if (flightChart->getPlot()->graphCount() == 0) {
        onCheckboxChanged();
    }

    auto rnd = [](double a, double b) {
        return a + (b - a) * QRandomGenerator::global()->generateDouble();
    };

    auto noise = [&](double amp) {
        return rnd(-amp, amp);
    };

    constexpr double dt = 0.001;          // 1 ms
    constexpr double twoPi = 2.0 * M_PI;

    // Початкові координати
    double x = 0.0;
    double y = 1000.0;   // висота, м
    double z = 0.0;

    // Індивідуальні фази, щоб графіки не були однакові
    const double phase1 = rnd(0.0, twoPi);
    const double phase2 = rnd(0.0, twoPi);
    const double phase3 = rnd(0.0, twoPi);

    for (int i = 0; i < count; ++i) {
        parametrs p{};

        const double t = i * dt;

        // ==========================
        // Швидкість
        // ==========================
        p.time = t;

        p.v_ground =
            230.0
            + 12.0 * std::sin(twoPi * 0.15 * t + phase1)
            + 4.0  * std::sin(twoPi * 1.20 * t + phase2)
            + noise(0.8);

        p.vx = p.v_ground * std::cos(qDegreesToRadians(3.0)) + noise(0.5);
        p.vy =
            2.0 * std::sin(twoPi * 0.25 * t + phase2)
            + 0.5 * std::sin(twoPi * 2.00 * t)
            + noise(0.2);

        p.vz =
            4.0 * std::sin(twoPi * 0.18 * t + phase3)
            + noise(0.3);

        // ==========================
        // Кутові швидкості, рад/с
        // ==========================
        p.wx = qDegreesToRadians(
            1.5 * std::sin(twoPi * 0.9 * t + phase1) + noise(0.1)
            );

        p.wy = qDegreesToRadians(
            1.0 * std::sin(twoPi * 0.7 * t + phase2) + noise(0.1)
            );

        p.wz = qDegreesToRadians(
            2.0 * std::sin(twoPi * 0.4 * t + phase3) + noise(0.15)
            );

        p.angleWx = qRadiansToDegrees(p.wx);
        p.angleWy = qRadiansToDegrees(p.wy);
        p.angleWz = qRadiansToDegrees(p.wz);

        // ==========================
        // Кути орієнтації
        // ==========================
        p.teta = qDegreesToRadians(
            -5.0
            + 3.0 * std::sin(twoPi * 0.08 * t + phase1)
            + noise(0.05)
            );

        p.gamma = qDegreesToRadians(
            20.0 * std::sin(twoPi * 0.12 * t + phase2)
            + noise(0.15)
            );

        p.psi = qDegreesToRadians(
            std::fmod(90.0 + 4.0 * t + 8.0 * std::sin(twoPi * 0.03 * t), 360.0)
            );

        // ==========================
        // Аеродинамічні кути
        // ==========================
        p.alfa = qDegreesToRadians(
            4.0
            + 2.0 * std::sin(twoPi * 0.35 * t + phase1)
            + noise(0.1)
            );

        p.beta = qDegreesToRadians(
            1.5 * std::sin(twoPi * 0.45 * t + phase2)
            + noise(0.08)
            );

        // ==========================
        // Mach і висота
        // ==========================
        p.machNumber =
            0.72
            + 0.04 * std::sin(twoPi * 0.10 * t + phase3)
            + noise(0.003);

        p.altitude_wgs =
            y
            + 80.0 * std::sin(twoPi * 0.02 * t + phase1)
            + 10.0 * std::sin(twoPi * 0.25 * t)
            + noise(1.0);

        // ==========================
        // Перевантаження
        // ==========================
        p.nx =
            0.15 * std::sin(twoPi * 0.6 * t + phase1)
            + noise(0.01);

        p.ny =
            0.10 * std::sin(twoPi * 0.5 * t + phase2)
            + noise(0.01);

        p.nz =
            1.0
            + 0.35 * std::sin(twoPi * 0.35 * t + phase3)
            + noise(0.02);

        // ==========================
        // Моменти
        // ==========================
        p.mx =
            40.0 * std::sin(twoPi * 0.7 * t + phase1)
            + 10.0 * std::sin(twoPi * 2.5 * t)
            + noise(3.0);

        p.my =
            35.0 * std::sin(twoPi * 0.5 * t + phase2)
            + noise(3.0);

        p.mz =
            50.0 * std::sin(twoPi * 0.4 * t + phase3)
            + noise(4.0);

        // ==========================
        // Канали керування
        // ==========================
        p.deltaChannel1 =
            5.0 * std::sin(twoPi * 0.6 * t + phase1)
            + noise(0.2);

        p.deltaChannel2 =
            4.0 * std::sin(twoPi * 0.45 * t + phase2)
            + noise(0.2);

        p.deltaElerons =
            7.0 * std::sin(twoPi * 0.8 * t + phase3)
            + noise(0.25);

        // ==========================
        // Координати
        // ==========================
        x += p.vx * dt;
        y += p.vy * dt;
        z += p.vz * dt;

        p.xg = x;
        p.yg = y;
        p.zg = z;

        // ==========================
        // Маса і тяга
        // ==========================
        p.mass =
            600.0
            - 0.02 * t
            + noise(0.02);

        p.trust =
            2500.0
            + 250.0 * std::sin(twoPi * 0.12 * t + phase1)
            + noise(20.0);

        flightChart->appendDataChart(p);
    }

    flightChart->getPlot()->replot(QCustomPlot::rpQueuedReplot);
}
// void ChartPanelWidget::generateTestDataCount(int count)
// {
//     if (count <= 0) return;

//     // Якщо твій тестовий таймер крутиться — зупинимо, щоб не мішався
//     const bool timerWasRunning = (testTimer && testTimer->isActive());
//     if (timerWasRunning) testTimer->stop();

//     // Якщо графіки ще не створені (жоден чекбокс не вибраний) — увімкнемо дефолтні
//     if (flightChart->getPlot()->graphCount() == 0) {
//         onCheckboxChanged();
//     }

//     auto rnd = [](double a, double b){
//         return a + (b - a) * QRandomGenerator::global()->generateDouble();
//     };

//     // Генеруємо і додаємо БЕЗ проміжних перепобудов
//     for (int i = 0; i < count; ++i) {
//         parametrs p{};

//         p.time =i * 0.001;
//         p.v_ground    = rnd(200.0, 250.0);
//         p.vx          = p.v_ground + rnd(-2.0, 2.0);
//         p.vy          = rnd(-5.0, 5.0);
//         p.vz          = rnd(-2.0, 2.0);

//         p.wx          = qDegreesToRadians(rnd(-2.0, 2.0));
//         p.wy          = qDegreesToRadians(rnd(-2.0, 2.0));
//         p.wz          = qDegreesToRadians(rnd(-2.0, 2.0));

//         p.angleWx     = rnd(-1.0, 1.0);
//         p.angleWy     = rnd(-1.0, 1.0);
//         p.angleWz     = rnd(-1.0, 1.0);

//         p.teta        = qDegreesToRadians(rnd(-10.0, 10.0));
//         p.gamma       = qDegreesToRadians(rnd(-30.0, 30.0));
//         p.psi         = qDegreesToRadians(rnd(0.0, 360.0));

//         p.alfa        = qDegreesToRadians(rnd(-2.0, 8.0));
//         p.beta        = qDegreesToRadians(rnd(-3.0, 3.0));

//         p.machNumber  = rnd(0.6, 0.9);
//         p.altitude_wgs= rnd(100.0, 200.0);

//         p.nx          = rnd(-0.2, 0.2);
//         p.ny          = rnd(-0.2, 0.2);
//         p.nz          = 1.0 + rnd(-0.05, 0.05);

//         p.mx          = rnd(-100.0, 100.0);
//         p.my          = rnd(-100.0, 100.0);
//         p.mz          = rnd(-100.0, 100.0);

//         p.deltaChannel1 = rnd(-5.0, 5.0);
//         p.deltaChannel2 = rnd(-5.0, 5.0);
//         p.deltaElerons  = rnd(-5.0, 5.0);

//         // координати/масо-тяга — як хочеш
//         p.xg += rnd(-3.0, 3.0);
//         p.yg += rnd(-3.0, 3.0);
//         p.zg += rnd(-1.0, 1.0);
//         p.mass  = rnd(500.0, 600.0);
//         p.trust = rnd(2000.0, 3000.0);

//         // Додаємо прямо у FlightChart, БЕЗ scheduleReplot() з Panel
//         flightChart->appendDataChart(p);
//     }

//     // Один фінальний реплот
//     flightChart->getPlot()->replot(QCustomPlot::rpQueuedReplot);
// }


QStringList ChartPanelWidget::selectedKeys() const
{
    QStringList keys;
    for (auto it = checkboxes.constBegin(); it != checkboxes.constEnd(); ++it)
        if (it.value() && it.value()->isChecked())
            keys << it.key();
    return keys;
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

void ChartPanelWidget::modeChange(QString mode)
{
    if (mode == "drag") {
        flightChart->setRulerMode(false);
        flightChart->getPlot()->setSelectionRectMode(QCP::srmNone);
        flightChart->getPlot()->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom | QCP::iSelectAxes | QCP::iSelectPlottables);
    } else if (mode == "rect") {
        flightChart->setRulerMode(false);
        flightChart->getPlot()->setSelectionRectMode(QCP::srmZoom);
        flightChart->getPlot()->setInteractions(QCP::iRangeZoom | QCP::iSelectPlottables | QCP::iSelectAxes);
    } else if (mode == "ruler") {
        flightChart->setRulerMode(true);
        // дозволяємо скрол/зум, але без selectionRect:
        flightChart->getPlot()->setSelectionRectMode(QCP::srmNone);
        flightChart->getPlot()->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom);
    } else {
        flightChart->setRulerMode(false);
        flightChart->getPlot()->setSelectionRectMode(QCP::srmNone);
        flightChart->getPlot()->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom | QCP::iSelectAxes | QCP::iSelectPlottables);
    }
}
void ChartPanelWidget::liveButt(bool mode, int step)
{
    flightChart->setLiveModeEnabled(mode, step);
    flightChart->getPlot()->replot(QCustomPlot::rpQueuedReplot);
}

void ChartPanelWidget::deleteData()
{
    flightChart->clearData();
}

void ChartPanelWidget::setupUi()
{
    // ===== ГОЛОВНИЙ ГОРИЗОНТАЛЬНИЙ ЛЕЙАУТ (ЛІВО: чекбокси, ПРАВО: панель+графік) =====
    auto *rootRow = new QHBoxLayout(this);
    rootRow->setContentsMargins(0, 0, 0, 0);
    rootRow->setSpacing(5);

    // ------------------ ЛІВА ПАНЕЛЬ (СКРОЛ ЧЕКБОКСІВ) ------------------
    QWidget *checkboxContent = new QWidget;
    auto *checkboxLayout = new QVBoxLayout(checkboxContent);
    checkboxLayout->setAlignment(Qt::AlignTop);
    checkboxLayout->setSpacing(2);
    checkboxLayout->setContentsMargins(6, 6, 6, 6);

    // застосовуємо стилі до всього віджета (раз, а не в циклі)
    this->setStyleSheet(checkboxQss());

    for (const auto &f : PARAM_FIELDS()) {
        QCheckBox *cb = new QCheckBox(f.label);
        cb->setChecked(f.defaultChecked);

        // робимо чекбокси "у всю довжину" (тобто на всю доступну ширину панелі)
        cb->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
        // cb->setWordWrap(true); // якщо підписи довгі — переносимо

        checkboxes[f.key] = cb;
        checkboxLayout->addWidget(cb);
        // connect(cb, &QCheckBox::stateChanged, this, &ChartPanelWidget::onCheckboxChanged);
    }
    checkboxLayout->addStretch(); // щоб зверху було щільно, а низ заповнювався

    QScrollArea *scrollArea = new QScrollArea;
    scrollArea = new QScrollArea;
    scrollArea->setWidgetResizable(true);
    scrollArea->setWidget(checkboxContent);
    scrollArea->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);

    leftPanel = new QWidget;
    auto *leftPanelLayout = new QVBoxLayout(leftPanel);
    leftPanelLayout->setContentsMargins(0, 0, 0, 0);
    leftPanelLayout->setSpacing(0);
    leftPanelLayout->addWidget(scrollArea, 1);
    leftPanel->setMaximumWidth(300); // або за бажанням: setMinimumWidth(250), setMaximumWidth(350)

    // ------------------ ПРАВА КОЛОНКА: QML ПАНЕЛЬ + ГРАФІК ------------------
    rightCol = new QWidget;
    auto *rightColLayout = new QVBoxLayout(rightCol);
    rightColLayout->setContentsMargins(0, 0, 0, 0);
    rightColLayout->setSpacing(5);

    // ===== QML ПАНЕЛЬ (належить тільки правій колонці з графіком) =====
    m_qmlTopBar = new QQuickWidget(rightCol);
    m_qmlTopBar->setResizeMode(QQuickWidget::SizeRootObjectToView);
    m_qmlTopBar->setFixedHeight(60);          // фіксована висота

    windowColor = QApplication::palette().color(QPalette::Window);
    m_qmlTopBar->rootContext()->setContextProperty("appWindowColor", windowColor);

    m_qmlTopBar->rootContext()->setContextProperty("chartPanel", this); // для ретранслювання сигнала в qml
    m_qmlTopBar->setSource(QUrl(QStringLiteral("qrc:/Chart_panel.qml")));

    // ------------------ ГРАФІК ------------------
    flightChart = new FlightChart;

    // панель зверху над графіком:
    rightColLayout->addWidget(m_qmlTopBar);
    rightColLayout->addWidget(flightChart, 1);

    // // якщо треба панель знизу під графіком — поміняй порядок:
    // rightColLayout->addWidget(flightChart, 1);
    // rightColLayout->addWidget(m_qmlTopBar);

   // ------------------ ЗБІР ВСЬОГО ------------------
    // ------------------------------ кнопка для заїду чек боксів
    // Ручка-стрілка між панелями:
    collapseBtn = new QToolButton(this);
    collapseBtn->setAutoRaise(true);
    collapseBtn->setCheckable(true);
    collapseBtn->setFixedWidth(18);
    collapseBtn->setArrowType(Qt::LeftArrow); // спочатку показує «згорнути»
    collapseBtn->setCursor(Qt::PointingHandCursor);
    collapseBtn->setToolTip(tr("Згорнути/розгорнути панель"));

    // трошки стилю (тонка темна смуга + ховер)
    collapseBtn->setStyleSheet(R"(
    QToolButton {
        border: 2px solid #444;
        background: #ed0d1216;
        width: 24px;
        /*  radius: 5; */
        border-radius: 6px;
    }
    QToolButton:hover {
        border: 2px solid lightblue;   /* світло-блакитна рамка як у чекбоксів */
        background: #ed0d1216;         /* фон залишається темним */
    }
    QToolButton:checked {
        border: 2px solid lightblue;   /* коли кнопка "натиснута" — теж підсвічується */
        background: #ed0d1216;
    }
)");


    // collapseBtn->setStyleSheet(R"(
    //     QToolButton {
    //         border: none;
    //         border-left: 1px solid #333;
    //         border-right: 1px solid #222;
    //         background: #1f1f1f;
    //     }
    //     QToolButton:hover {
    //         background: #262626;
    //     }
    // )");


    connect(collapseBtn, &QToolButton::clicked, this, &ChartPanelWidget::toggleLeftPanel);

    // Збірка (ручка між лівою та правою частиною):
    rootRow->addWidget(leftPanel);      // лівий стовпчик: чекбокси
    rootRow->addWidget(collapseBtn);    // ручка
    rootRow->addWidget(rightCol, 1);    // права частина


    // у setupUi(), одразу після створення правої колонки та flightChart:
    coordLabel = new QLabel("x: —    y: —", rightCol);
    coordLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    coordLabel->setMinimumHeight(22);
    coordLabel->setStyleSheet(
        "QLabel{color:white;padding:0 8px;font-family:'DejaVu Sans Mono',Consolas,monospace;"
        "font-size:14px;background:transparent;}"
        );
    // ------------------------------ кнопка для заїду чек боксів



    // ДОДАЙ ПІСЛЯ графіка:
    rightColLayout->addWidget(flightChart, 1);
    rightColLayout->addWidget(coordLabel);





}

QString ChartPanelWidget::checkboxQss() const
{
    return QString::fromUtf8(R"(
        QCheckBox {
            spacing: 8px;
            font-size: 16px;
            color: #ddd;
        }
        QCheckBox:hover {
            color: white;
        }
        QCheckBox::indicator {
            width: 24px;
            height: 24px;
            border-radius: 6px;
            border: 2px solid #444;
            background: #ed0d1216;
        }
        QCheckBox::indicator:hover {
            border: 2px solid lightblue;
        }
        QCheckBox::indicator:checked {
            image: url(:/Icon/checkmark.svg);
            /* фон не заливаємо */
        }
    )");
}


void ChartPanelWidget::exportToCsv(const QString& pathOrUrl)
{
    // 1) Конвертуємо URL -> локальний шлях і додаємо .csv, якщо нема
    QString localPath = QUrl(pathOrUrl).isValid()
                            ? QUrl(pathOrUrl).toLocalFile()
                            : pathOrUrl;
    if (localPath.isEmpty())
        return;

    if (QFileInfo(localPath).suffix().isEmpty())
        localPath += ".csv";

    // 2) Збираємо дані в GUI-потоці (доступ до чекбоксів/пам’яті тут безпечно)
    const QStringList keys = selectedKeys();
    const auto dataCopy    = flightChart->rawData();   // КОПІЯ (auto робить копію)

    // 3) Готуємо воркер і потік
    auto *exporter = new Csv_worker;   // QObject із сигналом exportFinished(...)
    auto *thread   = new QThread;

    exporter->moveToThread(thread);

    // Роботу запускаємо у воркер-потоці (ресівер = exporter => код виконається у thread)
    connect(thread, &QThread::started, exporter,
            [exporter, dataCopy, keys, localPath]() {
                const bool ok = exporter->exportSelected(dataCopy, keys, localPath);
                emit exporter->exportFinished(ok, localPath);
            });

    // Коректне завершення життєвого циклу
    connect(exporter, &Csv_worker::exportFinished, thread, &QThread::quit);
    connect(thread, &QThread::finished, exporter, &QObject::deleteLater);
    connect(thread, &QThread::finished, thread,   &QObject::deleteLater);

    // (опційно) лог/повідомлення
    connect(exporter, &Csv_worker::exportFinished, this,
            [](bool ok, const QString& p){
                qDebug() << "CSV export finished:" << ok << p;
            });

    thread->start();
}

void ChartPanelWidget::exportImage(const QString &pathOrUrl)
{

        // URL -> локальний шлях, додати суфікс якщо немає
        QString path = QUrl(pathOrUrl).isValid() ? QUrl(pathOrUrl).toLocalFile() : pathOrUrl;
        if (path.isEmpty()) return;

        // визначимо формат із розширення
        QString ext = QFileInfo(path).suffix().toLower();
        if (ext.isEmpty()) { ext = "png"; path += ".png"; }

        // підвищимо чіткість на HiDPI (не обов’язково)
        const double dpr = devicePixelRatioF();
        const int    w   = int(flightChart->getPlot()->width()  * dpr);
        const int    h   = int(flightChart->getPlot()->height() * dpr);

        // на всяк випадок — актуалізуємо малюнок
        flightChart->getPlot()->replot();

        bool ok = false;
        if (ext == "png")
            ok = flightChart->getPlot()->savePng(path, w, h, 2.0 /*dpiScale*/);
        else if (ext == "jpg" || ext == "jpeg")
            ok = flightChart->getPlot()->saveJpg(path, w, h, 90 /*quality*/);
        else if (ext == "bmp")
            ok = flightChart->getPlot()->saveBmp(path, w, h);
        else if (ext == "pdf")
            ok = flightChart->getPlot()->savePdf(path); // PDF сам векторний, без w/h
        else {
            // незнайомий формат — спробуємо як PNG
            ok = flightChart->getPlot()->savePng(path + ".png", w, h, 2.0);
        }

        qDebug() << "Image export:" << ok << path;

}
void ChartPanelWidget::toggleLeftPanel()
{
    leftCollapsed = !leftCollapsed;

    // На всякий — запам’ятаємо актуальну ширину як «розгорнуту», якщо користувач міняв її вручну
    if (!leftCollapsed) {
        // відкриваємо назад — якщо раптом ширина була 0, повернемо до last known
        if (leftExpandedWidth < 120) leftExpandedWidth = 300;
    } else {
        // коли згортаємо — збережемо поточну ширину як «розгорнуту» на майбутнє
        leftExpandedWidth = leftPanel->width();
        if (leftExpandedWidth < 120) leftExpandedWidth = 300;
    }

    // Плавна анімація властивості maximumWidth (працює стабільно)
    auto *anim = new QPropertyAnimation(leftPanel, "maximumWidth", this);
    anim->setDuration(180);
    anim->setEasingCurve(QEasingCurve::InOutCubic);
    anim->setStartValue(leftPanel->maximumWidth());

    if (leftCollapsed) {
        leftPanel->setMinimumWidth(0);
        anim->setEndValue(0);
        collapseBtn->setArrowType(Qt::RightArrow); // вказує, що можна розгорнути
    } else {
        anim->setEndValue(leftExpandedWidth);
        collapseBtn->setArrowType(Qt::LeftArrow);  // вказує, що можна згорнути
    }

    anim->start(QAbstractAnimation::DeleteWhenStopped);
}

void ChartPanelWidget::autoZoomX()
{
    QCustomPlot* plot = flightChart ? flightChart->getPlot() : nullptr;
    if (!plot || plot->graphCount() == 0)
        return;

    // Знайдемо глобальні min/max X серед усіх графіків (видимих)
    double minX =  std::numeric_limits<double>::infinity();
    double maxX = -std::numeric_limits<double>::infinity();

    for (int i = 0; i < plot->graphCount(); ++i) {
        QCPGraph* g = plot->graph(i);
        if (!g || !g->visible()) continue;
        const auto *container = g->data().data(); // QSharedPointer<QCPGraphDataContainer>
        if (!container || container->isEmpty()) continue;

        // Контейнер відсортований за key
        auto itBegin = container->constBegin();
        auto itEnd   = container->constEnd();
        --itEnd; // останній елемент

        double firstKey = itBegin->key;
        double lastKey  = itEnd->key;

        if (firstKey < minX) minX = firstKey;
        if (lastKey  > maxX) maxX = lastKey;
    }

    if (!std::isfinite(minX) || !std::isfinite(maxX)) {
        // Немає даних — нічого масштабувати
        return;
    }

    // Додамо невеликий відступ (2%)
    double span = maxX - minX;
    if (span <= 0) span = 1.0;        // щоб не було нульового діапазону
    double pad = span * 0.02;

    // Якщо всі графіки сидять на ГОЛОВНІЙ осі xAxis:
    // plot->xAxis->setRange(minX - pad, maxX + pad);

    // Якщо графіки можуть бути на різних осях X — розтягнемо КОЖНУ їхню вісь окремо:
    QSet<QCPAxis*> touchedAxes;
    for (int i = 0; i < plot->graphCount(); ++i) {
        QCPGraph* g = plot->graph(i);
        if (!g || !g->visible()) continue;
        QCPAxis* xax = g->keyAxis();
        if (!xax || touchedAxes.contains(xax)) continue;
        xax->setRange(minX - pad, maxX + pad);
        touchedAxes.insert(xax);
    }

    plot->replot(); // краще негайно
}

void ChartPanelWidget::importCSV()
{
    QString filePath = QFileDialog::getOpenFileName(
        this,
        tr("Виберіть CSV файл"),
        QDir::currentPath(),
        tr("CSV Files (*.csv)")
        );

    if (filePath.isEmpty()) {
        qWarning() << "importCSV: файл не вибрано";
        return;
    }

    std::vector<parametrs> d;

    Csv_worker csv;

    if (!csv.importCsv(d, filePath)) {
        qWarning() << "importCSV: не вдалося завантажити CSV";
        return;
    }

    qDebug() << "CSV rows:" << d.size();

    setData(d);
}
