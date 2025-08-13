#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTimer>
#include <QRandomGenerator>
#include <QtMath>
#include "Chart/parametrs.h"
#include "Chart/chartpanelwidget.h"   // ✅ замість flightchart.h

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void generateTestData();

private:
    Ui::MainWindow *ui;
    ChartPanelWidget *panel;  // ✅ панель з графіком + чекбоксами
    QTimer *testTimer;
};

#endif // MAINWINDOW_H
