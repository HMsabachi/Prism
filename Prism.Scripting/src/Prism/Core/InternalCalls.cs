using System;
using System.Runtime.InteropServices;

using Rolky.Managed.Interop;

namespace Prism
{


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
        internal static delegate* unmanaged[Cdecl]<KeyCode, bool> Prism_Input_IsKeyPressed;
        // Entity
        internal static delegate* unmanaged[Cdecl]<UInt64, Matrix4*, void> Prism_Entity_GetTransform;
        internal static delegate* unmanaged[Cdecl]<UInt64, Matrix4*, void> Prism_Entity_SetTransform;
        internal static delegate* unmanaged[Cdecl]<UInt64, ReflectionType, void> Prism_Entity_CreateComponent;
        internal static delegate* unmanaged[Cdecl]<UInt64, ReflectionType, bool> Prism_Entity_HasComponent;
        // Mesh
        internal static delegate* unmanaged[Cdecl]<UInt64, IntPtr> Prism_MeshComponent_GetMesh;
        internal static delegate* unmanaged[Cdecl]<UInt64, IntPtr, void> Prism_MeshComponent_SetMesh;
        internal static delegate* unmanaged[Cdecl]<NativeString, IntPtr> Prism_Mesh_Constructor;
        internal static delegate* unmanaged[Cdecl]<IntPtr, void> Prism_Mesh_Destructor;
        internal static delegate* unmanaged[Cdecl]<IntPtr, IntPtr> Prism_Mesh_GetMaterial;
        internal static delegate* unmanaged[Cdecl]<IntPtr, Int32, IntPtr> Prism_Mesh_GetMaterialByIndex;
        internal static delegate* unmanaged[Cdecl]<IntPtr, Int32> Prism_Mesh_GetMaterialCount;
        internal static delegate* unmanaged[Cdecl]<IntPtr, Int32, IntPtr, void> Prism_Mesh_SetMaterialByIndex;
        internal static delegate* unmanaged[Cdecl]<IntPtr, IntPtr, void> Prism_Mesh_SetOverrideMaterial;
        internal static delegate* unmanaged[Cdecl]<IntPtr, IntPtr> Prism_Mesh_GetOverrideMaterial;
        internal static delegate* unmanaged[Cdecl]<float, float, IntPtr> Prism_MeshFactory_CreatePlane;
        // MaterialComponent
        internal static delegate* unmanaged[Cdecl]<UInt64, IntPtr> Prism_MaterialComponent_GetMaterial;
        internal static delegate* unmanaged[Cdecl]<UInt64, IntPtr, void> Prism_MaterialComponent_SetMaterial;
        // Texture2D
        internal static delegate* unmanaged[Cdecl]<UInt32, UInt32, IntPtr> Prism_Texture2D_Constructor;
        internal static delegate* unmanaged[Cdecl]<IntPtr, void> Prism_Texture2D_Destructor;
        internal static delegate* unmanaged[Cdecl]<IntPtr, NativeArray<Vector4>, Int32, void> Prism_Texture2D_SetData;
        // Material
        internal static delegate* unmanaged[Cdecl]<NativeString, IntPtr> Prism_Material_Constructor;
        internal static delegate* unmanaged[Cdecl]<IntPtr, void> Prism_Material_Destructor;
        internal static delegate* unmanaged[Cdecl]<IntPtr, NativeString, float, void> Prism_Material_SetFloat;
        internal static delegate* unmanaged[Cdecl]<IntPtr, NativeString, IntPtr, void> Prism_Material_SetTexture;
        internal static delegate* unmanaged[Cdecl]<IntPtr, IntPtr> Prism_MaterialInstance_Constructor;
        internal static delegate* unmanaged[Cdecl]<IntPtr, void> Prism_MaterialInstance_Destructor;
        internal static delegate* unmanaged[Cdecl]<IntPtr, NativeString, float, void> Prism_MaterialInstance_SetFloat;
        internal static delegate* unmanaged[Cdecl]<IntPtr, NativeString, IntPtr, void> Prism_MaterialInstance_SetVector3;
        internal static delegate* unmanaged[Cdecl]<IntPtr, NativeString, IntPtr, void> Prism_MaterialInstance_SetVector4;
        internal static delegate* unmanaged[Cdecl]<IntPtr, NativeString, IntPtr, void> Prism_MaterialInstance_SetTexture;
        internal static delegate* unmanaged[Cdecl]<IntPtr, NativeString, bool, void> Prism_Material_SetKeyword;
        internal static delegate* unmanaged[Cdecl]<IntPtr, NativeString, bool> Prism_Material_IsKeywordEnabled;
        internal static delegate* unmanaged[Cdecl]<IntPtr, NativeString, bool, void> Prism_MaterialInstance_SetKeyword;
        internal static delegate* unmanaged[Cdecl]<IntPtr, NativeString, bool> Prism_MaterialInstance_IsKeywordEnabled;

        // RigidBody2DComponent
        internal static delegate* unmanaged[Cdecl]<UInt64, Vector2*, Vector2*, bool, void> Prism_RigidBody2DComponent_ApplyLinearImpulse;
        internal static delegate* unmanaged[Cdecl]<UInt64, Vector2*, void> Prism_RigidBody2DComponent_GetLinearVelocity;
        internal static delegate* unmanaged[Cdecl]<UInt64, Vector2*, void> Prism_RigidBody2DComponent_SetLinearVelocity;

        // Entity
        internal static delegate* unmanaged[Cdecl]<NativeString, UInt64> Prism_Entity_FindEntityByTag;


    }
}
