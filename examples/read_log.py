# Copyright 2024 Daniel Koch

from tidal_parser import Parser


if __name__ == "__main__":
    # import debugpy
    # debugpy.listen(5678)
    # debugpy.wait_for_client()

    p = Parser("/tmp/meh.bin")

    print("Found the following streams:")
    for stream_name, stream in p.items():
        print(f"\n======== {stream_name} ========")
        print(f"time: {stream.time}")
        for name, data in stream.data_items():
            print(f"{name}: {data}")
