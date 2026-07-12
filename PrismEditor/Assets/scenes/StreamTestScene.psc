Scene: Scene Name
Environment:
  Light:
    Direction: [-0.5, -1, -0.5]
    Radiance: [1, 1, 1]
    Multiplier: 1
  Shadow:
    Enabled: true
    Bias: 0.001
    NormalBias: 0.1
    CascadeCount: 4
    MaxDistance: 200
PhysicsLayers:
  []
Entities:
  - Entity: 16175966859253634730
    Parent: 0
    Children:
      []
    TagComponent:
      Tag: Empty Entity
    TransformComponent:
      Position: [0, 0, 0]
      Rotation: [0, 0, 0]
      Scale: [1, 1, 1]
    CSharpScriptComponent:
      Behaviours:
        - ClassID: 4883224417939687974
          Enabled: true
          Fields:
            - ID: 3811645116
              Name: mesh
              Type: 16
              Value: 0
            - ID: 3048789568
              Name: ID
              Type: 11
              Value: 0
    PythonScriptComponent:
      Behaviours:
        []