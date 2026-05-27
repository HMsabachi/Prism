import PrismNative as _Prism
from Prism.Renderer.Mesh import Mesh


class MeshFactory:
    @staticmethod
    def CreatePlane(width: float, height: float) -> "Mesh":
        handle = _Prism.Prism_MeshFactory_CreatePlane(width, height)
        return Mesh(handle)
