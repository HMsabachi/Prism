Scene: Scene Name
Environment:
  AssetPath: assets\env\pink_sunrise_4k.hdr
  Light:
    Direction: [-0.787, -0.73299998, 1]
    Radiance: [1, 1, 1]
    Multiplier: 0.514999986
Entities:
  - Entity: 10169503531257462571
    TagComponent:
      Tag: Box
    TransformComponent:
      Position: [0, 1.5, 0]
      Rotation: [1, 0, 0, 0]
      Scale: [2, 2, 2]
    MeshComponent:
      AssetPath: assets\meshes\Cube1m.fbx
    RigidBodyComponent:
      BodyType: 1
      Mass: 0.5
    PhysicsMaterialComponent:
      StaticFriction: 1
      DynamicFriction: 1
      Bounciness: 0
    BoxColliderComponent:
      Offset: [0, 0, 0]
      Size: [2, 2, 2]
  - Entity: 14057422478420564497
    TagComponent:
      Tag: Player
    TransformComponent:
      Position: [-19.43363, 4.50874043, -1.96695328e-06]
      Rotation: [1, 0, 0, 0]
      Scale: [1, 1, 1]
    ScriptComponent:
      ModuleName: Example.PlayerSphere
      StoredFields:
        - Name: HorizontalForce
          Type: 1
          Data: 10
        - Name: MaxSpeed
          Type: 6
          Data: [10, 10, 10]
        - Name: JumpForce
          Type: 1
          Data: 200
    MeshComponent:
      AssetPath: assets\meshes\Sphere1m.fbx
    RigidBodyComponent:
      BodyType: 1
      Mass: 1
    PhysicsMaterialComponent:
      StaticFriction: 1
      DynamicFriction: 1
      Bounciness: 0
    SphereColliderComponent:
      Radius: 0.5
  - Entity: 5178862374589434728
    TagComponent:
      Tag: Camera
    TransformComponent:
      Position: [-21.7406311, 9.70659542, 15]
      Rotation: [0.999910355, -0.0133911213, 0, 0]
      Scale: [1, 1, 1]
    ScriptComponent:
      ModuleName: Example.BasicController
      StoredFields:
        - Name: Speed
          Type: 1
          Data: 12
        - Name: DistanceFromPlayer
          Type: 1
          Data: 15
    CameraComponent:
      Camera: some camera data...
      Primary: true
  - Entity: 18306113171518048249
    TagComponent:
      Tag: Box
    TransformComponent:
      Position: [0, 0, 0]
      Rotation: [1, 0, 0, 0]
      Scale: [50, 1, 50]
    MeshComponent:
      AssetPath: assets\meshes\Cube1m.fbx
    RigidBodyComponent:
      BodyType: 0
      Mass: 1
    PhysicsMaterialComponent:
      StaticFriction: 1
      DynamicFriction: 1
      Bounciness: 0
    BoxColliderComponent:
      Offset: [0, 0, 0]
      Size: [50, 1, 50]