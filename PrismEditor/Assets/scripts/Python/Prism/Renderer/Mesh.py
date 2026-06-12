import PrismNative as _Prism
from Prism.Renderer.Material import MaterialInstance

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

    @property
    def BaseMaterial(self):
        from Prism.Renderer.Material import Material
        handle = _Prism.Prism_Mesh_GetMaterial(self._handle)
        return Material(handle) if handle else None

    def GetMaterial(self, index) -> 'MaterialInstance':
        from Prism.Renderer.Material import MaterialInstance
        handle = _Prism.Prism_Mesh_GetMaterialByIndex(self._handle, int(index))
        return MaterialInstance(handle) if handle else None

    def GetMaterialCount(self):
        return _Prism.Prism_Mesh_GetMaterialCount(self._handle)

    def SetMaterial(self, index, material):
        h = material._handle if material else 0
        _Prism.Prism_Mesh_SetMaterialByIndex(self._handle, int(index), h)

    def SetOverrideMaterial(self, material):
        h = material._handle if material else 0
        _Prism.Prism_Mesh_SetOverrideMaterial(self._handle, h)

    def GetOverrideMaterial(self):
        from Prism.Renderer.Material import MaterialInstance
        handle = _Prism.Prism_Mesh_GetOverrideMaterial(self._handle)
        return MaterialInstance(handle) if handle else None
