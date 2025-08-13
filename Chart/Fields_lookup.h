#pragma once
#include "Fields_parametrs.h"
#include <optional>

inline std::optional<FieldSpec<parametrs>> findFieldByKey(const QString& key) {
    for (const auto& f : PARAM_FIELDS()) {
        if (f.key == key) return f;
    }
    return std::nullopt;
}
