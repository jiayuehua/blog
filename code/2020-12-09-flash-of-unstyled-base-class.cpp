// Compile with:
//     g++ -std=c++14 test.cpp -lgtest -lgtest_main

#include <gtest/gtest.h>
#include <cstdio>
#include <map>
#include <string>

#if USE_CXX20

#include <semaphore>
#include <thread>
#include <utility>

class OncePerSecond {
    std::thread t_;
    std::binary_semaphore sem_{0};
    bool stopped_ = false;

    virtual void do_action() { }

public:
    explicit OncePerSecond() {
        t_ = std::thread([this]() {
            do {
                do_action();
            } while (!sem_.try_acquire_for(std::chrono::seconds(1)));
        });
    }
    void stop() {
        if (!std::exchange(stopped_, true)) {
            sem_.release();
            t_.join();
        }
    }
    virtual ~OncePerSecond() { stop(); }
};

#else

#include <condition_variable>
#include <mutex>
#include <thread>
#include <utility>

class OncePerSecond {
    std::thread t_;
    std::mutex mtx_;
    std::condition_variable cv_;
    bool stopped_ = false;

    virtual void do_action() { }

public:
    explicit OncePerSecond() {
        t_ = std::thread([this]() {
            std::unique_lock<std::mutex> lk(mtx_);
            while (!stopped_) {
                do_action();
                cv_.wait_for(lk, std::chrono::seconds(1));
            }
        });
    }
    void stop() {
        std::unique_lock<std::mutex> lk(mtx_);
        if (!std::exchange(stopped_, true)) {
            lk.unlock();
            cv_.notify_all();
            t_.join();
        }
    }
    virtual ~OncePerSecond() { stop(); }
};

#endif  // USE_CXX20

int *production_state = nullptr;

class StatusDaemon : public OncePerSecond {
    std::map<int, int> config_;
    void do_action() override {
        printf("The global state is %d\n", *production_state);
    }
public:
    int config_size() const { return config_.size(); }
};

class NeuteredStatusDaemon : public StatusDaemon {
    void do_action() override { }
};

TEST(StatusDaemon, config_starts_empty) {
    NeuteredStatusDaemon sd;
    EXPECT_EQ(sd.config_size(), 0);
}
