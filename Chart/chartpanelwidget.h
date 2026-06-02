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
#include "csv_worker.h"
#include <QPropertyAnimation>
#include <qtoolbutton.h>

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
    void generateTestDataCount(int count);
    QColor windowColor ;

    QStringList selectedKeys() const;


public slots:
   void  exportToCsv(const QString& path);
   void  exportImage(const QString& pathOrUrl);



private slots:
    void onCheckboxChanged();
    void updatePlot();
    void clearSelection();
    void toggleTheme();
    void autoZoom();
    void modeChange(QString mode);
    void liveButt(bool mode, int step);
    void deleteData();
    void toggleLeftPanel();
    void autoZoomX();
    void importCSV();

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
    int mMinReplotIntervalMs = 100; //  16 мс ~60 FPS    20 мс ~50 FPS

    // Live кнопка
    QPushButton *liveBtn = nullptr;
    QTimer *testTimer = nullptr;

    QLabel *coordLabel = nullptr;

    QWidget* leftPanel = nullptr;        // уже є у тебе
    QWidget* rightCol  = nullptr;        // уже є у тебе
    QToolButton* collapseBtn = nullptr;  // нове: ручка-стрілка
    bool leftCollapsed = false;
    int  leftExpandedWidth = 300;        // ширина, коли панель відкрита

};
