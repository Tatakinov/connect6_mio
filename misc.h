#ifndef MISC_H_
#define MISC_H_

#include <cmath>

enum class State { Continue, Win, Draw, Lose };

using move_t = int;
using move2_t = int;
using score_t = int;

template<typename T>
struct data_t {
    move_t move;
    T score;
};

constexpr int kSize = 15;
constexpr int kBoardSize = kSize * kSize;
constexpr move_t kInvalidMove = 0xff;

template <typename T>
inline T infinite() {
    if constexpr (std::is_same_v<T, score_t>) {
        return 0x0fffffff;
    }
}

template <typename T>
inline T reverse(T score) {
    if constexpr (std::is_same_v<T, score_t>) {
        return -score;
    }
}

template <typename T>
inline T invalidScore() {
    if constexpr (std::is_same_v<T, score_t>) {
        return reverse<T>(infinite<T>()) - 1;
    }
}

template <typename T>
inline T draw() {
    if constexpr (std::is_same_v<T, score_t>) {
        return 0;
    }
}

template <typename T>
inline double toScore(T score) {
    if constexpr (std::is_same_v<T, score_t>) {
        if (score == infinite<T>()) {
            return 1;
        }
        else if (score == reverse<T>(score)) {
            return 0;
        }
        else {
            return (tanh((score / 100.0) / 2.0) + 1) / 2;
        }
    }
}

#endif // MISC_H_
