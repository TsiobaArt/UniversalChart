#ifndef CSVEXPORTER_H
#define CSVEXPORTER_H

#include <QObject>

#pragma once
#include <QString>
#include <QStringList>
#include <vector>
#include "Fields_parametrs.h"   // parametrs

class CsvExporter : public QObject
{
    Q_OBJECT
public:
    explicit CsvExporter(QObject* parent = nullptr);

    // Основний метод
    bool exportSelected(const std::vector<parametrs>& data,
                        const QStringList& keys,
                        const QString& filePath,
                        QChar sep = ',');

signals:
    void exportFinished(bool success, QString path);

private:
    struct Column {
        QString header;
        std::function<double(const parametrs&)> getter;
    };
};


#endif // CSVEXPORTER_H
