Scene: Scene Name
Environment:
  AssetPath: E:\PrismEngine\Prism\PrismEditor\Assets\env\birchwood_4k.hdr
  Light:
    Direction: [0, 0, 0]
    Radiance: [0, 0, 0]
    Multiplier: 1
Entities:
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
  - Entity: 10654856600807563614
    TagComponent:
      Tag: Empty Entity
    TransformComponent:
      Position: [0, 0, 0]
      Rotation: [1, 0, 0, 0]
      Scale: [1, 1, 1]
    MeshComponent:
      AssetPath: E:\PrismEngine\Prism\PrismEditor\Assets\meshes\TestScene.fbx
  - Entity: 13982802161734492964
    TagComponent:
      Tag: Camera
    TransformComponent:
      Position: [26.25, 13.5, 0]
      Rotation: [0.6794104, -0.1920591, 0.68147516, 0.19263016]
      Scale: [0.9999976, 0.9999893, 0.9999899]
    CameraComponent:
      Camera: some camera data...
      Primary: true
  - Entity: 4272325128816557942
    TagComponent:
      Tag: Noise
    TransformComponent:
      Position: [0, 6.5, 0]
      Rotation: [1, 0, 0, 0]
      Scale: [9.25, 1, 10.25]
    ScriptComponent:
      ModuleName: Example.MapGenerator
      StoredFields:
        - Name: MapWidth
          Type: 2
          Data: 105
        - Name: MapHeight
          Type: 2
          Data: 116
        - Name: Offset
          Type: 5
          Data: [7.8, 7.2]
        - Name: Octaves
          Type: 2
          Data: 3
        - Name: Seed
          Type: 2
          Data: 24
        - Name: Persistance
          Type: 1
          Data: 6.2
        - Name: NoiseScale
          Type: 1
          Data: 4.2
        - Name: Lacunarity
          Type: 1
          Data: 5.2