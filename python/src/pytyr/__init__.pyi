from pathlib import Path

from . import (
    formalism as formalism,
    planning as planning,
)

__version__: str

def native_prefix() -> Path: ...
