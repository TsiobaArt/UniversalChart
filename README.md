# Universal Flight Chart (QCustomPlot + Field Registry)

Цей архів містить готову реалізацію універсального графіка з **реєстром полів**.
Ви просто додаєте/вимикаєте поля в одному місці (``Fields_parametrs.h``), і це
автоматично відображається в чекбоксах, легенді та на графіку — **без if-else**.

## Склад
- `include/FieldSpec.h` — універсальний опис поля (key/label/unit/color/getter).
- `include/Fields_parametrs.h` — реєстр полів для структури `parametrs`.
- `include/Fields_lookup.h` — пошук опису поля за ключем.
- `include/parametrs.h` — ваша структура даних (можете підставити свою версію).
- `include/flightchart.h`, `src/FlightChart.cpp` — віджет графіка.
- `include/chartpanelwidget.h`, `src/ChartPanelWidget.cpp` — панель з інструментами та чекбоксами.

## Швидкий старт
1. Додайте папки `include/` і `src/` у ваш проєкт (CMake/qmake).
2. Переконайтесь, що у вас підключено `QCustomPlot` та модулі Qt: Widgets, Core, Gui.
3. Використовуйте `ChartPanelWidget` як готовий віджет:
   ```cpp
   ChartPanelWidget* panel = new ChartPanelWidget(parent);
   panel->setData(dataVector); // std::vector<parametrs>
   ```
4. Для покрокового оновлення потоку:
   ```cpp
   parametrs p = ...;
   panel->appendDataStep(p);
   ```

## Додати нове поле
Відкрийте `include/Fields_parametrs.h` і додайте рядок у вектор `PARAM_FIELDS()`:
```cpp
{"myFieldKey", "Мій підпис", "од.", niceColor(), false, [](auto& d){ return d.myValue; }},
```
- `key` — машинне ім’я (стабільне).
- `label` — підпис, який бачить користувач (чекбокси/легенда).
- `unit` — одиниці вимірювання (необов’язково).
- `defaultChecked` — чи вмикати за замовчуванням.
- `getter` — як витягнути число з `parametrs`.

## Примітки
- В легенді відображається `label` + `[unit]`.
- У чекбоксах теж використовується `label`, а у внутрішній логіці — `key`.
- Немає жодних довгих `if-else`: увесь доступ іде через `getter`.
- Якщо даних дуже багато, вмикайте "ковзне вікно" (`removeDataBefore`) у `appendDataChart`.

Успіхів! 🙌
