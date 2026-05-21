#include <iostream>
#include <windows.h>
#include "classes/headers/MutableArraySequence.h"
#include <tuple>
#include <cmath>
#include <stdexcept>
#include <memory>


#include "classes/headers/SegmentedDeque.h"
#include "classes/headers/UIDequeSFML.h"

double Length(double g, double v, double alpha) {
    return v * v * sin(2.0 * alpha) / g;
}

template<typename T>
std::tuple<double, double> SolveProjectile(double g, double x, const Sequence<T> &v, double eps) {
    if (g < 0 || x < 0 || eps < 0 || v.GetLength() == 0) {
        throw std::invalid_argument("Invalid arguments");
    }
    T v_current = 0;
    double alpha1 = 0, alpha2 = 0, alpha = 0;
    auto iter = v.GetEnumerator();
    while (iter->MoveNext()) {
        v_current = iter->Current();
        alpha1 = 0;
        alpha2 = M_PI / 4;
        while (alpha2 - alpha1 > eps) {
            alpha = (alpha1 + alpha2) / 2;
            if ((Length(g, v_current, alpha) - x) * (Length(g, v_current, alpha2) - x) >= 0) {
                alpha2 = alpha;
            } else {
                alpha1 = alpha;
            }
        }
        if ((Length(g, v_current, (alpha2 + alpha1) / 2) - x) > 0) {
            return {v_current, (alpha2 + alpha1) / 2};
        }
    }
    return {0, 0};
}

int main() {
    system("chcp 65001");
    runSFML();
    return 0;
}
