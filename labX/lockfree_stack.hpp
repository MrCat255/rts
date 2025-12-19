#pragma once

#include <vector>
#include <atomic>
#include <limits>
#include <algorithm>

#include "node.hpp"

namespace lf {

template<typename T>
class LockFreeVersionedStack {
public:
    typedef Node<T>* NodePtr;
    typedef std::atomic<Node<T>*> AtomicNodePtr;
    typedef std::atomic_uint64_t AtomicVersion;

    LockFreeVersionedStack(size_t readers_num) : subs_num_(readers_num) {
        stop_flag_.store(false);
        stack_.version.store(0);
        stack_.head.store(nullptr);

        subscribers_ = new AtomicVersion[readers_num];
        for (size_t i = 0; i < readers_num; i++) {
            subscribers_[i].store(0);
        }
    }

    ~LockFreeVersionedStack() {
        stop_flag_.store(true);

        // Удаляем все узлы из стека
        NodePtr current = stack_.head.load();
        while (current != nullptr) {
            NodePtr next = current->next;
            delete current;
            current = next;
        }

        // Удаляем узлы из мусорки
        for (auto& node : trash_) {
            delete node;
        }
        trash_.clear();

        delete[] subscribers_;
    }

    bool subscribe(const unsigned int& id, NodePtr& stack_ptr) {
        auto& sub = subscribers_[id];

        // Цикл ожидания новой версии
        uint64_t current_version = stack_.version.load(std::memory_order_relaxed);
        while (sub.load(std::memory_order_relaxed) == current_version) {
            if (stop_flag_.load()) {
                return false;
            }
            current_version = stack_.version.load(std::memory_order_relaxed);
        }

        auto readed_version = sub.load();
        while (!sub.compare_exchange_strong(readed_version, current_version)) {
            if (stop_flag_.load()) {
                return false;
            }
            current_version = stack_.version.load(std::memory_order_relaxed);
            readed_version = sub.load();
        }

        stack_ptr = stack_.head.load();
        return true;
    }

    void unsubscribe(const unsigned int& id) {
        subscribers_[id].store(0);
    }

    void push(T value) {
        NodePtr new_node = new Node<T>;
        new_node->data = std::move(value);
        new_node->next = stack_.head.load();
        new_node->version = stack_.version.load() + 1;

        stack_.head.store(new_node);
        stack_.version.fetch_add(1);
    }

    bool pop() {
        NodePtr old_node = stack_.head.load();
        if (old_node == nullptr) {
            return false;
        }

        NodePtr new_first_node = old_node->next;
        if (new_first_node != nullptr) {
            new_first_node->version = stack_.version.load() + 1;
        }

        stack_.head.store(new_first_node);
        stack_.version.fetch_add(1);

        update_trash(old_node);
        return true;
    }

    void stop() {
        stop_flag_.store(true);
    }

    bool is_stopped() {
        return stop_flag_.load();
    }

    uint64_t last_version() {
        return stack_.version.load();
    }

private:
    void update_trash(NodePtr old_node) {
        trash_.push_back(old_node);

        uint64_t min_version = std::numeric_limits<uint64_t>::max();
        for (size_t i = 0; i < subs_num_; ++i) {
            uint64_t version = subscribers_[i].load();
            if (version == 0) {
                continue;
            }
            min_version = std::min(min_version, version);
        }

        for (size_t i = 0; i < trash_.size();) {
            if (trash_[i]->version < min_version) {
                delete trash_[i];
                trash_.erase(trash_.begin() + i);
            } else {
                ++i;
            }
        }
    }

    struct VersionedHead {
        AtomicVersion version;
        AtomicNodePtr head;
    };

    VersionedHead stack_;
    AtomicVersion* subscribers_;
    size_t subs_num_;
    std::vector<NodePtr> trash_;
    std::atomic_bool stop_flag_;
};

}
