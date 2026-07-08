from PrismEngine import *
from Prism.Component import MeshRendererComponent

class RuntimeMesh(Behaviour):
    mesh: Mesh = None
    _mesh_renderer: MeshRendererComponent = None
    _bound_mesh: Mesh = None
    def __init__(self) -> None:
        super().__init__()

    def OnCreate(self) -> None:
        self._mesh_renderer = self.GetComponent(MeshRendererComponent)
        if (self.mesh is not None) and (self._mesh_renderer is not None):
            self._bound_mesh = self.mesh
            self._mesh_renderer.Mesh = self.mesh
    
    def OnUpdate(self) -> None:
        if (self.mesh is not None) and (self._mesh_renderer is not None):
            if self._bound_mesh != self.mesh:
                self._bound_mesh = self.mesh
                self._mesh_renderer.Mesh = self.mesh