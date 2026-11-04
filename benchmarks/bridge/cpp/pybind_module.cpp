#include <atomic>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <tuple>
#include <vector>

#include <pybind11/pybind11.h>

namespace py = pybind11;

namespace {

std::atomic<std::uint8_t> benchmarkSink{0};

void validateByteBuffer(const py::buffer_info& view) {
    if (view.itemsize != 1 || view.ndim != 1 || view.strides[0] != 1) {
        throw py::value_error("payload must be a contiguous one-dimensional byte buffer");
    }
}

void consumeBytes(const std::byte* bytes, std::size_t size) {
    std::uint8_t checksum = 0;
    for (std::size_t index = 0; index < size; ++index) {
        checksum =
            static_cast<std::uint8_t>(checksum + std::to_integer<std::uint8_t>(bytes[index]));
    }
    benchmarkSink.store(checksum, std::memory_order_relaxed);
}

std::tuple<std::uint64_t, std::uint64_t, std::int32_t>
exchangeView(std::uint64_t gameInstanceId, std::uint64_t sequence, const py::buffer& payload) {
    const py::buffer_info view = payload.request();
    validateByteBuffer(view);
    {
        py::gil_scoped_release release;
        consumeBytes(static_cast<const std::byte*>(view.ptr), static_cast<std::size_t>(view.size));
    }
    return {gameInstanceId, sequence, 7};
}

std::tuple<std::uint64_t, std::uint64_t, std::int32_t>
exchangeCopy(std::uint64_t gameInstanceId, std::uint64_t sequence, const py::buffer& payload) {
    const py::buffer_info view = payload.request();
    validateByteBuffer(view);
    std::vector<std::byte> copied(static_cast<std::size_t>(view.size));
    {
        py::gil_scoped_release release;
        std::memcpy(copied.data(), view.ptr, copied.size());
        consumeBytes(copied.data(), copied.size());
    }
    return {gameInstanceId, sequence, 7};
}

} // namespace

PYBIND11_MODULE(aegis_bridge_pybind, module) {
    module.doc() = "In-process PB-065 benchmark baseline";
    module.def("exchange_view", &exchangeView);
    module.def("exchange_copy", &exchangeCopy);
}
