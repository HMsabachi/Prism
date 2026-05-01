Scene: Scene Name
Environment:
  AssetPath: E:\PrismEngine\Prism\PrismEditor\Assets\env\birchwood_4k.hdr
  Light:
    Direction: [0, 0, 0]
    Radiance: [0, 0, 0]
    Multiplier: 1
Entities:
  - Entity: 4272325128816557942
    TagComponent:
      Tag: Noise
    TransformComponent:
      Position: [0, 0, 0]
      Rotation: [1, 0, 0, 0]
      Scale: [1, 1, 1]
    ScriptComponent:
      ModuleName: Example.MapGenerator
      StoredFields:
        - Name: MapWidth
          Type: 2
          Data: 0
        - Name: MapHeight
          Type: 2
          Data: 0
        - Name: Offset
          Type: 5
          Data: [0, 0]
        - Name: Octaves
          Type: 2
          Data: 0
        - Name: Seed
          Type: 2
          Data: 0
        - Name: Persistance
          Type: 1
          Data: 0
        - Name: NoiseScale
          Type: 1
          Data: 0
        - Name: Lacunarity
          Type: 1
          Data: 0
  - Entity: 13982802161734492964
    TagComponent:
      Tag: Camera
    TransformComponent:
      Position: [26.25, 13.5, 0]
      Rotation: [0.6794104, -0.1920591, 0.68147516, 0.19263016]
      Scale: [0.9999977, 0.9999893, 0.9999899]
    CameraComponent:
      Camera: some camera data...
      Primary: true
  - Entity: 10654856600807563614
    TagComponent:
      Tag: Empty Entity
    TransformComponent:
      Position: [0, 0, 0]
      Rotation: [1, 0, 0, 0]
      Scale: [1, 1, 1]
    MeshComponent:
      AssetPath: E:\PrismEngine\Prism\PrismEditor\Assets\meshes\TestScene.fbx
  - Entity: 10266587003235082940
    TagComponent:
      Tag: Script
    TransformComponent:
      Position: [0, 0, 0]
      Rotation: [1, 0, 0, 0]
      Scale: [1, 1, 1]
    ScriptComponent:
      ModuleName: Example.Script
      StoredFields:
        - Name: Speed
          Type: 1
          Data: 4.4