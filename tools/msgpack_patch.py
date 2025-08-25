#!/usr/bin/env python

import shutil
import os

def copy_file_to_directory(src_filename, dst_subdir):
    """
    Function to copy a file located in the same directory as the Python script
    to a specified subdirectory relative to the script's location.

    Args:
        src_filename (str): Name of the source file (located in the script's directory).
        dst_subdir (str): Subdirectory relative to the Python script's location.

    Returns:
        str: Path to the copied file.
    """
    try:
        # Get the directory of the current Python script
        script_dir = os.path.dirname(os.path.abspath(__file__))

        # Full path to the source file
        src_file = os.path.join(script_dir, src_filename)

        # Check if the source file exists
        if not os.path.exists(src_file):
            print(f"Source file does not exist: {src_file}")
            return None

        # Create the full path to the destination directory
        dst_dir = os.path.join(script_dir, dst_subdir)

        # Create the destination directory if it does not exist
        os.makedirs(dst_dir, exist_ok=True)

        # Create the destination file path
        dst_file = os.path.join(dst_dir, os.path.basename(src_file))

        # Copy the file
        shutil.copy(src_file, dst_file)
        print(f"Copied {src_file} to {dst_file}.")
        return dst_file
    except Exception as e:
        print(f"Failed to copy the file: {e}")
        return None

# By enabling the MAX_MSGPACK_MALLOC_SIZE macro, you can limit the total memory allocated by msgpack.
# CXXFLAGS=-DMAX_MSGPACK_MALLOC_SIZE=10000000
src1_file = 'cpp03_zone.hpp'
src2_file = 'cpp11_zone.hpp'
dst_dir = '../deps/msgpack/include/msgpack/v1/detail/'

copy_file_to_directory(src1_file, dst_dir)
copy_file_to_directory(src2_file, dst_dir)