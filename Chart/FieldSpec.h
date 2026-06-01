#pragma once
#include <functional>
#include <QString>
#include <QColor>

template <class T>
struct FieldSpec {
    QString key;                      // машинне ім’я (стабільне)
    QString label;                    // підпис у легенді/чекбоксі
    QString unit;                     // одиниці (опційно)
    QColor  color;                    // колір лінії
    bool    defaultChecked = false;   // чи вмикати чекбокс за замовчуванням

    std::function<double(const T&)> getter; // функція діставання значення
    std::function<void(T&, double)> setter;

};
