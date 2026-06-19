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

    def SetInt(self, uniform, value):
        _Prism.Prism_Material_SetInt(self._handle, uniform, int(value))

    def SetBool(self, uniform, value):
        _Prism.Prism_Material_SetBool(self._handle, uniform, bool(value))

    def SetVec2(self, uniform, value):
        _Prism.Prism_Material_SetVector2(self._handle, uniform, value)

    def SetColor3(self, uniform, value):
        _Prism.Prism_Material_SetColor3(self._handle, uniform, value)

    def SetColor(self, uniform, value):
        _Prism.Prism_Material_SetColor(self._handle, uniform, value)

    def SetMatrix4(self, uniform, value):
        _Prism.Prism_Material_SetMatrix4(self._handle, uniform, value)

    def SetVector3(self, uniform, value):
        _Prism.Prism_Material_SetVector3(self._handle, uniform, value)

    def SetVector4(self, uniform, value):
        _Prism.Prism_Material_SetVector4(self._handle, uniform, value)

    def SetTexture(self, uniform, texture):
        h = texture._handle if texture else 0
        _Prism.Prism_Material_SetTexture(self._handle, uniform, h)

    def SetKeyword(self, name, enabled):
        _Prism.Prism_Material_SetKeyword(self._handle, name, bool(enabled))

    def IsKeywordEnabled(self, name):
        return _Prism.Prism_Material_IsKeywordEnabled(self._handle, name)
