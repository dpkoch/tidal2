// Copyright 2024 Daniel Koch

#include <cstdint>
#include <cstdlib>
#include <filesystem>

#include <Eigen/Core>

#include "tidal/tidal.hh"

using Vec4 = Eigen::Vector4d;
using Mat3 = Eigen::Matrix3f;

int main() {
    tidal::Log log(std::filesystem::temp_directory_path() / "meh.bin");
    auto stream1 = log.add_stream<int, bool, Vec4, Mat3, double>("stuff");
    stream1->set_labels("int", "bool", "Vec4", "Mat3", "double");
    stream1->log(4000, 42, true, Vec4(12., 4., -23., 5.), Mat3::Random(), 123.);

    auto stream2 = log.add_stream<uint8_t>("small stream");

    stream1->log(4001, 12, false, Vec4::Ones(), Mat3::Zero(), -9000.);
    stream2->log(12345, 29);

    return EXIT_SUCCESS;
}