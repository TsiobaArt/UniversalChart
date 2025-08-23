#include <cmath> // для fmod
#include "mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)

{

    panel = new ChartPanelWidget(this);
    setCentralWidget(panel);
}

MainWindow::~MainWindow()
{
}

