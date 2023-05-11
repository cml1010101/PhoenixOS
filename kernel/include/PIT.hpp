#ifndef PIT_HPP
#define PIT_HPP
#include <PhoenixOS.hpp>
#include <Timer.hpp>
class PIT : public Timer
{
private:
    size_t frequency;
    volatile size_t count;
    TimerHandler handler;
    static PIT* instance;
public:
    inline PIT()
    {
        frequency = 0;
        count = 0;
        handler = NULL;
    }
    inline void incrementCount()
    {
        count++;
    }
    void start() override;
    void stop() override;
    void sleep(uint64_t microseconds);
    void setFrequency(size_t frequency);
    size_t getCount() override;
    size_t getFrequency() override;
    double getMicroseconds() override;
    void setInterruptHandler(TimerHandler handler) override;
    inline TimerHandler getHandler()
    {
        return handler;
    }
    static inline PIT* getInstance()
    {
        return instance;
    }
};
#endif