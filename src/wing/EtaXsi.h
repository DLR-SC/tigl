#pragma once

namespace tigl
{
struct EtaXsi {
    EtaXsi()
        : eta(0)
        , xsi(0) {};
    EtaXsi(double eta, double xsi)
        : eta(eta)
        , xsi(xsi) {};
    double eta;
    double xsi;
};
}
