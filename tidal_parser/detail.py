# Copyright 2024 Daniel Koch

import enum

import numpy as np


EOF = b""
NULL_TERMINATOR = b"\x00"
TIMESTAMP_DTYPE = np.dtype("uint64")


class Marker(enum.Enum):
    STREAM_METADATA = b"\xC3"
    DATA = b"\xA5"


class DataType(enum.IntEnum):
    U8 = 0
    I8 = 1
    U16 = 2
    I16 = 3
    U32 = 4
    I32 = 5
    U64 = 6
    I64 = 7
    F32 = 8
    F64 = 9
    BOOLEAN = 10
    VECTOR = 11
    MATRIX = 12


SCALAR_DATA_TYPE_TO_NUMPY_DTYPE: dict[DataType, type] = {
    DataType.U8: np.uint8,
    DataType.I8: np.int8,
    DataType.U16: np.uint16,
    DataType.I16: np.int16,
    DataType.U32: np.uint32,
    DataType.I32: np.int32,
    DataType.U64: np.uint64,
    DataType.I64: np.int64,
    DataType.F32: np.float32,
    DataType.F64: np.float64,
    DataType.BOOLEAN: np.bool,
}
SCALAR_DATA_TYPES: list[DataType] = list(SCALAR_DATA_TYPE_TO_NUMPY_DTYPE.keys())


class StructFormat(enum.StrEnum):
    """Format specifiers for struct package"""

    MARKER = "B"
    STREAM_ID = "I"
    DATA_TYPE = "B"
    DATA_SIZE = "I"
