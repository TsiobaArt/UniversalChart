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
#include <qapplication.h>

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
    QColor windowColor ;

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


// ChartPanelWidget::ChartPanelWidget(QWidget *parent)
//     : QWidget(parent)
// {
//     // ===== ГОЛОВНИЙ ВЕРТИКАЛЬНИЙ ЛЕЙАУТ =====
//     auto *rootLayout = new QVBoxLayout(this);
//     rootLayout->setContentsMargins(0, 0, 0, 0);
//     rootLayout->setSpacing(5);

//     // ===== ВЕРХНЯ QML ПАНЕЛЬ =====
//     m_qmlTopBar = new QQuickWidget(this);
//     m_qmlTopBar->setResizeMode(QQuickWidget::SizeRootObjectToView);
//     m_qmlTopBar->setFixedHeight(50);          // <-- фіксована висота 50
//     rootLayout->addWidget(m_qmlTopBar);       // займає всю ширину віджета


//     // ===== НИЖНІЙ РЯД (ТВІЙ ІСНУЮЧИЙ КОД) =====
//     QHBoxLayout *mainLayout = new QHBoxLayout();
//     mainLayout->setContentsMargins(0, 0, 0, 0);
//     mainLayout->setSpacing(5);
//     rootLayout->addLayout(mainLayout, 1);     // тягнеться на весь залишок

//     // ------------------ ГРАФІК ------------------
//     flightChart = new FlightChart;

//     // ------------------ СКРОЛ ЧЕКБОКСІВ ------------------
//     QWidget *checkboxContent = new QWidget;
//     QVBoxLayout *checkboxLayout = new QVBoxLayout(checkboxContent);
//     checkboxLayout->setAlignment(Qt::AlignTop);
//     checkboxLayout->setSpacing(2);
//     checkboxLayout->setContentsMargins(2, 2, 2, 2);

//     for (const auto &f : PARAM_FIELDS()) {
//         QCheckBox *cb = new QCheckBox(f.label);
//         cb->setChecked(f.defaultChecked);
//         checkboxes[f.key] = cb;
//         checkboxLayout->addWidget(cb);
//         connect(cb, &QCheckBox::stateChanged, this, &ChartPanelWidget::onCheckboxChanged);
//     }

//     QScrollArea *scrollArea = new QScrollArea;
//     scrollArea->setWidgetResizable(true);
//     scrollArea->setWidget(checkboxContent);
//     scrollArea->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

//     // ------------------ ЛІВА ПАНЕЛЬ ------------------
//     QVBoxLayout *leftPanelLayout = new QVBoxLayout;
//     leftPanelLayout->setContentsMargins(2, 2, 2, 2);
//     leftPanelLayout->setSpacing(5);
//     leftPanelLayout->addWidget(scrollArea, 1);

//     QWidget *leftPanel = new QWidget;
//     leftPanel->setLayout(leftPanelLayout);
//     leftPanel->setMaximumWidth(300);

//     // ------------------ ЗБІР ВСЬОГО ------------------
//     mainLayout->addWidget(leftPanel);
//     mainLayout->addWidget(flightChart, 1);
//     // 1) Встановлюємо джерело QML
//     m_qmlTopBar->setSource(QUrl(QStringLiteral("qrc:/Chart_panel.qml")));

//     // --------------------------  test Даних
//     testTimer = new QTimer(this);
//     connect(testTimer, &QTimer::timeout, this, &ChartPanelWidget::generateTestData);
//     testTimer->start(1); // 100 Гц
//     // --------------------------  test Даних


//     // QObject *toolbarRoot = m_qmlTopBar->rootObject();
//     // if (!toolbarRoot)
//     //     return;

//     // connect(toolbarRoot, SIGNAL(themeToggle()),
//     //         this, SLOT(onTopBarThemeToggle()), Qt::UniqueConnection);
//     // connect(toolbarRoot, SIGNAL(autoZoom()),
//     //         this, SLOT(onTopBarAutoZoom()), Qt::UniqueConnection);
//     // connect(toolbarRoot, SIGNAL(liveToggle()),
//     //         this, SLOT(onTopBarLiveToggle()), Qt::UniqueConnection);
//     // connect(toolbarRoot, SIGNAL(modeChanged(QString)),
//     //         this, SLOT(onTopBarModeChanged(QString)), Qt::UniqueConnection);
//     // connect(toolbarRoot, SIGNAL(clearRequested()),
//     //         this, SLOT(onTopBarClearRequested()), Qt::UniqueConnection);
//      mLastReplot.start();  // Timer який оновлює сам графік всередині в незалежності приходсять дані чи ні
// }
