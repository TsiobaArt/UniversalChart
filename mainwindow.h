#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTimer>
#include <QRandomGenerator>
#include <QtMath>
#include "Chart/parametrs.h"
#include "Chart/chartpanelwidget.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();


private:
    ChartPanelWidget *panel;  // ✅ панель з графіком + чекбоксами
    QTimer *testTimer = nullptr;
};

#endif // MAINWINDOW_H
