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
    void modeChange(QString mode);
    void liveButt(bool mode, int step);

signals:
    void rightClickDrag();

private:
    // ===== GUI =====
    void setupUi();                             // <-- уся побудова інтерфейсу тут
    QString checkboxQss() const;                // qss для чекбоксів

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

    QLabel *coordLabel = nullptr;

};
// ChartPanelWidget::ChartPanelWidget(QWidget *parent)  //  ОСТАННЯ ВЕРІСЯ ПРИБРАТИ  setupUi і вставити на міця
//     : QWidget(parent)
// {
//     setMinimumSize(800, 600);   // мінімальний розмір вікна (ширина × висота)
//     // ===== ГОЛОВНИЙ ГОРИЗОНТАЛЬНИЙ ЛЕЙАУТ (ЛІВО: чекбокси, ПРАВО: панель+графік) =====
//     auto *rootRow = new QHBoxLayout(this);
//     rootRow->setContentsMargins(0, 0, 0, 0);
//     rootRow->setSpacing(5);

//     // ------------------ ЛІВА ПАНЕЛЬ (СКРОЛ ЧЕКБОКСІВ) ------------------
//     QWidget *checkboxContent = new QWidget;
//     QVBoxLayout *checkboxLayout = new QVBoxLayout(checkboxContent);
//     checkboxLayout->setAlignment(Qt::AlignTop);
//     checkboxLayout->setSpacing(2);
//     checkboxLayout->setContentsMargins(6, 6, 6, 6);

//     for (const auto &f : PARAM_FIELDS()) {
//         QCheckBox *cb = new QCheckBox(f.label);

//         this->setStyleSheet(R"(
//     QCheckBox {
//         spacing: 8px;
//         font-size: 16px;
//         color: #ddd;
//     }
//         QCheckBox::hover {
//         color: white
//     }
//     QCheckBox::indicator {
//         width: 24px;
//         height: 24px;
//         border-radius: 6px;
//         border: 2px solid #444;
//         background: #ed0d1216;
//     }
//     QCheckBox::indicator:hover {
//         border: 2px solid lightblue;
//     }
//     QCheckBox::indicator:checked {
//         image: url(:/Icon/checkmark.svg);   /* можна svg галочку */
//     }
// )");

//         cb->setChecked(f.defaultChecked);
//         cb->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
//         checkboxes[f.key] = cb;
//         checkboxLayout->addWidget(cb);
//         connect(cb, &QCheckBox::stateChanged, this, &ChartPanelWidget::onCheckboxChanged);
//     }
//     checkboxLayout->addStretch(); // щоб зверху було щільно, а низ заповнювався

//     QScrollArea *scrollArea = new QScrollArea;
//     scrollArea->setWidgetResizable(true);
//     scrollArea->setWidget(checkboxContent);
//     scrollArea->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);

//     QWidget *leftPanel = new QWidget;
//     auto *leftPanelLayout = new QVBoxLayout(leftPanel);
//     leftPanelLayout->setContentsMargins(0, 0, 0, 0);
//     leftPanelLayout->setSpacing(0);
//     leftPanelLayout->addWidget(scrollArea, 1);
//     leftPanel->setMaximumWidth(300); // або за бажанням: setFixedWidth(280);

//     // ------------------ ПРАВА КОЛОНКА: ВЕРХНЯ QML ПАНЕЛЬ + ГРАФІК ------------------
//     QWidget *rightCol = new QWidget;
//     auto *rightColLayout = new QVBoxLayout(rightCol);
//     rightColLayout->setContentsMargins(0, 0, 0, 0);
//     rightColLayout->setSpacing(5);

//     // ===== ВЕРХНЯ QML ПАНЕЛЬ (належить тільки правій колонці з графіком) =====
//     m_qmlTopBar = new QQuickWidget(rightCol);
//     m_qmlTopBar->setResizeMode(QQuickWidget::SizeRootObjectToView);
//     m_qmlTopBar->setFixedHeight(60);          // <-- фіксована висота 50
//     windowColor =   QApplication::palette().color(QPalette::Window);
//     m_qmlTopBar->rootContext()->setContextProperty("appWindowColor", windowColor);
//     m_qmlTopBar->setSource(QUrl(QStringLiteral("qrc:/Chart_panel.qml")));

//     // ------------------ ГРАФІК ------------------
//     flightChart = new FlightChart;

//     rightColLayout->addWidget(m_qmlTopBar);   // панель тільки над графіком (у правій колонці)
//     rightColLayout->addWidget(flightChart, 1);
//     // rightColLayout->addWidget(flightChart, 1);   // графік займає весь простір
//     // rightColLayout->addWidget(m_qmlTopBar);      // панель піде внизу

//     // ------------------ ЗБІР ВСЬОГО ------------------
//     rootRow->addWidget(leftPanel);            // лівий стовпчик: чекбокси
//     rootRow->addWidget(rightCol, 1);          // правий стовпчик: панель+графік

//     // // --------------------------  test Даних
//     // testTimer = new QTimer(this);
//     // connect(testTimer, &QTimer::timeout, this, &ChartPanelWidget::generateTestData);
//     // testTimer->start(1); // 100 Гц
//     // // --------------------------  test Даних

//     mLastReplot.start();  // Timer який оновлює сам графік всередині в незалежності приходсять дані чи ні

// }



// ChartPanelWidget::ChartPanelWidget(QWidget *parent)
//     : QWidget(parent)
// {
//     setMinimumSize(800, 600);   // мінімальний розмір вікна (ширина × висота)

//     // ===== ГОЛОВНИЙ ВЕРТИКАЛЬНИЙ ЛЕЙАУТ (ПАНЕЛЬ КНОПОК + НИЖНІЙ РЯД) =====
//     auto *rootCol = new QVBoxLayout(this);
//     rootCol->setContentsMargins(0, 0, 0, 0);
//     rootCol->setSpacing(5);

//     // ===== ВЕРХНЯ QML ПАНЕЛЬ (на всю ширину) =====
//     m_qmlTopBar = new QQuickWidget(this);
//     m_qmlTopBar->setResizeMode(QQuickWidget::SizeRootObjectToView);
//     m_qmlTopBar->setFixedHeight(60);                  // фіксована висота панелі
//     m_qmlTopBar->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

//     // фон панелі = фон вікна
//     windowColor = QApplication::palette().color(QPalette::Window);
//     m_qmlTopBar->setClearColor(Qt::transparent);
//     m_qmlTopBar->rootContext()->setContextProperty("appWindowColor", windowColor);

//     m_qmlTopBar->setSource(QUrl(QStringLiteral("qrc:/Chart_panel.qml")));

//     // додаємо панель у верх rootCol (вона тепер над усім і на всю ширину)
//     rootCol->addWidget(m_qmlTopBar);

//     // ===== НИЖНІЙ РЯД (ЛІВО: чекбокси, ПРАВО: графік) =====
//     auto *rootRow = new QHBoxLayout();                // тепер rootRow всередині rootCol
//     rootRow->setContentsMargins(0, 0, 0, 0);
//     rootRow->setSpacing(5);

//     // ------------------ ЛІВА ПАНЕЛЬ (СКРОЛ ЧЕКБОКСІВ) ------------------
//     QWidget *checkboxContent = new QWidget;
//     auto *checkboxLayout = new QVBoxLayout(checkboxContent);
//     checkboxLayout->setAlignment(Qt::AlignTop);
//     checkboxLayout->setSpacing(2);
//     checkboxLayout->setContentsMargins(6, 6, 6, 6);

//     for (const auto &f : PARAM_FIELDS()) {
//         QCheckBox *cb = new QCheckBox(f.label);
//                 this->setStyleSheet(R"(
//             QCheckBox {
//                 spacing: 8px;
//                 font-size: 16px;
//                 color: #ddd;
//             }
//                 QCheckBox::hover {
//                 color: white
//             }
//             QCheckBox::indicator {
//                 width: 24px;
//                 height: 24px;
//                 border-radius: 6px;
//                 border: 2px solid #444;
//                 background: #ed0d1216;
//             }
//             QCheckBox::indicator:hover {
//                 border: 2px solid lightblue;
//             }
//             QCheckBox::indicator:checked {
//                 image: url(:/Icon/checkmark.svg);   /* можна svg галочку */
//             }
//         )");
//         cb->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
//         cb->setChecked(f.defaultChecked);
//         checkboxes[f.key] = cb;
//         checkboxLayout->addWidget(cb);
//         connect(cb, &QCheckBox::stateChanged, this, &ChartPanelWidget::onCheckboxChanged);
//     }
//     checkboxLayout->addStretch();

//     auto *scrollArea = new QScrollArea;
//     scrollArea->setWidgetResizable(true);
//     scrollArea->setWidget(checkboxContent);
//     scrollArea->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);

//     QWidget *leftPanel = new QWidget;
//     auto *leftPanelLayout = new QVBoxLayout(leftPanel);
//     leftPanelLayout->setContentsMargins(0, 0, 0, 0);
//     leftPanelLayout->setSpacing(0);
//     leftPanelLayout->addWidget(scrollArea, 1);
//     leftPanel->setMinimumWidth(250);    // трішки більша ліва панель (за бажанням)
//     leftPanel->setMaximumWidth(350);    // або прибери ліміт, якщо не треба

//     // ------------------ ПРАВА КОЛОНКА: ГРАФІК ------------------
//     QWidget *rightCol = new QWidget;
//     auto *rightColLayout = new QVBoxLayout(rightCol);
//     rightColLayout->setContentsMargins(0, 0, 0, 0);
//     rightColLayout->setSpacing(5);

//     flightChart = new FlightChart;
//     rightColLayout->addWidget(flightChart, 1);

//     // ------------------ ЗБІР НИЖНЬОГО РЯДУ ------------------
//     rootRow->addWidget(leftPanel);
//     rootRow->addWidget(rightCol, 1);

//     // додали ряд у головний вертикальний лейаут
//     // rootCol->addLayout(rootRow, 1);
//     rootCol->addLayout(rootRow, 1);    // спочатку додаємо основний контент
//     // rootCol->addWidget(m_qmlTopBar);   // панель піде внизу, на всю ширину



//     mLastReplot.start();
// }


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
