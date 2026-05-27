import PrismNative as _Prism


class Material:
    def __init__(self, arg):
        if isinstance(arg, int):
            self._handle = arg
        else:
            self._handle = _Prism.Prism_Material_Constructor(arg)

    def __del__(self):
        if hasattr(self, '_handle') and self._handle != 0:
            _Prism.Prism_Material_Destructor(self._handle)
            self._handle = 0

    def SetFloat(self, uniform, value):
        _Prism.Prism_Material_SetFloat(self._handle, uniform, float(value))

    def SetTexture(self, uniform, texture):
        h = texture._handle if texture else 0
        _Prism.Prism_Material_SetTexture(self._handle, uniform, h)

    def Set(self, uniform, value):
        if hasattr(value, '_handle'):
            self.SetTexture(uniform, value)
        else:
            self.SetFloat(uniform, float(value))

    def SetKeyword(self, name, enabled):
        _Prism.Prism_Material_SetKeyword(self._handle, name, bool(enabled))

    def IsKeywordEnabled(self, name):
        return _Prism.Prism_Material_IsKeywordEnabled(self._handle, name)


class MaterialInstance:
    def __init__(self, arg):
        if isinstance(arg, int):
            self._handle = arg
        else:
            self._handle = _Prism.Prism_MaterialInstance_Constructor(arg._handle)

    def __del__(self):
        if hasattr(self, '_handle') and self._handle != 0:
            _Prism.Prism_MaterialInstance_Destructor(self._handle)
            self._handle = 0

    def SetFloat(self, uniform, value):
        _Prism.Prism_MaterialInstance_SetFloat(self._handle, uniform, float(value))

    def SetVector3(self, uniform, value):
        _Prism.Prism_MaterialInstance_SetVector3(self._handle, uniform, value)

    def SetVector4(self, uniform, value):
        _Prism.Prism_MaterialInstance_SetVector4(self._handle, uniform, value)

    def SetTexture(self, uniform, texture):
        h = texture._handle if texture else 0
        _Prism.Prism_MaterialInstance_SetTexture(self._handle, uniform, h)

    def Set(self, uniform, value):
        from Prism.Math.Vector3 import Vector3
        from Prism.Math.Vector4 import Vector4
        if isinstance(value, Vector3):
            self.SetVector3(uniform, value)
        elif isinstance(value, Vector4):
            self.SetVector4(uniform, value)
        elif hasattr(value, '_handle'):
            self.SetTexture(uniform, value)
        else:
            self.SetFloat(uniform, float(value))

    def SetKeyword(self, name, enabled):
        _Prism.Prism_MaterialInstance_SetKeyword(self._handle, name, bool(enabled))

    def IsKeywordEnabled(self, name):
        return _Prism.Prism_MaterialInstance_IsKeywordEnabled(self._handle, name)
