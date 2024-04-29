#ifndef AND_NODE_H_
#define AND_NODE_H_

#include "node.h"

template <typename T>
class OrNode;

template <typename T>
class AndNode : public Node<T> {
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
        AndNode(Connect6<T> c, Node<T> *parent) : Node<T>(c, parent) {
            auto map = c_.template getRen<false>();
            ren6_ = map[6];
            ren5_ = map[5] + ren6_;
            ren4_ = map[4] + ren5_;
            ren3_ = map[3] + ren4_;

            ren_ = (ren4_ << 8) + ren3_;
        }
        std::vector<typename Connect6<T>::moves_t> get() override {
            auto list = c_.template countValidMatePosition<false>(ren3_, ren4_, ren5_, ren6_);
            return list;
        }
        std::shared_ptr<Node<T>> create(Connect6<T> c) override {
            if (c_.isPlayerChanged()) {
                return std::make_shared<AndNode<T>>(c, this);
            }
            else {
                return std::make_shared<OrNode<T>>(c, this);
            }
        }
        void update() override {
            if (child_) {
                pn_ = kMate;
                dn_ = kInfinite;
                for (auto& e : child_.value()) {
                    auto& [m, n] = e;
                    pn_ += n->pn();
                    dn_ = std::min(dn_, n->dn());
                }
                std::stable_sort(child_->begin(), child_->end(), [](const typename Node<T>::node_t& a, const typename Node<T>::node_t& b) {
                            if (a.second->dn() >= kInfinite || b.second->dn() >= kInfinite) {
                                return a.second->dn() < b.second->dn();
                            }
                            if (a.second->ren() != b.second->ren()) {
                                return a.second->ren() < b.second->ren();
                            }
                            return a.second->dn() < b.second->dn();
                        });
            }
            mate_t key = {c_.getPosition(), false};
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
            expandInternal(false);
        }
};

#endif // AND_NODE_H_
