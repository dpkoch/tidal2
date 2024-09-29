// Copyright 2024 Daniel Koch

#include <cstdint>
#include <cstdlib>
#include <filesystem>

#include <Eigen/Core>

#include "tidal/tidal.hh"

using Vec4 = Eigen::Vector4d;
using Mat2x3 = Eigen::Matrix<double, 2, 3>;

int main() {
    tidal::Log log(std::filesystem::temp_directory_path() / "meh.bin");
    auto stream1 = log.add_stream<int, bool, Vec4, Mat2x3, double>(
        "stuff", {"my_ints", "my_bools", "vectors", "matrices", "my_doubles"});
    stream1->log(4000, 42, true, Vec4(12., 4., -23., 5.), Mat2x3::Zero(), 123.);

    auto stream2 = log.add_stream<uint8_t, int8_t>("small stream", {"unsigned", "signed"});

    stream1->log(4001, 12, false, Vec4::Ones(), (Mat2x3() << 1, 2, 3, 4, 5, 6).finished(), -9000.);
    stream2->log(12345, 29, -2);

    return EXIT_SUCCESS;
}