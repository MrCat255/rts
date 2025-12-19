#include <cstddef>
#include <iostream>
#include <stdexcept>
#include <thread>
#include <vector>
#include <cmath>
#include <random>

#include "node.hpp"
#include "lockfree_stack.hpp"


struct Position {
    double x;
    double y;
};

// Параметры параболы: y = -(x^2) + 4x
constexpr double X_START = 0.0;
constexpr double X_END = 4.0;
constexpr double STEP = 0.001;
constexpr double EPSILON = 1e-6; // Коэфициент погрешности

size_t readers_num = 4;
lf::LockFreeVersionedStack<Position> stack(readers_num);

double calculateY(double x) {
    return -(x * x) + 4.0 * x;
}

// Проверка принадлежности точки параболе с учетом погрешности
bool isPointOnParabola(const Position& p) {
    double expected_y = calculateY(p.x);
    return std::abs(p.y - expected_y) < EPSILON;
}

void writer() {
    std::mt19937 gen(std::random_device{}());
    std::uniform_int_distribution<> action_dist(0, 10);
    std::uniform_int_distribution<> count_dist(1, 3);

    double current_x = X_START;

    while (current_x <= X_END && !stack.is_stopped()) {
        // Добавляем блок точек
        int push_count = count_dist(gen) * 100;
        for (int i = 0; i < push_count && current_x <= X_END; ++i) {
            Position p = {
                current_x,
                calculateY(current_x)
            };
            stack.push(p);
            current_x += STEP;
        }

        // Удаляем случайное количество точек
        if (action_dist(gen) > 6) {
            int pop_count = count_dist(gen) * 20;
            for (int i = 0; i < pop_count; ++i) {
                if (!stack.pop()) break;
            }
        }

        // Пауза для читателей
        std::this_thread::sleep_for(std::chrono::milliseconds(3));
    }

    stack.stop();
}

class Reader {
public:
    Reader(unsigned int id, lf::LockFreeVersionedStack<Position>* stack)
        : id_(id), stack_(stack) {}

    void life() {
        while (!stack_->is_stopped()) {
            auto data = read();
            if (!data.empty()) {
                check(data);
            }
        }
    }

    std::vector<Position> read() {
        lf::LockFreeVersionedStack<Position>::NodePtr data_ptr;
        if (!stack_->subscribe(id_, data_ptr)) {
            return {};
        }

        std::vector<Position> result;
        auto current = data_ptr;
        while (current != nullptr) {
            result.push_back(current->data);
            current = current->next;
        }
        versions_cnt++;

        // Отписываемся после чтения
        stack_->unsubscribe(id_);

        return result;
    }

    void check(const std::vector<Position>& data) {
        if (data.empty()) return;

        // Проверка 1: Все точки должны лежать на параболе
        for (const auto& point : data) {
            if (!isPointOnParabola(point)) {
                throw std::logic_error("The point does not lie on the parabola");
                return;
            }
        }

        // Проверка 2: Последовательность по X должна быть убывающей
        if (data.size() > 1) {
            for (size_t i = 1; i < data.size(); ++i) {
                if (data[i].x >= data[i-1].x) {
                    throw std::logic_error("The sequence of points is broken");
                    return;
                }
            }
        }
    }

    unsigned int versions_cnt = 0;

private:
    unsigned int id_;
    lf::LockFreeVersionedStack<Position>* stack_;
};


int main() {
    std::vector<Reader> readers;
    std::vector<std::thread> threads;

    // Создаем читателей
    for (size_t i = 0; i < readers_num; i++) {
        readers.emplace_back(i, &stack);
    }

    // Запускаем читающие потоки
    for (size_t i = 0; i < readers_num; i++) {
        threads.emplace_back(&Reader::life, &readers[i]);
    }

    writer();

    // Ожидаем завершения читающих потоков
    for (auto& t : threads) {
        t.join();
    }

    std::cout << "РЕЗУЛЬТАТЫ ИСПЫТАНИЯ:" << std::endl;
    std::cout << "Финальная версия стека: " << stack.last_version() << std::endl;
    std::cout << std::endl;

    for (size_t i = 0; i < readers_num; i++) {
        std::cout << "Читатель " << i << ":" << std::endl;
        std::cout << "  - Прочитано версий: " << readers[i].versions_cnt << std::endl;
    }

    return 0;
}
