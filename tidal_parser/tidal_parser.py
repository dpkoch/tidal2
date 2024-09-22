# Copyright 2024 Daniel Koch

import collections
import dataclasses
import io
import struct
from collections.abc import Buffer, Callable

import numpy as np

from tidal_parser import detail


class ParserError(Exception):
    """Base class for parser module exceptions"""


class InvalidLogFile(ParserError):
    """Indicates the log file format was not valid"""


@dataclasses.dataclass(frozen=True)
class Stream:
    time: np.ndarray
    data: np.ndarray

    def data_items(self):
        if self.data.dtype.names is None:
            yield "data", self.data
            return

        for name in self.data.dtype.names:
            yield name, self.data[name]


class Parser:
    def __init__(self, filename: str):
        self._filename = filename

        self._data: dict[str, Stream] = {}  # the parsed data

        self._metadata = collections.defaultdict(dict)  # stream information by id
        self._time_bytestream = {}  # bytestream buckets for timestamps
        self._data_bytestream = {}  # bytestream buckets for data

        # binary format decoders
        self._read_stream_id = self._read_struct_type_func(
            detail.StructFormat.STREAM_ID
        )
        self._read_data_type = self._read_struct_type_func(
            detail.StructFormat.DATA_TYPE
        )
        self._read_data_size = self._read_struct_type_func(
            detail.StructFormat.DATA_SIZE
        )

        # parse the log file on construction
        self._parse()

        # convert parsed data into numpy arrays
        self._convert()

    def __getitem__(self, key):
        return self._data[key]

    def keys(self):
        return self._data.keys()

    def streams(self):
        return self._data.values()

    def items(self):
        return self._data.items()

    @staticmethod
    def _read_struct_type_func(
        t: detail.StructFormat,
    ) -> Callable[[Buffer], int]:
        return lambda f: struct.unpack(t, f.read(struct.calcsize(t)))[0]

    def _parse(self):
        with open(self._filename, "rb") as f:
            while True:
                try:
                    byte = f.read(1)
                    if byte == detail.Marker.DATA.value:
                        self._read_data(f)
                    elif byte == detail.Marker.METADATA.value:
                        self._read_metadata(f)
                    elif byte == detail.Marker.LABELS.value:
                        self._read_labels(f)
                    elif byte == detail.EOF:
                        break
                    else:
                        raise InvalidLogFile(f"Got unexpected byte {byte}")

                except EOFError:
                    break

    def _read_metadata(self, f):
        stream_id = self._read_stream_id(f)
        self._metadata[stream_id]["name"] = self._read_string(f)
        self._metadata[stream_id]["dtype"] = self._read_stream_format(f)

        # initialize bytestreams
        self._time_bytestream[stream_id] = io.BytesIO()
        self._data_bytestream[stream_id] = io.BytesIO()

    def _read_stream_format(self, f) -> np.dtype:
        num_entries = self._read_data_size(f)
        return np.dtype(
            ",".join([self._read_entry_format(f) for _ in range(num_entries)])
        )

    def _read_entry_format(self, f) -> np.dtype:
        data_type = self._read_data_type(f)
        if data_type in detail.SCALAR_DATA_TYPES:
            return detail.SCALAR_DATA_TYPE_TO_NUMPY_TYPE[data_type]
        if data_type == detail.DataType.VECTOR:
            return self._read_vector_format(f)
        if data_type == detail.DataType.MATRIX:
            return self._read_matrix_format(f)
        raise InvalidLogFile(f"Unsupported data type {data_type}")

    def _read_vector_format(self, f) -> str:
        scalar_type = self._read_data_type(f)
        size = self._read_data_size(f)
        return f"({size},){detail.SCALAR_DATA_TYPE_TO_NUMPY_TYPE[scalar_type]}"

    def _read_matrix_format(self, f) -> str:
        scalar_type = self._read_data_type(f)
        rows = self._read_data_size(f)
        cols = self._read_data_size(f)
        return f"({rows},{cols}){detail.SCALAR_DATA_TYPE_TO_NUMPY_TYPE[scalar_type]}"

    def _read_labels(self, f):
        stream_id = self._read_stream_id(f)
        self._metadata[stream_id]["labels"] = tuple(
            self._read_string(f) for _ in range(len(self._metadata[stream_id]["dtype"]))
        )
        self._metadata[stream_id]["dtype"].names = self._metadata[stream_id]["labels"]

    def _read_string(self, f) -> str:
        b = io.BytesIO()
        while True:
            byte = f.read(1)
            if byte == detail.NULL_TERMINATOR:
                break
            if byte == detail.EOF:
                raise InvalidLogFile(
                    "Reached end of file before string null terminator"
                )
            b.write(byte)
        return b.getvalue().decode()

    def _read_data(self, f):
        stream_id = self._read_stream_id(f)
        self._time_bytestream[stream_id].write(f.read(detail.TIMESTAMP_DTYPE.itemsize))
        self._data_bytestream[stream_id].write(
            f.read(self._metadata[stream_id]["dtype"].itemsize)
        )

    def _convert(self):
        for stream_id, metadata in self._metadata.items():
            self._data[metadata["name"]] = Stream(
                time=np.frombuffer(
                    self._time_bytestream[stream_id].getvalue(), detail.TIMESTAMP_DTYPE
                ),
                data=np.frombuffer(
                    self._data_bytestream[stream_id].getvalue(), metadata["dtype"]
                ),
            )
