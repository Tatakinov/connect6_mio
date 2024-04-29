#ifndef NODE_H_
#define NODE_H_

#include <memory>
#include <optional>
#include <unordered_map>
#include <vector>

#include "connect6.h"

constexpr int kInfinite = 0xffff;
constexpr int kMate = 0;

namespace {
    std::unordered_map<position_t, move_t> mate_move_table;
    std::unordered_map<mate_t, bool, Hash> mate_table;
}

template <typename T>
class Node {
    protected:
        using node_t = std::pair<move_t, std::shared_ptr<Node<T>>>;
        Connect6<T> c_;
        Node<T> *parent_;
        int pn_, dn_, ren_;
        std::optional<std::vector<node_t>> child_;

        void expandInternal(bool attack) {
            if (!attack && c_.isMate(true, 0)) {
                mate_t key = {c_.getPosition(), attack};
                mate_table[key] = false;
                pn_ = kInfinite;
                dn_ = kMate;
                ren_ = kInfinite;
                return;
            }
            if (c_.isMate(attack, 0)) {
                mate_t key = {c_.getPosition(), attack};
                mate_table[key] = true;
                pn_ = kMate;
                dn_ = kInfinite;
                ren_ = kInfinite;
                return;
            }
            auto list = get();
            if (list.size() == 0) {
                mate_t key = {c_.getPosition(), attack};
                mate_table[key] = false;
                pn_ = kInfinite;
                dn_ = kMate;
                ren_ = kInfinite;
                return;
            }
            child_ = std::make_optional<std::vector<node_t>>();
            for (auto& e : list) {
                auto& [m, c] = e;
                auto child = create(c);
                child_->push_back({m, child});
            }
            std::stable_sort(child_->begin(), child_->end(), [=](const node_t& a, const node_t& b) {
                        if (attack) {
                            return a.second->ren() > b.second->ren();
                        }
                        else {
                            return a.second->ren() < b.second->ren();
                        }
                    });
        }
    public:
        Node<T>(Connect6<T> c, Node<T> *parent) : c_(c), parent_(parent), pn_(1), dn_(1) {}
        virtual ~Node<T>() {}
        void debug(std::string indent = "") {
            c_.dump();
            std::cout << indent << pn_ << ", " << dn_ << ", " << ren_ <<std::endl;
#if 0
            if (child_) {
                for (auto& c : child_.value()) {
                    c->debug(indent + "\t");
                }
            }
            else {
                c_.dump();
            }
#endif
        }
        inline bool isLeaf() const {
            return !child_;
        }
        virtual void update() {};
        virtual std::vector<typename Connect6<T>::moves_t> get() {
            return {};
        }
        virtual std::shared_ptr<Node<T>> create(Connect6<T> c) { return {}; }
        virtual void expand() {}
        std::shared_ptr<Node<T>> search() const {
            return child_.value()[0].second;
        }
        int pn() const {
            return pn_;
        }
        int dn() const {
            return dn_;
        }
        int ren() const {
            return ren_;
        }
};

#endif // NODE_H_
