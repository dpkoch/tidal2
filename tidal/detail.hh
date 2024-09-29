// Copyright 2024 Daniel Koch

#pragma once

#include <cstdint>
#include <type_traits>

#include <Eigen/Core>

namespace tidal {

class Log;

namespace detail {

enum class Marker : uint8_t {
    STREAM_METADATA = 0xC3,
    DATA = 0xA5,
};

enum class DataType : uint8_t {
    u8,
    i8,
    u16,
    i16,
    u32,
    i32,
    u64,
    i64,
    f32,
    f64,
    boolean,
    vector,
    matrix,
};

// clang-format off
template <typename T> constexpr DataType resolve_scalar_type() = delete;
template <> constexpr DataType resolve_scalar_type<uint8_t>() { return DataType::u8; }
template <> constexpr DataType resolve_scalar_type<int8_t>() { return DataType::i8; }
template <> constexpr DataType resolve_scalar_type<uint16_t>() { return DataType::u16; }
template <> constexpr DataType resolve_scalar_type<int16_t>() { return DataType::i16; }
template <> constexpr DataType resolve_scalar_type<uint32_t>() { return DataType::u32; }
template <> constexpr DataType resolve_scalar_type<int32_t>() { return DataType::i32; }
template <> constexpr DataType resolve_scalar_type<uint64_t>() { return DataType::u64; }
template <> constexpr DataType resolve_scalar_type<int64_t>() { return DataType::i64; }
template <> constexpr DataType resolve_scalar_type<float>() { return DataType::f32; }
template <> constexpr DataType resolve_scalar_type<double>() { return DataType::f64; }
template <> constexpr DataType resolve_scalar_type<bool>() { return DataType::boolean; }
// clang-format on

template <typename T>
struct is_eigen : std::false_type {};

template <typename T, int... Is>
struct is_eigen<Eigen::Matrix<T, Is...>> : std::true_type {};

template <typename T>
constexpr bool is_eigen_v = is_eigen<T>::value;

template <typename T>
constexpr std::enable_if_t<is_eigen_v<T>, DataType> resolve_data_type() {
    EIGEN_STATIC_ASSERT_FIXED_SIZE(T);

    if constexpr (T::IsVectorAtCompileTime) {
        return DataType::vector;
    } else {
        return DataType::matrix;
    }
}

template <typename T>
constexpr std::enable_if_t<std::is_scalar_v<T>, DataType> resolve_data_type() {
    return resolve_scalar_type<T>();
}

}  // namespace detail
}  // namespace tidal