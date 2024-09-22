// Copyright 2024 Daniel Koch

#include <fstream>
#include <memory>
#include <string>
#include <type_traits>

#include <Eigen/Core>

#include "tidal/log_format.hh"

namespace tidal {

class Log {
   public:
    template <typename... Types>
    class Stream {
       public:
        Stream(Log& log, unsigned int id) : log_(log), id_(id) {}

        template <typename... Labels>
        typename std::enable_if<sizeof...(Labels) == sizeof...(Types)>::type set_labels(
            Labels... labels) {
            log_ << detail::Marker::LABELS << id_;
            label_recurse(labels...);
        }

        void log(unsigned long timestamp, Types... data) {
            write_data_prefix(timestamp);
            log_recurse(data...);
        }

       private:
        void write_header(const std::string& name) {
            log_ << detail::Marker::METADATA << id_ << name;
            write_format();
        }

        void write_format() {
            log_ << static_cast<uint32_t>(sizeof...(Types));
            format_recurse<Types...>();
        }

        template <typename First, typename... Tail>
        void format_recurse() {
            log_ << detail::resolve_data_type<First>();
            write_additional_format_data<First>(log_);
            format_recurse<Tail...>();
        }

        template <typename... Tail>
        typename std::enable_if<sizeof...(Tail) == 0>::type format_recurse() {}

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

        template <typename... Labels>
        void label_recurse(const std::string& first, Labels... labels) {
            log_ << first;
            label_recurse(labels...);
        }

        template <typename... Tail>
        typename std::enable_if<sizeof...(Tail) == 0>::type label_recurse() {}

        void write_data_prefix(unsigned long timestamp) {
            log_ << detail::Marker::DATA << id_ << timestamp;
        }

        template <typename First, typename... Tail>
        void log_recurse(First value, Tail... tail) {
            log_ << value;
            log_recurse(tail...);
        }

        template <typename... Tail>
        void log_recurse(Tail... tail) {}

       private:
        friend class Log;
        Log& log_;
        unsigned int id_;
    };

    Log(const std::string& filename)
        : file_(filename, std::ios_base::out | std::ios_base::binary) {}
    ~Log() { file_.close(); }

    template <typename... Types>
    std::shared_ptr<Stream<Types...>> add_stream(const std::string& name) {
        auto stream = std::make_shared<Stream<Types...>>(*this, next_id_++);
        stream->write_header(name);
        return stream;
    }

   private:
    Log& operator<<(const std::string& data) {
        file_.write(data.c_str(), sizeof(char) * data.size());
        file_ << '\0';  // null terminate strings
        return *this;
    }

    template <typename T, unsigned int rows, unsigned int cols>
    Log& operator<<(const Eigen::Matrix<T, rows, cols>& data) {
        file_.write(data.data(), data.size());
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