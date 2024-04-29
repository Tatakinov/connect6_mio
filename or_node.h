#ifndef OR_NODE_H_
#define OR_NODE_H_

#include "node.h"

template <typename T>
class OrNode : public Node<T> {
    using Node<T>::c_;
    using Node<T>::parent_;
    using Node<T>::child_;
    using Node<T>::pn_;
    using Node<T>::dn_;
    using Node<T>::ren_;
    using Node<T>::expandInternal;
    private:
        int ren3_;
        int ren4_;
        int ren5_;
        int ren6_;
    public:
        OrNode(Connect6<T> c, Node<T> *parent) : Node<T>(c, parent) {
            auto map = c_.template getRen<true>();
            ren6_ = map[6];
            ren5_ = map[5] + ren6_;
            ren4_ = map[4] + ren5_;
            ren3_ = map[3] + ren4_;

            ren_ = (ren4_ << 8) + ren3_;
        }
        std::vector<typename Connect6<T>::moves_t> get() override {
            auto list = c_.template countValidMatePosition<true>(ren3_, ren4_, ren5_, ren6_);
            return list;
        }
        std::shared_ptr<Node<T>> create(Connect6<T> c) override {
            if (c_.isPlayerChanged()) {
                return std::make_shared<OrNode<T>>(c, this);
            }
            else {
                return std::make_shared<AndNode<T>>(c, this);
            }
        }
        void update() override {
            if (child_) {
                pn_ = kInfinite;
                dn_ = kMate;
                for (auto& e : child_.value()) {
                    auto& [m, n] = e;
                    pn_ = std::min(pn_, n->pn());
                    dn_ += n->dn();
                    if (n->pn() == 0) {
                        mate_move_table[c_.getPosition()] = m;
                    }
                }
                std::stable_sort(child_->begin(), child_->end(), [](const typename Node<T>::node_t& a, const typename Node<T>::node_t& b) {
                            if (a.second->pn() >= kInfinite || b.second->pn() >= kInfinite) {
                                return a.second->pn() < b.second->pn();
                            }
                            if (a.second->ren() != b.second->ren()) {
                                return a.second->ren() > b.second->ren();
                            }
                            return a.second->pn() < b.second->pn();
                        });
            }
            mate_t key = {c_.getPosition(), true};
            if (pn_ == 0) {
                mate_table[key] = true;
            }
            else if (dn_ == 0) {
                mate_table[key] = false;
            }
            if (parent_ != nullptr) {
                parent_->update();
            }
        }
        void expand() override {
            expandInternal(true);
        }
};

#endif // OR_NODE_H_
