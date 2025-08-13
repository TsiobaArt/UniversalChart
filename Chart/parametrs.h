#ifndef PARAMETRS_H
#define PARAMETRS_H

#include <cstdint>
#include <qobject.h>

struct parametrs
{
    float time {0}; // час
    float step {0};

    uint8_t configuration {0};

    float psi {0};      // рискання (рад)
    float teta {0};     // тангаж (рад)
    float gamma {0};    // крен (рад)

    float teta_ks {0};  // кут нахилу траєкторії (рад)
    float psi_ks {0};   // курс (рад)

    float alfa {0};     // кут атаки (рад)
    float beta {0};     // кут ковзання (рад)
    float gamma_c {0};  // кут швидкісного крену (рад)

    float v_ground {0};

    float wx {0}; float wy {0}; float wz {0}; // кутові швидкості (рад/с)
    float angleWx {0}; float angleWy {0}; float angleWz {0}; // інтегровані кути (рад)

    float machNumber {0};

    float vx {0}; float vy {0}; float vz {0}; // проекції швидкості

    float mx {0}; float my {0}; float mz {0};

    float deltaChannel1  {0};
    float deltaChannel2  {0};
    float deltaElerons   {0};

    float delta_1 {0}; float delta_2 {0};
    float delta_3 {0}; float delta_4 {0};

    float jx {0}; float jy {0}; float jz {0};

    float xp {0}; float yp {0}; float zp {0};

    float mass {0};
    float trust {0};

    float g {9.80665};

    float xg {0}; float yg {0}; float zg {0};

    float latitude {0};
    float longitude {0};
    float altitude_wgs {0};

    float startLatitude {0};
    float startLongitude {0};

    float nx {0}; float ny {0}; float nz {0};
    float n_abs {0};
    float n_razp {0}; float n_max {15};

    float reserve_1 {0}; float reserve_2 {0}; float reserve_3 {0}; float reserve_4 {0};
};

struct Channel_PID {
    double channel1_kp = 0.0, channel1_ki = 0.0, channel1_kd = 0.0;
    double channel2_kp = 0.0, channel2_ki = 0.0, channel2_kd = 0.0;
    double channel3_kp = 0.0, channel3_ki = 0.0, channel3_kd = 0.0;
};

struct DeviceOrientation {
    double latitude = 0.0, longitude = 0.0;
    double x = 0.0, y = 0.0, z = 0.0;
    double yaw = 0.0, pitch = 0.0, roll = 0.0;
    double wx = 0, wy = 0, wz = 0;
};

struct Target {
    double latitude = 0.0, longitude = 0.0;
    double x = 0.0, y = 0.0, z = 0.0;
};

struct ControlSettings {
    bool rudderEnabled = false;
    double deviation1 = 0.0, deviation2 = 0.0, deviation3 = 0.0;
    double duration = 0.0, startTime = 0.0;
    double k_h = 0.0, k_Wy = 0.0, k_teta = 0.0;
    bool altitudeEnabled = false;
    bool altitude = 0.0;
};

struct GeoCoord { double lat_deg; double lon_deg; double alt; };
struct ENU { double x; double y; double z; };

Q_DECLARE_METATYPE(Channel_PID)
Q_DECLARE_METATYPE(DeviceOrientation)
Q_DECLARE_METATYPE(Target)
Q_DECLARE_METATYPE(ControlSettings)

#endif // PARAMETRS_H
