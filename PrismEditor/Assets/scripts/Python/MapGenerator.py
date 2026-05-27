from Prism import (
    Behaviour, Log, Time, Input, KeyCodes,
    TransformComponent, MeshComponent,
)
from Prism.Math import Vector2, Vector3, Vector4
from Prism.Math.Mathf import Mathf
from Prism.Math.Matrix4 import Matrix4
from Prism.Renderer import Color, Texture2D, MeshFactory
from Prism.Renderer.Material import MaterialInstance

import Noise as NoiseUtil


class MapGenerator(Behaviour):
    MapWidth: int = 128
    MapHeight: int = 128
    Octaves: int = 4
    Persistance: float = 0.74
    Seed: int = 21
    Lacunarity: float = 3.0
    Offset: Vector2 = Vector2(13.4, 6.26)
    NoiseScale: float = 0.5
    Speed: float = 0.0

    def GenerateMap(self):
        noiseMap = NoiseUtil.GenerateNoiseMap(
            self.MapWidth, self.MapHeight, self.Seed, self.NoiseScale,
            self.Octaves, self.Persistance, self.Lacunarity, self.Offset)

        width = len(noiseMap)
        height = len(noiseMap[0]) if width > 0 else 0

        texture = Texture2D(width, height)
        colorMap = []
        for y in range(height):
            for x in range(width):
                colorMap.append(Mathf.Lerp(Color.Black, Color.White, noiseMap[x][y]))

        texture.SetData(colorMap)

        Log.Trace("HasComponent - TransformComponent = {}", self.HasComponent(TransformComponent))
        Log.Trace("HasComponent - MeshComponent = {}", self.HasComponent(MeshComponent))

        meshComponent = self.GetComponent(MeshComponent)
        if meshComponent is None:
            Log.Trace("MeshComponent is null!")
            meshComponent = self.CreateComponent(MeshComponent)

        meshComponent.Mesh = MeshFactory.CreatePlane(1.0, 1.0)

        Log.Trace("Mesh has {} materials!", meshComponent.Mesh.GetMaterialCount())

        material = meshComponent.Mesh.GetMaterial(1)
        material.SetKeyword("ALBEDO_MAP", True)
        material.Set("u_AlbedoTexture", texture)

        transformComponent = self.GetComponent(TransformComponent)
        position = self.Entity.GetTransform().Translation
        transformComponent.Transform = Matrix4.Scale(Vector3(10.0, 1.0, 10.0)) * Matrix4.Translate(position)

    def OnCreate(self):
        self.GenerateMap()

    def OnUpdate(self):
        ts = Time.DeltaTime
        transform = self.Entity.GetTransform()
        translation = transform.Translation
        translation.y += ts * self.Speed
        if Input.IsKeyPressed(KeyCodes.Space):
            translation.y -= 10.0
        transform.Translation = translation
        self.Entity.SetTransform(transform)
