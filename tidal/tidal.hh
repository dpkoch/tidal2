// Copyright 2024 Daniel Koch

#pragma once

#include <fstream>
#include <memory>
#include <string>
#include <type_traits>

#include <Eigen/Core>

#include "tidal/detail.hh"

namespace tidal {

class Log {
   public:
    template <typename... Types>
    class Stream {
       public:
        void log(unsigned long timestamp, Types... data) {
            write_data_prefix(timestamp);
            write_data_recurse(data...);
        }

       private:
        using FieldLabels = std::array<std::string_view, sizeof...(Types)>;
        friend class Log;

        Stream(Log& log, unsigned int id, std::string_view name, const FieldLabels& field_labels)
            : log_(log), id_(id) {
            write_stream_metadata(name, field_labels);
        }

        void write_stream_metadata(std::string_view name, const FieldLabels& field_labels) {
            log_ << detail::Marker::STREAM_METADATA << id_ << name
                 << static_cast<int>(sizeof...(Types));
            write_field_metadata_recurse<0, Types...>(field_labels);
        }

        template <int index, typename First, typename... Tail>
        void write_field_metadata_recurse(const FieldLabels& field_labels) {
            log_ << detail::resolve_data_type<First>();
            write_additional_format_data<First>(log_);
            log_ << field_labels[index];

            write_field_metadata_recurse<index + 1, Tail...>(field_labels);
        }

        template <int index>
        void write_field_metadata_recurse(const FieldLabels& field_labels) {}

        template <typename T>
        std::enable_if_t<std::is_scalar_v<T>> write_additional_format_data(Log& log) {}

        template <typename T>
        std::enable_if_t<detail::is_eigen_v<T>> write_additional_format_data(Log& log) {
            EIGEN_STATIC_ASSERT_FIXED_SIZE(T);
            log << detail::resolve_scalar_type<typename T::Scalar>();
            if constexpr (T::IsVectorAtCompileTime) {
                log << T::RowsAtCompileTime;
            } else {
                log << T::RowsAtCompileTime << T::ColsAtCompileTime;
            }
        }

        void write_data_prefix(unsigned long timestamp) {
            log_ << detail::Marker::DATA << id_ << timestamp;
        }

        template <typename First, typename... Tail>
        void write_data_recurse(First value, Tail... tail) {
            log_ << value;
            write_data_recurse(tail...);
        }

        void write_data_recurse() {}

        Log& log_;
        unsigned int id_;
    };

    Log(const std::string& filename)
        : file_(filename, std::ios_base::out | std::ios_base::binary) {}
    ~Log() { file_.close(); }

    template <typename... Types>
    std::shared_ptr<Stream<Types...>> add_stream(
        std::string_view name, const std::array<std::string_view, sizeof...(Types)>& labels) {
        return std::shared_ptr<Stream<Types...>>(
            new Stream<Types...>(*this, next_id_++, name, labels));
    }

   private:
    Log& operator<<(std::string_view data) {
        file_.write(data.data(), sizeof(char) * data.length());
        file_ << '\0';  // null terminate strings
        return *this;
    }

    template <typename T, int rows, int cols>
    Log& operator<<(const Eigen::Matrix<T, rows, cols>& data) {
        using MatrixType = std::decay_t<decltype(data)>;
        EIGEN_STATIC_ASSERT_FIXED_SIZE(MatrixType);

        // By default, Eigen stores matrices in column-major order. Numpy stores them in
        // row-major order, and there is no easy way swap the order after reading from the
        // buffer. If this matrix is in column-major order and has more than one column, write
        // the transpose to the log.
        if constexpr (!MatrixType::IsRowMajor && cols > 1) {
            // Use assignment to a temporary matrix to force evaluation of transpose
            const Eigen::Matrix<T, cols, rows> tmp = data.transpose();
            file_.write(reinterpret_cast<const char*>(tmp.data()),
                        MatrixType::SizeAtCompileTime * sizeof(T));

        } else {
            file_.write(reinterpret_cast<const char*>(data.data()),
                        MatrixType::SizeAtCompileTime * sizeof(T));
        }
        return *this;
    }

    template <typename T>
    Log& operator<<(const T& data) {
        file_.write(reinterpret_cast<const char*>(&data), sizeof(T));
        return *this;
    }

    std::ofstream file_;
    unsigned int next_id_ = 0;
};

}  // namespace tidal