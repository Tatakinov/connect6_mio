#ifndef CONNECT6_H_
#define CONNECT6_H_

#include <bitset>
#include <fstream>
#include <iostream>
#include <sstream>
#include <tuple>
#include <unordered_map>
#include <vector>

#include "misc.h"

using position_t = std::pair<std::bitset<kBoardSize>, std::bitset<kBoardSize>>;
using mate_t = std::pair<position_t, bool>;

template<typename T>
class Connect6 {
    private:
        std::bitset<kBoardSize> player_, opponent_;
        int tesuu_;
        int random_move_factor_;

        T evaluate() const;
        T searchInternal(int depth, double alpha, double beta, bool search_mate) const;
        data_t<T> searchInternalRoot(int depth, double alpha, double beta, bool learn = false) const;
        template <bool attack>
        std::vector<move_t> generateMateMoves() const;

    public:
        using moves_t = std::pair<move2_t, Connect6<T>>;
        Connect6() : player_(0), opponent_(0),
            tesuu_(0), random_move_factor_(5) {}
        ~Connect6() {}
        inline bool operator==(const Connect6& rhs) const {
            const Connect6& lhs = *this;
            if (lhs.player_ != rhs.player_) {
                return false;
            }
            if (lhs.opponent_ != rhs.opponent_) {
                return false;
            }
            return true;
        }
        inline bool operator!=(const Connect6& rhs) const {
            const Connect6& lhs = *this;
            return !(lhs == rhs);
        }
        void init();
        inline position_t getPosition() const {
            return { player_, opponent_ };
        }
        void setPosition(std::string fen);
        inline void setRandomMoveFactor(int factor) {
            if (factor > 0) {
                random_move_factor_ = factor;
            }
        }
        inline int getTesuu() const { return tesuu_; }
        std::bitset<kBoardSize> getPlayer() const {
            return player_;
        }
        std::bitset<kBoardSize> getOpponent() const {
            return opponent_;
        }
        std::vector<move_t> generateMoves() const;
        std::vector<move_t> generateAIMoves() const;
        bool isMate(bool attack, int depth) const;
        data_t<T> search(int depth, bool learn = false) const {
            return searchInternalRoot(depth, -0.001, 1.001, learn);
        }
        void nullMove();
        void move(move_t move);
        inline bool isPlayerChanged() const {
            return tesuu_ & 0x01;
        }
        void dump() const;
        void dump(T score) const;
        State isGameOver() const;
        inline bool isBlack() const {
            return (tesuu_ + 1) % 4 < 2;
        }
        std::string board() const;
        template<bool attack>
        std::unordered_map<int, int> getRen() const;
        template <bool attack>
        std::vector<moves_t> countValidMatePosition(int ren3, int ren4, int ren5, int ren6) const;
};

template<>
struct std::hash<position_t> {
    inline size_t operator()(const position_t& key) const {
        auto& [p, o] = key;
        return std::hash<std::bitset<kBoardSize>>()(p) ^ std::hash<std::bitset<kBoardSize>>()(o);
    }
};

template <typename T>
struct std::hash<Connect6<T>> {
    inline size_t operator()(const Connect6<T>& key) const {
        return std::hash<position_t>()(key.getPosition());
    }
};

struct Hash {
    size_t operator()(const std::pair<position_t, bool>& a) const {
        auto& [position, attack] = a;
        auto& [p, o] = position;
        return std::hash<std::bitset<kBoardSize>>()(p) ^
            std::hash<std::bitset<kBoardSize>>()(o) ^
            std::hash<bool>()(attack);
    }
};

#endif // CONNECT6_H_
