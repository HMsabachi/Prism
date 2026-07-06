import PrismNative as _Prism


class Texture2D:
    def __init__(self, width, height):
        self._handle = _Prism.Prism_Texture2D_Constructor(int(width), int(height))

    def __del__(self):
        if hasattr(self, '_handle') and self._handle != 0:
            _Prism.Prism_Texture2D_Destructor(self._handle)
            self._handle = 0

    def SetData(self, data):
        raise NotImplementedError("Texture2D.SetData is not yet implemented in Python bindings")
