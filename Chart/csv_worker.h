#ifndef CSV_WORKER_H
#define CSV_WORKER_H

#include <QObject>
#include <QString>
#include <QStringList>
#include <vector>
#include "Fields_parametrs.h"   // parametrs

class Csv_worker : public QObject
{
    Q_OBJECT
public:
    explicit Csv_worker(QObject *parent = nullptr);
    // Основний метод
    bool exportSelected(const std::vector<parametrs>& data,
                        const QStringList& keys,
                        const QString& filePath,
                        QChar sep = ',');


    bool exportAll(const std::vector<parametrs>& data,
                   const QString& filePath,
                   QChar sep = ',');

    bool importCsv(std::vector<parametrs>& outData,
                   const QString& filePath,
                   QChar sep = ',');

signals:
    void exportFinished(bool success, QString path);
    void importFinished(bool success, QString path);

private:
    struct Column {
        QString header;
        std::function<double(const parametrs&)> getter;
    };

    struct ImportColumn {
        int csvIndex = -1;
        std::function<void(parametrs&, double)> setter;
    };

    bool exportByKeys(const std::vector<parametrs>& data,
                      const QStringList& keys,
                      const QString& filePath,
                      QChar sep);
};


#endif // CSV_WORKER_H
