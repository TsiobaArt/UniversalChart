#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include <cmath> // для fmod

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    panel = new ChartPanelWidget(this);   // ✅
    setCentralWidget(panel);              // ✅

    testTimer = new QTimer(this);
    connect(testTimer, &QTimer::timeout, this, &MainWindow::generateTestData);
    testTimer->start(1); // 100 Гц
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::generateTestData()
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

    panel->appendDataStep(p);   // ✅ саме панель, не FlightChart

    t += dt;
}
