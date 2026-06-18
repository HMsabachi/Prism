import PrismNative as _Prism


class Mesh:
    def __init__(self, filepath=""):
        if isinstance(filepath, int):
            self._handle = filepath
        else:
            self._handle = _Prism.Prism_Mesh_Constructor(filepath)

    def __del__(self):
        if hasattr(self, '_handle') and self._handle != 0:
            _Prism.Prism_Mesh_Destructor(self._handle)
            self._handle = 0