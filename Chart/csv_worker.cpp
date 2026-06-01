#include "csv_worker.h"
#include "FieldSpec.h"
#include <fstream>
#include "Fields_lookup.h"
#include <iomanip>
#include <QDebug>
#include <qdir.h>

Csv_worker::Csv_worker(QObject *parent)
    : QObject{parent}
{}

bool Csv_worker::exportSelected(const std::vector<parametrs> &data, const QStringList &keys, const QString &filePath, QChar sep)
{
    std::vector<Column> cols;

    cols.push_back({"time", [](const parametrs& p){ return p.time; }});

    for (const auto& k : keys) {
        if (auto spec = findFieldByKey(k)) {
            if (spec->key == "time") continue;
            cols.push_back({ spec->key, spec->getter });
        }
    }

    for (const auto& k : keys) {
        if (auto spec = findFieldByKey(k)) {
            cols.push_back({ spec->key, spec->getter });
        }
    }


    if (cols.empty()) {
        emit exportFinished(false, filePath);
        return false;
    }

    std::ofstream file(filePath.toStdString(), std::ios::out | std::ios::trunc | std::ios::binary);
    if (!file.is_open()) {
        emit exportFinished(false, filePath);
        return false;
    }

    file.imbue(std::locale::classic());
    file << std::fixed << std::setprecision(6);

    const char csep = static_cast<char>(sep.unicode());

    auto writeCsvText = [&](const QString& s){
        const QByteArray utf8 = s.toUtf8();
        const bool needQuotes = utf8.contains(csep) || utf8.contains('"') ||
                                utf8.contains('\n')  || utf8.contains('\r');
        if (!needQuotes) { file.write(utf8.constData(), utf8.size()); return; }
        file.put('"');
        for (char ch : utf8) {
            if (ch == '"') file.put('"'); // escape подвійною лапкою
            file.put(ch);
        }
        file.put('"');
    };

    auto writeCsvNumber = [&](double v){
        if (std::isfinite(v)) file << v; // NaN/Inf -> порожнє поле
    };

    // --- Заголовок (одним текстом: "ψ (курс) [deg]" і т.д.) ---
    for (size_t i = 0; i < cols.size(); ++i) {
        if (i) file << csep;
        writeCsvText(cols[i].header);
    }
    file << '\n';

    // --- Дані ---
    for (const auto& p : data) {
        for (size_t i = 0; i < cols.size(); ++i) {
            if (i) file << csep;
            writeCsvNumber(cols[i].getter(p));
        }
        file << '\n';
    }

    file.close();
    emit exportFinished(true, filePath);
    return true;
}

bool Csv_worker::exportAll(const std::vector<parametrs>& data,
                           const QString& filePath,
                           QChar sep)
{
    QStringList keys;
    keys << "time";

    for (const auto& spec : PARAM_FIELDS()) {
        if (spec.key != "time")
            keys << spec.key;
    }

    return exportByKeys(data, keys, filePath, sep);
}

bool Csv_worker::importCsv(std::vector<parametrs>& outData,
                           const QString& filePath,
                           QChar sep)
{
    outData.clear();

    QFile file(filePath);

    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        emit importFinished(false, filePath);
        return false;
    }

    QTextStream in(&file);
    in.setEncoding(QStringConverter::Utf8);

    auto parseCsvLine = [&](const QString& line) {
        QStringList result;
        QString cell;
        bool inQuotes = false;

        for (int i = 0; i < line.size(); ++i) {
            const QChar ch = line[i];

            if (ch == '"') {
                if (inQuotes && i + 1 < line.size() && line[i + 1] == '"') {
                    cell += '"';
                    ++i;
                } else {
                    inQuotes = !inQuotes;
                }
            } else if (ch == sep && !inQuotes) {
                result << cell.trimmed();
                cell.clear();
            } else {
                cell += ch;
            }
        }

        result << cell.trimmed();
        return result;
    };

    if (in.atEnd()) {
        emit importFinished(false, filePath);
        return false;
    }

    const QString headerLine = in.readLine();
    const QStringList headers = parseCsvLine(headerLine);

    std::vector<ImportColumn> importColumns;
    int timeIndex = -1;

    for (int i = 0; i < headers.size(); ++i) {
        const QString key = headers[i].trimmed();

        if (key == "time") {
            timeIndex = i;
            continue;
        }

        if (auto spec = findFieldByKey(key)) {
            if (spec->setter) {
                importColumns.push_back({i, spec->setter});
            }
        }
    }

    while (!in.atEnd()) {
        const QString line = in.readLine().trimmed();

        if (line.isEmpty())
            continue;

        const QStringList cells = parseCsvLine(line);

        parametrs p{};

        if (timeIndex >= 0 && timeIndex < cells.size()) {
            bool ok = false;
            const double v = cells[timeIndex].toDouble(&ok);
            if (ok)
                p.time = v;
        }

        for (const auto& col : importColumns) {
            if (col.csvIndex < 0 || col.csvIndex >= cells.size())
                continue;

            bool ok = false;
            const double v = cells[col.csvIndex].toDouble(&ok);

            if (ok)
                col.setter(p, v);
        }

        outData.push_back(p);
    }

    file.close();

    emit importFinished(true, filePath);
    return true;
}

bool Csv_worker::exportByKeys(const std::vector<parametrs>& data,
                              const QStringList& keys,
                              const QString& filePath,
                              QChar sep)
{
    std::vector<Column> cols;

    for (const QString& key : keys) {
        if (key == "time") {
            cols.push_back({"time", [](const parametrs& p){ return p.time; }});
            continue;
        }

        if (auto spec = findFieldByKey(key)) {
            cols.push_back({spec->key, spec->getter});
        }
    }

    if (cols.empty()) {
        emit exportFinished(false, filePath);
        return false;
    }

    std::ofstream file(filePath.toStdString(),
                       std::ios::out | std::ios::trunc | std::ios::binary);

    if (!file.is_open()) {
        emit exportFinished(false, filePath);
        return false;
    }

    file.imbue(std::locale::classic());
    file << std::fixed << std::setprecision(6);

    const char csep = static_cast<char>(sep.unicode());

    auto writeCsvText = [&](const QString& s) {
        const QByteArray utf8 = s.toUtf8();

        const bool needQuotes =
            utf8.contains(csep) ||
            utf8.contains('"') ||
            utf8.contains('\n') ||
            utf8.contains('\r');

        if (!needQuotes) {
            file.write(utf8.constData(), utf8.size());
            return;
        }

        file.put('"');

        for (char ch : utf8) {
            if (ch == '"')
                file.put('"');
            file.put(ch);
        }

        file.put('"');
    };

    auto writeCsvNumber = [&](double v) {
        if (std::isfinite(v))
            file << v;
    };

    for (size_t i = 0; i < cols.size(); ++i) {
        if (i) file << csep;
        writeCsvText(cols[i].header);
    }

    file << '\n';

    for (const auto& p : data) {
        for (size_t i = 0; i < cols.size(); ++i) {
            if (i) file << csep;
            writeCsvNumber(cols[i].getter(p));
        }

        file << '\n';
    }

    file.close();

    emit exportFinished(true, filePath);
    return true;
}
