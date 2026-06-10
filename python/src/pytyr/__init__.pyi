from pathlib import Path

# Load public native dependency packages before this package loads native extensions.
import pypddl as pypddl
import pyyggdrasil as pyyggdrasil

from . import (
    formalism as formalism,
    planning as planning,
)

__version__: str

def native_prefix() -> Path: ...
def cmake_prefix() -> Path: ...
def cmake_dir() -> Path: ...
