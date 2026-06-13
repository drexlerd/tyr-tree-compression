"""PEP 517 backend for the pytyr wheel; machinery lives in pyyggdrasil.build_support."""

import os

from pyyggdrasil.build_support import ProviderBackend


def _env_cmake_defines():
    return [
        f"-DTYR_HEADER_INSTANTIATION={os.environ.get('TYR_HEADER_INSTANTIATION', 'OFF')}",
        f"-DTYR_ENABLE_LTO={os.environ.get('TYR_ENABLE_LTO', 'OFF')}",
    ]


_BACKEND = ProviderBackend(
    package="pytyr",
    providers=("pypddl", "pyyggdrasil"),
    cmake_defines=(
        "-DTYR_BUILD_PYTYR=ON",
        "-DTYR_BUILD_TESTS=OFF",
        "-DTYR_BUILD_EXECUTABLES=OFF",
        "-DTYR_BUILD_PROFILING=OFF",
        "-DTYR_USE_LLD=OFF",
        "-DCMAKE_INSTALL_LIBDIR=lib",
    ),
    extra_cmake_defines=_env_cmake_defines,
    rename_packages=("pytyr", "pypddl", "pyyggdrasil"),
    jobs_env="TYR_JOBS",
    strip_env="PYTYR_STRIP_WHEEL",
)

_num_jobs = _BACKEND._num_jobs
_prepare_native_build = _BACKEND._prepare_native_build
_fix_wheel_stubs = _BACKEND._fix_wheel_stubs
_strip_wheel_native_libraries = _BACKEND._strip_wheel_native_libraries

_BACKEND.install_hooks(globals())
