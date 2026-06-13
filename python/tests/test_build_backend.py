import importlib.util
import os
import sys
import types
import zipfile
from contextlib import contextmanager
from pathlib import Path


def load_backend():
    sys.modules.setdefault("scikit_build_core", types.SimpleNamespace(build=types.SimpleNamespace()))
    backend_path = Path(__file__).resolve().parents[1] / "pytyr_build_backend.py"
    spec = importlib.util.spec_from_file_location("pytyr_build_backend", backend_path)
    assert spec is not None
    assert spec.loader is not None
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    return module


@contextmanager
def temporary_environment(updates):
    sentinel = object()
    old_values = {name: os.environ.get(name, sentinel) for name in updates}
    try:
        for name, value in updates.items():
            if value is None:
                os.environ.pop(name, None)
            else:
                os.environ[name] = value
        yield
    finally:
        for name, old_value in old_values.items():
            if old_value is sentinel:
                os.environ.pop(name, None)
            else:
                os.environ[name] = old_value


@contextmanager
def fake_providers(provider_prefixes):
    sentinel = object()
    old_modules = {name: sys.modules.get(name, sentinel) for name in provider_prefixes}
    try:
        for name, prefix in provider_prefixes.items():
            sys.modules[name] = types.SimpleNamespace(native_prefix=lambda prefix=prefix: prefix)
        yield
    finally:
        for name, old_module in old_modules.items():
            if old_module is sentinel:
                sys.modules.pop(name, None)
            else:
                sys.modules[name] = old_module


def test_num_jobs_uses_default_and_env_override():
    backend = load_backend()

    with temporary_environment({"TYR_JOBS": None}):
        assert backend._num_jobs() == 8
    with temporary_environment({"TYR_JOBS": "6"}):
        assert backend._num_jobs() == 6


def test_prepare_native_build_sets_expected_environment(tmp_path):
    backend = load_backend()
    pypddl_prefix = tmp_path / "pypddl_native"
    yggdrasil_prefix = tmp_path / "yggdrasil_native"
    (pypddl_prefix / "lib").mkdir(parents=True)
    (yggdrasil_prefix / "lib").mkdir(parents=True)

    with fake_providers({"pypddl": pypddl_prefix, "pyyggdrasil": yggdrasil_prefix}):
        with temporary_environment(
            {
                "CMAKE_ARGS": "-DUSER_OPTION=ON",
                "CMAKE_BUILD_PARALLEL_LEVEL": None,
                "LD_LIBRARY_PATH": "/existing/ld",
                "TYR_JOBS": "6",
            }
        ):
            backend._prepare_native_build()

            assert os.environ["CMAKE_BUILD_PARALLEL_LEVEL"] == "6"
            assert os.environ["LD_LIBRARY_PATH"].startswith(str((pypddl_prefix / "lib").resolve()))
            assert os.environ["LD_LIBRARY_PATH"].endswith("/existing/ld")

            cmake_args = os.environ["CMAKE_ARGS"]
            expected_prefix_path = f"{pypddl_prefix.resolve()};{yggdrasil_prefix.resolve()}"
            assert f"-DCMAKE_PREFIX_PATH={expected_prefix_path}" in cmake_args
            assert f"-DYGGDRASIL_NATIVE_PREFIX={yggdrasil_prefix.resolve()}" in cmake_args
            assert "-DTYR_BUILD_PYTYR=ON" in cmake_args
            assert "-DTYR_HEADER_INSTANTIATION=OFF" in cmake_args
            assert cmake_args.endswith("-DUSER_OPTION=ON")


def test_fix_wheel_stubs_rewrites_private_references(tmp_path):
    backend = load_backend()
    wheel_path = tmp_path / "pytyr-0.0.0-py3-none-any.whl"
    with zipfile.ZipFile(wheel_path, "w") as wheel:
        wheel.writestr("pytyr/__init__.pyi", "ref: pytyr._pytyr.planning pyyggdrasil._pyyggdrasil\n")
        wheel.writestr("pytyr-0.0.0.dist-info/RECORD", "")

    backend._fix_wheel_stubs(wheel_path)

    with zipfile.ZipFile(wheel_path) as wheel:
        stub = wheel.read("pytyr/__init__.pyi").decode()
    assert stub == "ref: pytyr.planning pyyggdrasil\n"
