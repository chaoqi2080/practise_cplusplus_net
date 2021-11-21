//
// Created by chaoqi on 2021/11/21.
//

#ifndef PRACTISE_CPLUSPLUS_NET_CELL_TIMESTAMP_HPP
#define PRACTISE_CPLUSPLUS_NET_CELL_TIMESTAMP_HPP

#include <chrono>
using namespace std::chrono;

class CellTimestamp
{
public:
    void update()
    {
        _begin = high_resolution_clock::now();
    }

    double get_elapsed_seconds()
    {
        return get_elapsed_microseconds() * 0.000001;
    }

    double get_elapsed_milliseconds()
    {
        return get_elapsed_microseconds() * 0.001;
    }

    long long get_elapsed_microseconds()
    {
        return duration_cast<microseconds>(high_resolution_clock::now() - _begin).count() ;
    }
private:
    time_point<high_resolution_clock> _begin = high_resolution_clock::now();
};

#endif //PRACTISE_CPLUSPLUS_NET_CELL_TIMESTAMP_HPP
