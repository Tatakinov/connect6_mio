#define _USE_MATH_DEFINES
//#define _DEBUG_MATE

#include "connect6.h"

#include <algorithm>
#include <cassert>
#include <chrono>
#include <cmath>
#include <iostream>
#include <memory>
#include <numeric>
#include <optional>
#include <random>
#include <unordered_set>

#include "and_node.h"
#include "or_node.h"

namespace {
    constexpr int max_mate_depth = -1;
    constexpr int mate_threshold = 1024;

    std::random_device rd;
    std::mt19937 mt(rd());
    std::uniform_real_distribution<> dist(0.0, 1.0);

    const std::bitset<kBoardSize>  shift1("011111111111111011111111111111011111111111111011111111111111011111111111111011111111111111011111111111111011111111111111011111111111111011111111111111011111111111111011111111111111011111111111111011111111111111011111111111111");
    const std::bitset<kBoardSize> shift14("111111111111110111111111111110111111111111110111111111111110111111111111110111111111111110111111111111110111111111111110111111111111110111111111111110111111111111110111111111111110111111111111110111111111111110111111111111110");
    const std::bitset<kBoardSize> mask_horizontal("000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000111111");
    const std::bitset<kBoardSize> mask_vertical("000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000001000000000000001000000000000001000000000000001000000000000001000000000000001");
    const std::bitset<kBoardSize> mask_diagonal1("000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000100000000000000010000000000000001000000000000000100000000000000010000000000000001");
    const std::bitset<kBoardSize> mask_diagonal2("000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000001000000000000010000000000000100000000000001000000000000010000000000000100000");

    template<int size, int direction>
    std::bitset<kBoardSize> renSingle(const std::bitset<kBoardSize>& bits) {
        auto b = bits;
        switch (direction) {
            case 0:
                for (int i = 0; i < size - 1; i++) {
                    b &= (b & shift1) << 1;
                }
                break;
            case 1:
                for (int i = 0; i < size - 1; i++) {
                    b &= b << kSize;
                }
                break;
            case 2:
                for (int i = 0; i < size - 1; i++) {
                    b &= (b & shift14) << (kSize - 1);
                }
                break;
            case 3:
                for (int i = 0; i < size - 1; i++) {
                    b &= (b & shift1) << (kSize + 1);
                }
                break;
            default:
                assert(false);
                break;
        }
        return b;
    }

    template<int size>
    std::bitset<kBoardSize> ren(const std::bitset<kBoardSize>& b) {
        return renSingle<size, 0>(b) | renSingle<size, 1>(b) |
            renSingle<size, 2>(b) | renSingle<size, 3>(b);
    }

    std::unordered_map<int, int> countAliveMaxRen(const std::bitset<kBoardSize>& target, const std::bitset<kBoardSize>& mask){
        std::unordered_map<int, int> map = {
            {1, 0},
            {2, 0},
            {3, 0},
            {4, 0},
            {5, 0},
            {6, 0},
        };
        {
            auto hash = [](int x, int y) {
                return y;
            };
            std::unordered_map<int, int> max;
            std::unordered_map<int, bool> ascend;
            for (int y = 0; y < kSize; y++) {
                auto m = mask_horizontal << (y * kSize);
                for (int x = 0; x < kSize - 6 + 1; x++) {
                    if ((mask & (m << x)).count() == 6) {
                        int count = (target & (m << x)).count();
                        if (max[hash(x, y)] < count) {
                            max[hash(x, y)] = count;
                            ascend[hash(x, y)] = true;
                            if (x == kSize - 6 && ascend[hash(x, y)]) {
                                map[max[hash(x, y)]]++;
                            }
                        }
                        else if (max[hash(x, y)] == count) {
                            if (x == kSize - 6 && ascend[hash(x, y)]) {
                                map[max[hash(x, y)]]++;
                            }
                            // nop
                        }
                        else {
                            if (ascend[hash(x, y)]) {
                                map[max[hash(x, y)]]++;
                            }
                            max[hash(x, y)] = count;
                            ascend[hash(x, y)] = false;
                        }
                    }
                    else {
                        if (ascend[hash(x, y)]) {
                            map[max[hash(x, y)]]++;
                        }
                        max[hash(x, y)] = 0;
                        ascend[hash(x, y)] = false;
                    }
                }
            }
        }
        {
            auto hash = [](int x, int y) {
                return x;
            };
            std::unordered_map<int, int> max;
            std::unordered_map<int, bool> ascend;
            for (int y = 0; y < kSize - 6 + 1; y++) {
                auto m = mask_vertical << (y * kSize);
                for (int x = 0; x < kSize; x++) {
                    if ((mask & (m << x)).count() == 6) {
                        int count = (target & (m << x)).count();
                        if (max[hash(x, y)] < count) {
                            max[hash(x, y)] = count;
                            ascend[hash(x, y)] = true;
                            if (y == kSize - 6 && ascend[hash(x, y)]) {
                                map[max[hash(x, y)]]++;
                            }
                        }
                        else if (max[hash(x, y)] == count) {
                            if (y == kSize - 6 && ascend[hash(x, y)]) {
                                map[max[hash(x, y)]]++;
                            }
                        }
                        else {
                            if (ascend[hash(x, y)]) {
                                map[max[hash(x, y)]]++;
                            }
                            max[hash(x, y)] = count;
                            ascend[hash(x, y)] = false;
                        }
                    }
                    else {
                        if (ascend[hash(x, y)]) {
                            map[max[hash(x, y)]]++;
                        }
                        max[hash(x, y)] = 0;
                        ascend[hash(x, y)] = false;
                    }
                }
            }
        }
        {
            auto hash = [](int x, int y) {
                return x - y;
            };
            std::unordered_map<int, int> max;
            std::unordered_map<int, bool> ascend;
            for (int y = 0; y < kSize - 6 + 1; y++) {
                auto m = mask_diagonal1 << (y * kSize);
                for (int x = 0; x < kSize - 6 + 1; x++) {
                    if ((mask & (m << x)).count() == 6) {
                        int count = (target & (m << x)).count();
                        if (max[hash(x, y)] < count) {
                            max[hash(x, y)] = count;
                            ascend[hash(x, y)] = true;
                            if ((x == kSize - 6 || y == kSize - 6) && ascend[hash(x, y)]) {
                                map[max[hash(x, y)]]++;
                            }
                        }
                        else if (max[hash(x, y)] == count) {
                            if ((x == kSize - 6 || y == kSize - 6) && ascend[hash(x, y)]) {
                                map[max[hash(x, y)]]++;
                            }
                        }
                        else {
                            if (ascend[hash(x, y)]) {
                                map[max[hash(x, y)]]++;
                            }
                            max[hash(x, y)] = count;
                            ascend[hash(x, y)] = false;
                        }
                    }
                    else {
                        if (ascend[hash(x, y)]) {
                            map[max[hash(x, y)]]++;
                        }
                        max[hash(x, y)] = 0;
                        ascend[hash(x, y)] = false;
                    }
                }
            }
        }
        {
            auto hash = [](int x, int y) {
                return x + y;
            };
            std::unordered_map<int, int> max;
            std::unordered_map<int, bool> ascend;
            for (int y = 0; y < kSize - 6 + 1; y++) {
                auto m = mask_diagonal2 << (y * kSize);
                for (int x = 0; x < kSize - 6 + 1; x++) {
                    if ((mask & (m << x)).count() == 6) {
                        int count = (target & (m << x)).count();
                        if (max[hash(x, y)] < count) {
                            max[hash(x, y)] = count;
                            ascend[hash(x, y)] = true;
                            if ((x == 0 || y == kSize - 6) && ascend[hash(x, y)]) {
                                map[max[hash(x, y)]]++;
                            }
                        }
                        else if (max[hash(x, y)] == count) {
                            if ((x == 0 || y == kSize - 6) && ascend[hash(x, y)]) {
                                map[max[hash(x, y)]]++;
                            }
                        }
                        else {
                            if (ascend[hash(x, y)]) {
                                map[max[hash(x, y)]]++;
                            }
                            max[hash(x, y)] = count;
                            ascend[hash(x, y)] = false;
                        }
                    }
                    else {
                        if (ascend[hash(x, y)]) {
                            map[max[hash(x, y)]]++;
                        }
                        max[hash(x, y)] = 0;
                        ascend[hash(x, y)] = false;
                    }
                }
            }
        }
        return map;
    }

    template<int num, bool immediately>
    int countAliveRen(const std::bitset<kBoardSize>& target, const std::bitset<kBoardSize>& mask){
        int sum = 0;
        {
            for (int y = 0; y < kSize; y++) {
                auto m = mask_horizontal << (y * kSize);
                for (int x = 0; x < kSize - 6 + 1; x++) {
                    if ((mask & (m << x)).count() == 6) {
                        if ((target & (m << x)).count() == num) {
                            if constexpr (immediately) {
                                return 1;
                            }
                            sum++;
                        }
                    }
                }
            }
        }
        {
            for (int y = 0; y < kSize - 6 + 1; y++) {
                auto m = mask_vertical << (y * kSize);
                for (int x = 0; x < kSize; x++) {
                    if ((mask & (m << x)).count() == 6) {
                        if ((target & (m << x)).count() == num) {
                            if constexpr (immediately) {
                                return 1;
                            }
                            sum++;
                        }
                    }
                }
            }
        }
        {
            for (int y = 0; y < kSize - 6 + 1; y++) {
                auto m = mask_diagonal1 << (y * kSize);
                for (int x = 0; x < kSize - 6 + 1; x++) {
                    if ((mask & (m << x)).count() == 6) {
                        if ((target & (m << x)).count() == num) {
                            if constexpr (immediately) {
                                return 1;
                            }
                            sum++;
                        }
                    }
                }
            }
        }
        {
            for (int y = 0; y < kSize - 6 + 1; y++) {
                auto m = mask_diagonal2 << (y * kSize);
                for (int x = 0; x < kSize - 6 + 1; x++) {
                    if ((mask & (m << x)).count() == 6) {
                        if ((target & (m << x)).count() == num) {
                            if constexpr (immediately) {
                                return 1;
                            }
                            sum++;
                        }
                    }
                }
            }
        }
        return sum;
    }

    template<int size>
    std::bitset<kBoardSize> generate(std::bitset<kBoardSize> bits) {
        std::bitset<kBoardSize> ret(0);
        {
            auto b = bits;
            ret |= b << kSize * size;
            ret |= b >> kSize * size;
        }
        {
            auto b = bits;
            for (int i = 0; i < size; i++) {
                b = (b & shift1) << 1;
            }
            ret |= b;
            ret |= b << kSize * size;
            ret |= b >> kSize * size;
        }
        {
            auto b = bits;
            for (int i = 0; i < size; i++) {
                b = (b & shift14) >> 1;
            }
            ret |= b;
            ret |= b << kSize * size;
            ret |= b >> kSize * size;
        }
        return ret;
    }
}

template <typename T>
void Connect6<T>::init() {
    std::bitset<kBoardSize> bit0(0);
    player_ &= bit0;
    opponent_ &= bit0;
    tesuu_ = 0;
    //mate_table.clear();
    //mate_move_table.clear();
}

template <typename T>
void Connect6<T>::setPosition(std::string fen) {
    init();
    for (int i = 0; i < kBoardSize; i++) {
        if (fen[i] == '1') {
            player_.set(i);
            tesuu_++;
        }
        if (fen[i + kBoardSize] == '1') {
            opponent_.set(i);
            tesuu_++;
        }
    }
}


template<>
score_t Connect6<score_t>::evaluate() const {
    score_t score = 0;
    {
        score += countAliveRen<5, false>(player_, ~opponent_) * 27;
        score += countAliveRen<4, false>(player_, ~opponent_) * 27;
        score += countAliveRen<3, false>(player_, ~opponent_) * 9;
        score += countAliveRen<2, false>(player_, ~opponent_) * 3;
        score += countAliveRen<1, false>(player_, ~opponent_);
    }
    {
        score -= countAliveRen<5, false>(opponent_, ~player_) * 81;
        score -= countAliveRen<4, false>(opponent_, ~player_) * 81;
        score -= countAliveRen<3, false>(opponent_, ~player_) * 27;
        score -= countAliveRen<2, false>(opponent_, ~player_) * 9;
        score -= countAliveRen<1, false>(opponent_, ~player_) * 3;
    }
    return score;
}

template <typename T>
T Connect6<T>::searchInternal(int depth, double alpha, double beta, bool search_mate) const {
    switch (isGameOver()) {
        case State::Win:
            return infinite<T>();
            break;
        case State::Draw:
            return draw<T>();
            break;
        case State::Lose:
            return reverse<T>(infinite<T>());
            break;
        case State::Continue:
            // nop
            break;
        default:
            assert(false);
            break;
    }
    if (isMate(true, search_mate ? max_mate_depth : 0)) {
        return infinite<T>();
    }
    if (isMate(false, search_mate ? max_mate_depth : 0)) {
        return reverse<T>(infinite<T>());
    }
    if (depth <= 0) {
        return evaluate();
    }

    T ret = reverse<T>(infinite<T>());
    // State::ContinueはgenerateMoves().size() > 0が保証されている。
    for (auto move : generateMoves()) {
        Connect6<T> c = *this;
        T score;
        c.move(move);
        if (isPlayerChanged()) {
            score = c.searchInternal(depth - 1, std::max(alpha, toScore<T>(ret)), beta, search_mate);
        }
        else {
            score = c.searchInternal(depth - 1, 1 - beta, 1 - std::max(alpha, toScore<T>(ret)), search_mate);
            score = reverse<T>(score);
        }
        if (toScore(ret) < toScore(score)) {
            ret = score;
            if (toScore(ret) >= beta) {
                break;
            }
        }
    }

    return ret;
}

template <typename T>
data_t<T> Connect6<T>::searchInternalRoot(int depth, double alpha, double beta, bool learn) const {
    data_t<T> ret = {kInvalidMove, invalidScore<T>()};
    switch (isGameOver()) {
        case State::Win:
            ret.score = infinite<T>();
            return ret;
            break;
        case State::Draw:
            ret.score = draw<T>();
            return ret;
            break;
        case State::Lose:
            ret.score = reverse<T>(infinite<T>());
            return ret;
            break;
        case State::Continue:
            // nop
            break;
        default:
            assert(false);
            break;
    }
    if (depth <= 0) {
        ret.score = evaluate();
        return ret;
    }

    std::vector<move_t> moves = generateMoves();
    std::vector<double> scores;
    scores.reserve(moves.size());
    // State::ContinueはgenerateMoves().size() > 0が保証されている。
    for (auto move : moves) {
        Connect6<T> c = *this;
        c.move(move);
        bool is_mate = false;
        if (isPlayerChanged()) {
            is_mate = c.isMate(true, 0);
        }
        else {
            is_mate = c.isMate(false, 0);
        }
        if (is_mate) {
            ret.move = move;
            ret.score = infinite<T>();
            return ret;
        }
    }
    bool search_mate = false;
    if (!learn) {
        auto c = *this;
        if (c.isMate(true, -1)) {
            if (mate_move_table.count(c.getPosition())) {
                std::cout << "mate(cached)." << std::endl;
                ret.move = mate_move_table[c.getPosition()];
                ret.score = infinite<T>();
                return ret;
            }
            moves = generateMateMoves<true>();
            search_mate = true;
            std::cout << "mate." << std::endl;
        }
        else if (c.isMate(false, -1)) {
            moves = generateMateMoves<false>();
            search_mate = true;
        }
        else {
            if (c.isPlayerChanged()) {
                c.nullMove();
                c.nullMove();
            }
            else {
                c.nullMove();
            }
            if (c.isMate(true, -1)) {
                moves = generateMateMoves<false>();
            }
        }
    }
    for (auto move : moves) {
        Connect6<T> c = *this;
        T score;
        c.move(move);
        if (isPlayerChanged()) {
            score = c.searchInternal(depth - 1, std::max(alpha, toScore<T>(ret.score)), beta, search_mate);
        }
        else {
            score = c.searchInternal(depth - 1, 1 - beta, 1 - std::max(alpha, toScore(ret.score)), search_mate);
            score = reverse<T>(score);
        }
        scores.push_back(toScore<T>(score));
        if (toScore(ret.score) < toScore(score) || ret.score == invalidScore<T>()) {
            ret.move = move;
            ret.score = score;
            if (score == infinite<T>()) {
                return ret;
            }
            if (toScore<T>(ret.score) >= beta) {
                break;
            }
        }
    }

    double total = std::reduce(scores.begin(), scores.end());
    if (total > 0) {
        for (auto& s : scores) {
            s /= total;
        }
        for (int i = 0; i < random_move_factor_; i++) {
            total = 0;
            for (auto& s : scores) {
                s *= s;
                total += s;
            }
            for (auto& s : scores) {
                s /= total;
            }
        }
#if 0
        for (int i = 0; i < moves.size(); i++) {
            auto m = moves[i];
            auto s = scores[i];
            std::cout << (m % kSize) << "," << (m / kSize) <<
                " -> " << s << std::endl;
        }
#endif
        auto threshold = dist(mt);
        for (int i = 0; i < scores.size(); i++) {
            threshold -= scores[i];
            if (threshold < 0) {
                ret.move = moves[i];
                break;
            }
        }
    }

    return ret;
}

template <typename T>
std::vector<move_t> Connect6<T>::generateMoves() const {
    std::vector<move_t> moves;
    std::bitset bits = player_ | opponent_;
    if (bits.all()) {
        return moves;
    }
    moves.reserve(kBoardSize);
    bits.flip();
    for (int i = 0; i < kBoardSize; i++) {
        if (bits[i]) {
            moves.push_back(i);
        }
    }
    return moves;
}

template <typename T>
std::vector<move_t> Connect6<T>::generateAIMoves() const {
    std::bitset<kBoardSize> m1(0);
    std::bitset<kBoardSize> m2(0);
    std::bitset<kBoardSize> m3(0);
    std::vector<move_t> moves;
    std::bitset bits = player_ | opponent_;
    if (bits.all()) {
        return moves;
    }
    if (bits.none()) {
        return generateMoves();
    }
    moves.reserve(kBoardSize);
    {
        auto b = bits;
        m2 |= b << kSize * 2;
        m1 |= b << kSize;
        m1 |= b >> kSize;
        m2 |= b >> kSize * 2;
    }
    {
        auto b = (bits & shift1) << 1;
        m2 |= b << kSize * 2;
        m1 |= b << kSize;
        m1 |= b;
        m1 |= b >> kSize;
        m2 |= b >> kSize * 2;
    }
    {
        auto b = (bits & shift1) << 1;
        b = (b & shift1) << 1;
        m2 |= b << kSize * 2;
        m2 |= b << kSize;
        m2 |= b;
        m2 |= b >> kSize;
        m2 |= b >> kSize * 2;
    }
    {
        auto b = (bits & shift14) >> 1;
        m2 |= b << kSize * 2;
        m1 |= b << kSize;
        m1 |= b;
        m1 |= b >> kSize;
        m2 |= b >> kSize * 2;
    }
    {
        auto b = (bits & shift14) >> 1;
        b = (b & shift14) >> 1;
        m2 |= b << kSize * 2;
        m2 |= b << kSize;
        m2 |= b;
        m2 |= b >> kSize;
        m2 |= b >> kSize * 2;
    }
    {
        auto b = bits;
        m3 |= b << kSize * 3;
        m3 |= b >> kSize * 3;
    }
    {
        auto b = (bits & shift1) << 1;
        b = (b & shift1) << 1;
        b = (b & shift1) << 1;
        m3 |= b;
        m3 |= b << kSize * 3;
        m3 |= b >> kSize * 3;
    }
    {
        auto b = (bits & shift14) >> 1;
        b = (b & shift14) >> 1;
        b = (b & shift14) >> 1;
        m3 |= b;
        m3 |= b << kSize * 3;
        m3 |= b >> kSize * 3;
    }
    m1 &= (~(bits));
    m2 &= (~(bits ^ m1));
    m3 &= (~(bits ^ m1 ^ m2));
    for (int i = 0; i < kBoardSize; i++) {
        if (m1[i]) {
            moves.push_back(i);
        }
    }
    for (int i = 0; i < kBoardSize; i++) {
        if (m2[i]) {
            moves.push_back(i);
        }
    }
    for (int i = 0; i < kBoardSize; i++) {
        if (m3[i]) {
            moves.push_back(i);
        }
    }
    return moves;
}

template <typename T>
void Connect6<T>::nullMove() {
    tesuu_++;
    if (isPlayerChanged()) {
        std::swap(player_, opponent_);
    }
}

template <typename T>
void Connect6<T>::move(move_t move) {
    assert(!player_[move]);
    assert(!opponent_[move]);
    player_.set(move);
    nullMove();
}

template <typename T>
State Connect6<T>::isGameOver() const {
    if ((player_ | opponent_).all()) {
        return State::Draw;
    }
    if (ren<6>(player_).any()) {
        return State::Win;
    }
    else if (ren<6>(opponent_).any()) {
        return State::Lose;
    }
    return State::Continue;
}

template <typename T>
template<bool attack>
std::unordered_map<int, int> Connect6<T>::getRen() const {
    if constexpr (attack) {
        return countAliveMaxRen(player_, ~opponent_);
    }
    else {
        return countAliveMaxRen(opponent_, ~player_);
    }
}

template <typename T>
bool Connect6<T>::isMate(bool attack, int depth) const {
    mate_t key = {getPosition(), attack};
    if (mate_table.count(key)) {
        return mate_table[key];
    }
    switch (isGameOver()) {
        case State::Win:
            mate_table[key] = attack;
            return attack;
            break;
        case State::Lose:
            mate_table[key] = !attack;
            return !attack;
            break;
        case State::Draw:
            mate_table[key] = false;
            return false;
            break;
        default:
            break;
    }
    if (attack) {
        auto map = countAliveMaxRen(player_, ~opponent_);
        if (isPlayerChanged()) {
            if (map[4] || map[5]) {
                mate_table[key] = true;
                return true;
            }
        }
        else {
            if (map[5]) {
                mate_table[key] = true;
                return true;
            }
        }
    }
    else {
        auto map = countAliveMaxRen(opponent_, ~player_);
        if (map[4] + map[5] + map[6] == 0) {
            mate_table[key] = false;
            return false;
        }
    }
    if (depth == 0) {
        return false;
    }
    std::shared_ptr<Node<T>> root;
    if (attack) {
        root = std::make_shared<OrNode<T>>(*this, nullptr);
    }
    else {
        root = std::make_shared<AndNode<T>>(*this, nullptr);
    }
    root->expand();
    int count = 0;
    while (true) {
        if (root->pn() == 0) {
            mate_table[key] = true;
            return true;
        }
        else if (root->dn() == 0) {
            mate_table[key] = false;
            return false;
        }
        //std::cout << "\t\t" << count << std::endl;
        if (++count == mate_threshold) {
            return false;
        }
        auto node = root->search();
        while (!node->isLeaf()) {
            node = node->search();
        }
        //node->debug();
        node->expand();
        node->update();
    }
}

template <typename T>
template <bool attack>
std::vector<move_t> Connect6<T>::generateMateMoves() const {
    std::bitset<kBoardSize> b1;
    std::bitset<kBoardSize> b2;
    std::bitset<kBoardSize> b3;
    if constexpr (attack) {
        b1 = generate<1>(player_) & (~(player_ | opponent_));
        b2 = generate<2>(player_) & (~(player_ | opponent_ | b1));
        b3 = generate<3>(player_) & (~(player_ | opponent_ | b1 | b2));
    }
    else {
        b1 = generate<1>(opponent_) & (~(player_ | opponent_));
        b2 = generate<2>(opponent_) & (~(player_ | opponent_ | b1));
    }
    std::vector<move_t> moves;
    moves.reserve(kBoardSize);
    for (int i = 0; i < kBoardSize; i++) {
        if (b1[i]) {
            moves.push_back(i);
        }
    }
    for (int i = 0; i < kBoardSize; i++) {
        if (b2[i]) {
            moves.push_back(i);
        }
    }
    if constexpr (attack) {
        for (int i = 0; i < kBoardSize; i++) {
            if (b3[i]) {
                moves.push_back(i);
            }
        }
    }
    return moves;
}

template <typename T>
template <bool attack>
std::vector<typename Connect6<T>::moves_t> Connect6<T>::countValidMatePosition(int ren3, int ren4, int ren5, int ren6) const {
    std::vector<moves_t> list;
    if constexpr (!attack) {
        if (isMate(true, 0)) {
            return {};
        }
    }
    if (isPlayerChanged()) {
        list.reserve(256);
        for (auto e3 : generateMateMoves<attack>()) {
            auto c3 = *this;
            c3.move(e3);
            auto map = c3.template getRen<attack>();
            auto ren6_3 = map[6];
            auto ren5_3 = map[5] + ren6_3;
            auto ren4_3 = map[4] + ren5_3;
            auto ren3_3 = map[3] + ren4_3;
            if constexpr (attack) {
                if (!(ren3 < ren3_3 || ren4 < ren4_3 ||
                            ren5 < ren5_3 || ren6 < ren6_3)) {
                    continue;
                }
            }
            else {
                if (ren4_3 == 0) {
                    return {};
                }
                if (!(ren4 > ren4_3)) {
                    continue;
                }
            }
            list.push_back({e3, c3});
        }
    }
    else {
        list.reserve(16);
        for (auto e4 : generateMateMoves<attack>()) {
            auto c4 = *this;
            c4.move(e4);
            auto map = c4.template getRen<!attack>();
            auto ren6_4 = map[6];
            auto ren5_4 = map[5] + ren6_4;
            auto ren4_4 = map[4] + ren5_4;
            if constexpr (attack) {
                if (!(ren4 < ren4_4 || ren6 < ren6_4)) {
                    continue;
                }
            }
            else {
                if (!(ren4 > ren4_4)) {
                    continue;
                }
            }
            list.push_back({e4, c4});
        }
    }
    return list;
}

template <typename T>
void Connect6<T>::dump() const {
    std::cout << "---------------" << std::endl;
    for (int y = 0; y < kSize; y++) {
        for (int x = 0; x < kSize; x++) {
            if (player_[y * kSize + x]) {
                std::cout << (isBlack() ? "1" : "2");
            }
            else if (opponent_[y * kSize + x]) {
                std::cout << (isBlack() ? "2" : "1");
            }
            else {
                std::cout << " ";
            }
        }
        std::cout << std::endl;
    }
    std::cout << "---------------" << std::endl;
}

template <typename T>
std::string Connect6<T>::board() const {
    std::ostringstream oss;
    for (int i = 0; i < kBoardSize; i++) {
        if (player_[i]) {
            oss << (isBlack() ? "1" : "2");
        }
        else if (opponent_[i]) {
            oss << (isBlack() ? "2" : "1");
        }
        else {
            oss << "0";
        }
        oss << ",";
    }
    return oss.str();
}

template class Connect6<score_t>;
