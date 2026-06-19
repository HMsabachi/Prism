using System;
using System.Runtime.InteropServices;

using Rolky.Managed.Interop;

namespace Prism
{

#pragma warning disable CS0649
    internal static unsafe class InternalCalls
    {
        internal static delegate* unmanaged[Cdecl]<Log.LogLevel, NativeString, void> Prism_Log_LogMessage;
        // Time
        internal static delegate* unmanaged[Cdecl]<float> Prism_Time_GetDeltaTime;
        internal static delegate* unmanaged[Cdecl]<float> Prism_Time_GetUnscaledDeltaTime;
        internal static delegate* unmanaged[Cdecl]<float> Prism_Time_GetTime;
        internal static delegate* unmanaged[Cdecl]<float> Prism_Time_GetUnscaledTime;
        internal static delegate* unmanaged[Cdecl]<float> Prism_Time_GetFixedDeltaTime;
        internal static delegate* unmanaged[Cdecl]<UInt64> Prism_Time_GetFrameCount;
        internal static delegate* unmanaged[Cdecl]<float, void> Prism_Time_SetTimeScale;
        internal static delegate* unmanaged[Cdecl]<float> Prism_Time_GetTimeScale;
        // Math
        internal static delegate* unmanaged[Cdecl]<float, float, float> Prism_Noise_PerlinNoise;
        // Input
        internal static delegate* unmanaged[Cdecl]<KeyCode, Bool32> Prism_Input_IsKeyPressed;
        internal static delegate* unmanaged[Cdecl]<MouseButton, Bool32> Prism_Input_IsMouseButtonPressed;
        internal static delegate* unmanaged[Cdecl]<Vector2*, void> Prism_Input_GetMousePosition;
        internal static delegate* unmanaged[Cdecl]<CursorMode, void> Prism_Input_SetCursorMode;
        internal static delegate* unmanaged[Cdecl]<CursorMode> Prism_Input_GetCursorMode;
        // Entity
        internal static delegate* unmanaged[Cdecl]<UInt64, Matrix4*, void> Prism_Entity_GetTransform;
        internal static delegate* unmanaged[Cdecl]<UInt64, Matrix4*, void> Prism_Entity_SetTransform;
        internal static delegate* unmanaged[Cdecl]<UInt64, ReflectionType, void> Prism_Entity_CreateComponent;
        internal static delegate* unmanaged[Cdecl]<UInt64, ReflectionType, Bool32> Prism_Entity_HasComponent;
        // MeshRendererComponent
        internal static delegate* unmanaged[Cdecl]<UInt64, IntPtr> Prism_MeshRendererComponent_GetMesh;
        internal static delegate* unmanaged[Cdecl]<UInt64, IntPtr, void> Prism_MeshRendererComponent_SetMesh;
        internal static delegate* unmanaged[Cdecl]<UInt64, IntPtr*, UInt64, void> Prism_MeshRendererComponent_GetMaterial;
        internal static delegate* unmanaged[Cdecl]<UInt64, IntPtr, UInt64, void> Prism_MeshRendererComponent_SetMaterial;
        internal static delegate* unmanaged[Cdecl]<UInt64, UInt64> Prism_MeshRendererComponent_GetMaterialCount;
        internal static delegate* unmanaged[Cdecl]<UInt64, IntPtr*, void> Prism_MeshRendererComponent_GetMaterials;
        internal static delegate* unmanaged[Cdecl]<UInt64, IntPtr*, UInt64, void> Prism_MeshRendererComponent_SetMaterials;
        // Mesh
        internal static delegate* unmanaged[Cdecl]<NativeString, IntPtr> Prism_Mesh_Constructor;
        internal static delegate* unmanaged[Cdecl]<IntPtr, void> Prism_Mesh_Destructor;
        internal static delegate* unmanaged[Cdecl]<float, float, IntPtr> Prism_MeshFactory_CreatePlane;
        // Texture2D
        internal static delegate* unmanaged[Cdecl]<UInt32, UInt32, IntPtr> Prism_Texture2D_Constructor;
        internal static delegate* unmanaged[Cdecl]<IntPtr, void> Prism_Texture2D_Destructor;
        internal static delegate* unmanaged[Cdecl]<IntPtr, NativeArray<Vector4>, Int32, void> Prism_Texture2D_SetData;
        // Material
        internal static delegate* unmanaged[Cdecl]<NativeString, IntPtr> Prism_Material_Constructor;
        internal static delegate* unmanaged[Cdecl]<IntPtr*, void> Prism_Material_GetDefaultMaterial;
        internal static delegate* unmanaged[Cdecl]<IntPtr, void> Prism_Material_Destructor;
        internal static delegate* unmanaged[Cdecl]<IntPtr, NativeString, float, void> Prism_Material_SetFloat;
        internal static delegate* unmanaged[Cdecl]<IntPtr, NativeString, int, void> Prism_Material_SetInt;
        internal static delegate* unmanaged[Cdecl]<IntPtr, NativeString, Bool32, void> Prism_Material_SetBool;
        internal static delegate* unmanaged[Cdecl]<IntPtr, NativeString, Vector3*, void> Prism_Material_SetColor3;
        internal static delegate* unmanaged[Cdecl]<IntPtr, NativeString, Vector4*, void> Prism_Material_SetColor;
        internal static delegate* unmanaged[Cdecl]<IntPtr, NativeString, Matrix4*, void> Prism_Material_SetMatrix4;
        internal static delegate* unmanaged[Cdecl]<IntPtr, NativeString, IntPtr, void> Prism_Material_SetTexture;
        internal static delegate* unmanaged[Cdecl]<IntPtr, NativeString, Vector2*, void> Prism_Material_SetVector2;
        internal static delegate* unmanaged[Cdecl]<IntPtr, NativeString, Vector3*, void> Prism_Material_SetVector3;
        internal static delegate* unmanaged[Cdecl]<IntPtr, NativeString, Vector4*, void> Prism_Material_SetVector4;
        internal static delegate* unmanaged[Cdecl]<IntPtr, NativeString, Bool32, void> Prism_Material_SetKeyword;
        internal static delegate* unmanaged[Cdecl]<IntPtr, NativeString, Bool32> Prism_Material_IsKeywordEnabled;

        // RigidBody2DComponent
        internal static delegate* unmanaged[Cdecl]<UInt64, Vector2*, Vector2*, Bool32, void> Prism_RigidBody2DComponent_ApplyLinearImpulse;
        internal static delegate* unmanaged[Cdecl]<UInt64, Vector2*, void> Prism_RigidBody2DComponent_GetLinearVelocity;
        internal static delegate* unmanaged[Cdecl]<UInt64, Vector2*, void> Prism_RigidBody2DComponent_SetLinearVelocity;

        // RigidBodyComponent
        internal static delegate* unmanaged[Cdecl]<UInt64, Vector3*, Int32, void> Prism_RigidBodyComponent_AddForce;
        internal static delegate* unmanaged[Cdecl]<UInt64, Vector3*, Int32, void> Prism_RigidBodyComponent_AddTorque;
        internal static delegate* unmanaged[Cdecl]<UInt64, Vector3*, void> Prism_RigidBodyComponent_GetLinearVelocity;
        internal static delegate* unmanaged[Cdecl]<UInt64, Vector3*, void> Prism_RigidBodyComponent_SetLinearVelocity;
        internal static delegate* unmanaged[Cdecl]<UInt64, Vector3*, void> Prism_RigidBodyComponent_Rotate;
        internal static delegate* unmanaged[Cdecl]<UInt64, UInt32> Prism_RigidBodyComponent_GetLayer;
        internal static delegate* unmanaged[Cdecl]<UInt64, float> Prism_RigidBodyComponent_GetMass;
        internal static delegate* unmanaged[Cdecl]<UInt64, float, void> Prism_RigidBodyComponent_SetMass;

        // Physics
        internal static delegate* unmanaged[Cdecl]<Vector3*, Vector3*, float, RaycastHit*, Bool32> Prism_Physics_Raycast;
        internal static delegate* unmanaged[Cdecl]<Vector3*, Vector3*, NativeArray<OverlapHitData>*, void> Prism_Physics_OverlapBox;
        internal static delegate* unmanaged[Cdecl]<Vector3*, float, NativeArray<OverlapHitData>*, void> Prism_Physics_OverlapSphere;

        // Entity
        internal static delegate* unmanaged[Cdecl]<NativeString, UInt64> Prism_Entity_FindEntityByTag;
        internal static delegate* unmanaged[Cdecl]<UInt64, NativeString, IntPtr> Prism_Entity_AddBehaviour;
        internal static delegate* unmanaged[Cdecl]<UInt64, ReflectionType, IntPtr> Prism_Entity_GetBehaviour;
        internal static delegate* unmanaged[Cdecl]<UInt64, UInt64, void> Prism_Entity_RemoveBehaviour;
        internal static delegate* unmanaged[Cdecl]<UInt64, Bool32> Prism_Behaviour_GetEnabled;
        internal static delegate* unmanaged[Cdecl]<UInt64, Bool32, void> Prism_Behaviour_SetEnabled;

        // TransformComponent
        internal static delegate* unmanaged[Cdecl]<UInt64, Vector3*, void> Prism_TransformComponent_GetPosition;
        internal static delegate* unmanaged[Cdecl]<UInt64, Vector3*, void> Prism_TransformComponent_GetRotation;
        internal static delegate* unmanaged[Cdecl]<UInt64, Vector3*, void> Prism_TransformComponent_GetScale;
        internal static delegate* unmanaged[Cdecl]<UInt64, Vector3, void> Prism_TransformComponent_SetPosition;
        internal static delegate* unmanaged[Cdecl]<UInt64, Vector3, void> Prism_TransformComponent_SetRotation;
        internal static delegate* unmanaged[Cdecl]<UInt64, Vector3, void> Prism_TransformComponent_SetScale;

    }
#pragma warning restore CS0649
}
