import argparse

import pytyr


def main() -> None:
    parser = argparse.ArgumentParser(
        prog="python -m pytyr",
        description="Print discovery paths for the bundled native tyr library.",
    )
    group = parser.add_mutually_exclusive_group(required=True)
    group.add_argument(
        "--prefix",
        action="store_true",
        help="Print the native prefix to put on CMAKE_PREFIX_PATH.",
    )
    group.add_argument(
        "--include-dir",
        action="store_true",
        help="Print the bundled C++ header include directory.",
    )
    group.add_argument(
        "--cmake-dir",
        action="store_true",
        help="Print the directory containing tyrConfig.cmake.",
    )
    group.add_argument(
        "--version",
        action="store_true",
        help="Print the pytyr version.",
    )
    args = parser.parse_args()

    if args.prefix:
        print(pytyr.cmake_prefix())
    elif args.include_dir:
        print(pytyr.native_prefix() / "include")
    elif args.cmake_dir:
        print(pytyr.cmake_dir())
    elif args.version:
        print(pytyr.__version__)


if __name__ == "__main__":
    main()
