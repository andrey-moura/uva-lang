#pragma once

#include <cstddef>
#include <cstdint>

template<const size_t DATA_SIZE>
class os_specific_data_member
{
public:
    os_specific_data_member() = default;
    ~os_specific_data_member() = default;
protected:
    uint8_t m_os_specific_data[DATA_SIZE];
public:
    template<typename T>
    const T& os_specific_data_as() const {
        static_assert(sizeof(T) <= DATA_SIZE, "in os_specific_data_as sizeof(T) is greater than DATA_SIZE");
        return *(reinterpret_cast<T*>(m_os_specific_data));
    }
    template<typename T>
    T& os_specific_data_as() {
        static_assert(sizeof(T) <= DATA_SIZE, "in os_specific_data_as sizeof(T) is greater than DATA_SIZE");
        return *(reinterpret_cast<T*>(m_os_specific_data));
    }
};