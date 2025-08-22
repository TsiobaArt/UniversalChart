#include "csvexporter.h"
#include "Fields_lookup.h"
#include <fstream>
#include <iomanip>
#include <locale>
#include <QDebug>
#include "FieldSpec.h"
CsvExporter::CsvExporter(QObject* parent) : QObject(parent) {}


static QString makeHeader(QString label, QString unit)
{
    label.replace(',', ' ');
    label = label.simplified();
    if (!unit.isEmpty() && !label.contains('['))
        label += " [" + unit + "]";
    return label;
}

bool CsvExporter::exportSelected(const std::vector<parametrs>& data,
                                 const QStringList& keys,
                                 const QString& filePath,
                                 QChar sep)
{
    std::vector<Column> cols;
    // якщо потрібен час — розкоментуй:
    // cols.push_back({"time", [](const parametrs& p){ return p.time; }});

    for (const auto& k : keys) {
        if (auto spec = findFieldByKey(k)) {
            cols.push_back({ makeHeader(spec->label, spec->unit), spec->getter });
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

// bool CsvExporter::exportSelected(const std::vector<parametrs>& data,
//                                  const QStringList& keys,
//                                  const QString& filePath,
//                                  QChar sep)
// {
//     std::vector<Column> cols;
//     // cols.push_back({"time", [](const parametrs& p){ return p.time; }});

//     for (const auto& k : keys) {
//         if (auto spec = findFieldByKey(k)) {
//             QString header = spec->label;
//             if (!spec->unit.isEmpty())
//                 header += " [" + spec->unit + "]";   // <- зшиваємо label + unit в ОДИН заголовок
//             cols.push_back({header, spec->getter});
//         }
//     }

//     if (cols.size() <= 1) {
//         emit exportFinished(false, filePath);
//         return false;
//     }

//     std::ofstream file(filePath.toStdString(), std::ios::out | std::ios::trunc);
//     if (!file.is_open()) {
//         emit exportFinished(false, filePath);
//         return false;
//     }

//     file.imbue(std::locale::classic());
//     file << std::fixed << std::setprecision(6);

//     const char csep = static_cast<char>(sep.unicode());

//     // Заголовок
//     for (size_t i = 0; i < cols.size(); ++i) {
//         if (i) file << csep;
//         file << cols[i].header.toStdString();
//     }
//     file << '\n';

//     // Дані
//     for (const auto& p : data) {
//         for (size_t i = 0; i < cols.size(); ++i) {
//             if (i) file << csep;
//             file << cols[i].getter(p);
//         }
//         file << '\n';
//     }

//     file.close();
//     emit exportFinished(true, filePath);
//     return true;
// }
