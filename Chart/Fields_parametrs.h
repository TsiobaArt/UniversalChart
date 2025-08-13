#pragma once
#include "parametrs.h"
#include "FieldSpec.h"
#include <QRandomGenerator>
#include <vector>
#include <QtMath>

inline QColor niceColor() {
    return QColor(
        QRandomGenerator::global()->bounded(100, 256),
        QRandomGenerator::global()->bounded(100, 256),
        QRandomGenerator::global()->bounded(100, 256)
    );
}

inline const std::vector<FieldSpec<parametrs>>& PARAM_FIELDS() {
    static std::vector<FieldSpec<parametrs>> F = {
        // кути (рад -> град)
        {"psi",        u8"ψ (курс), °",           "deg", niceColor(), true,  [](const auto& d){ return qRadiansToDegrees(d.psi); }},
        {"teta",       u8"θ (тангаж), °",         "deg", niceColor(), true,  [](const auto& d){ return qRadiansToDegrees(d.teta); }},
        {"gamma",      u8"φ (крен), °",           "deg", niceColor(), true,  [](const auto& d){ return qRadiansToDegrees(d.gamma); }},
        {"teta_ks",    u8"θ_ks (кут траєкт.), °", "deg", niceColor(), false, [](const auto& d){ return qRadiansToDegrees(d.teta_ks); }},
        {"psi_ks",     u8"ψ_ks (курс), °",        "deg", niceColor(), false, [](const auto& d){ return qRadiansToDegrees(d.psi_ks); }},
        {"alfa",       u8"α (атаки), °",          "deg", niceColor(), false, [](const auto& d){ return qRadiansToDegrees(d.alfa); }},
        {"beta",       u8"β (ковзання), °",       "deg", niceColor(), false, [](const auto& d){ return qRadiansToDegrees(d.beta); }},
        {"gamma_c",    u8"γ_c (швидк. крен), °",  "deg", niceColor(), false, [](const auto& d){ return qRadiansToDegrees(d.gamma_c); }},

        // швидкості/мах
        {"v_ground",   u8"V_ground, м/с",         "m/s", niceColor(), false, [](const auto& d){ return d.v_ground; }},
        {"vx",         u8"Vx, м/с",               "m/s", niceColor(), false, [](const auto& d){ return d.vx; }},
        {"vy",         u8"Vy, м/с",               "m/s", niceColor(), false, [](const auto& d){ return d.vy; }},
        {"vz",         u8"Vz, м/с",               "m/s", niceColor(), false, [](const auto& d){ return d.vz; }},
        {"machNumber", u8"Mach",                   "",   niceColor(), false, [](const auto& d){ return d.machNumber; }},

        // кутові швидкості (рад/с -> град/с)
        {"wx",         u8"ωx, °/с",               "deg/s", niceColor(), false, [](const auto& d){ return qRadiansToDegrees(d.wx); }},
        {"wy",         u8"ωy, °/с",               "deg/s", niceColor(), false, [](const auto& d){ return qRadiansToDegrees(d.wy); }},
        {"wz",         u8"ωz, °/с",               "deg/s", niceColor(), false, [](const auto& d){ return qRadiansToDegrees(d.wz); }},

        // моменти/сили
        {"mx",         u8"Mx",                     "",   niceColor(), false, [](const auto& d){ return d.mx; }},
        {"my",         u8"My",                     "",   niceColor(), false, [](const auto& d){ return d.my; }},
        {"mz",         u8"Mz",                     "",   niceColor(), false, [](const auto& d){ return d.mz; }},
        {"xp",         u8"Xp (лобовий)",           "",   niceColor(), false, [](const auto& d){ return d.xp; }},
        {"yp",         u8"Yp (підйомна)",          "",   niceColor(), false, [](const auto& d){ return d.yp; }},
        {"zp",         u8"Zp (бокова)",            "",   niceColor(), false, [](const auto& d){ return d.zp; }},

        // положення/координати
        {"xg",         u8"Xg, м",                 "m",  niceColor(), false, [](const auto& d){ return d.xg; }},
        {"yg",         u8"Yg, м",                 "m",  niceColor(), false, [](const auto& d){ return d.yg; }},
        {"zg",         u8"Zg, м",                 "m",  niceColor(), false, [](const auto& d){ return d.zg; }},

        // інше
        {"mass",       u8"Маса, кг",              "kg", niceColor(), false, [](const auto& d){ return d.mass; }},
        {"trust",      u8"Тяга, Н",               "N",  niceColor(), false, [](const auto& d){ return d.trust; }},
        {"g",          u8"g, м/с²",               "m/s²", niceColor(), false, [](const auto& d){ return d.g; }},

        {"nx",         u8"n_x, g",                "g",  niceColor(), false, [](const auto& d){ return d.nx; }},
        {"ny",         u8"n_y, g",                "g",  niceColor(), false, [](const auto& d){ return d.ny; }},
        {"nz",         u8"n_z, g",                "g",  niceColor(), false, [](const auto& d){ return d.nz; }},
        {"n_razp",     u8"n_разп, g",             "g",  niceColor(), false, [](const auto& d){ return d.n_razp; }},
        {"n_max",      u8"n_max, g",              "g",  niceColor(), false, [](const auto& d){ return d.n_max; }},

        {"deltaChannel1", u8"ΔCh1, °",            "deg", niceColor(), false, [](const auto& d){ return d.deltaChannel1; }},
        {"deltaChannel2", u8"ΔCh2, °",            "deg", niceColor(), false, [](const auto& d){ return d.deltaChannel2; }},
        {"deltaElerons",  u8"ΔAil, °",            "deg", niceColor(), false, [](const auto& d){ return d.deltaElerons; }},

        {"angleWx",    u8"Δωx інтегр., °",        "deg", niceColor(), false, [](const auto& d){ return qRadiansToDegrees(d.angleWx); }},
        {"angleWy",    u8"Δωy інтегр., °",        "deg", niceColor(), false, [](const auto& d){ return qRadiansToDegrees(d.angleWy); }},
        {"angleWz",    u8"Δωz інтегр., °",        "deg", niceColor(), false, [](const auto& d){ return qRadiansToDegrees(d.angleWz); }},

        // резерв
        {"reserve_1",  u8"reserve_1",             "",   niceColor(), false, [](const auto& d){ return d.reserve_1; }},
        {"reserve_2",  u8"reserve_2",             "",   niceColor(), false, [](const auto& d){ return d.reserve_2; }},
        {"reserve_3",  u8"reserve_3",             "",   niceColor(), false, [](const auto& d){ return d.reserve_3; }},
        {"reserve_4",  u8"reserve_4",             "",   niceColor(), false, [](const auto& d){ return d.reserve_4; }},
    };
    return F;
}
