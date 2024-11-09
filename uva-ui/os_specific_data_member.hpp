#pragma once

#include <cstddef>
#include <cstdint>

class os_specific_data_member
{
public:
    os_specific_data_member() = default;
    ~os_specific_data_member() = default;
protected:
    uint8_t os_specific_data[32];
public:
    template<typename T>
    T* os_specific_data_as() {
        return reinterpret_cast<T*>(os_specific_data);
    }
};