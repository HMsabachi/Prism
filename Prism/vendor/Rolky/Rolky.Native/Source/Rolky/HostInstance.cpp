#include "Rolky/HostInstance.hpp"
#include "Rolky/StringHelper.hpp"
#include "Rolky/TypeCache.hpp"

#include "Verify.hpp"
#include "HostFXRErrorCodes.hpp"
#include "RolkyManagedFunctions.hpp"

#ifdef ROLKY_WINDOWS
	#include <ShlObj_core.h>
#else
	#include <dlfcn.h>
#endif

namespace Rolky {

	struct CoreCLRFunctions
	{
		hostfxr_set_error_writer_fn SetHostFXRErrorWriter = nullptr;
		hostfxr_set_runtime_property_value_fn SetRuntimePropertyValue = nullptr;
		hostfxr_initialize_for_runtime_config_fn InitHostFXRForRuntimeConfig = nullptr;
		hostfxr_get_runtime_delegate_fn GetRuntimeDelegate = nullptr;
		hostfxr_close_fn CloseHostFXR = nullptr;
		load_assembly_and_get_function_pointer_fn GetManagedFunctionPtr = nullptr;
	};
	static CoreCLRFunctions s_CoreCLRFunctions;

	static MessageCallbackFn MessageCallback = nullptr;
	static MessageLevel MessageFilter;
	static ExceptionCallbackFn ExceptionCallback = nullptr;

	static void DefaultMessageCallback(std::string_view InMessage, MessageLevel InLevel)
	{
		const char* level = "";

		switch (InLevel)
		{
		default: break;
		case MessageLevel::Trace:
			level = "Trace";
			break;
		case MessageLevel::Info:
			level = "Info";
			break;
		case MessageLevel::Warning:
			level = "Warn";
			break;
		case MessageLevel::Error:
			level = "Error";
			break;
		}

		std::cout << "[Rolky](" << level << "): " << InMessage << std::endl;
	}

	RolkyInitStatus HostInstance::Initialize(HostSettings InSettings)
	{
		ROLKY_VERIFY(!m_Initialized);

		if (!LoadHostFXR())
		{
			return RolkyInitStatus::DotNetNotFound;
		}

		// Setup settings
		m_Settings = std::move(InSettings);

		if (!m_Settings.MessageCallback)
			m_Settings.MessageCallback = DefaultMessageCallback;
		MessageCallback = m_Settings.MessageCallback;
		MessageFilter = m_Settings.MessageFilter;

		s_CoreCLRFunctions.SetHostFXRErrorWriter([](const UCChar* InMessage)
		{
			auto message = StringHelper::ConvertWideToUtf8(InMessage);
			MessageCallback(message, MessageLevel::Error);
		});

		m_RolkyManagedAssemblyPath = std::filesystem::path(m_Settings.RolkyDirectory) / "Rolky.Managed.dll";

		if (!std::filesystem::exists(m_RolkyManagedAssemblyPath))
		{
			MessageCallback("Failed to find Rolky.Managed.dll", MessageLevel::Error);
			return RolkyInitStatus::RolkyManagedNotFound;
		}

		if (!InitializeRolkyManaged())
		{
			return RolkyInitStatus::RolkyManagedInitError;
		}

		return RolkyInitStatus::Success;
	}

	void HostInstance::Shutdown()
	{
		s_CoreCLRFunctions.CloseHostFXR(m_HostFXRContext);
	}
	
	AssemblyLoadContext HostInstance::CreateAssemblyLoadContext(std::string_view InName)
	{
		ScopedString name = String::New(InName);
		ScopedString dllPath = String::New("");
		AssemblyLoadContext alc;
		alc.m_ContextId = s_ManagedFunctions.CreateAssemblyLoadContextFptr(name, dllPath);
		alc.m_Host = this;
		return alc;
	}

	AssemblyLoadContext HostInstance::CreateAssemblyLoadContext(std::string_view InName, std::string_view InDllPath)
	{
		ScopedString name = String::New(InName);
		ScopedString dllPath = String::New(InDllPath);
		AssemblyLoadContext alc;
		alc.m_ContextId = s_ManagedFunctions.CreateAssemblyLoadContextFptr(name, dllPath);
		alc.m_Host = this;
		return alc;
	}

	void HostInstance::UnloadAssemblyLoadContext(AssemblyLoadContext& InLoadContext)
	{
		s_ManagedFunctions.UnloadAssemblyLoadContextFptr(InLoadContext.m_ContextId);
		InLoadContext.m_ContextId = -1;
		InLoadContext.m_LoadedAssemblies.Clear();
	}

#ifdef ROLKY_WINDOWS
	template <typename TFunc>
	TFunc LoadFunctionPtr(void* InLibraryHandle, const char* InFunctionName)
	{
		auto result = (TFunc)GetProcAddress((HMODULE)InLibraryHandle, InFunctionName);
		ROLKY_VERIFY(result);
		return result;
	}
#else
	template <typename TFunc>
	TFunc LoadFunctionPtr(void* InLibraryHandle, const char* InFunctionName)
	{
		auto result = (TFunc)dlsym(InLibraryHandle, InFunctionName);
		ROLKY_VERIFY(result);
		return result;
	}
#endif

	static std::filesystem::path GetHostFXRPath()
	{
#ifdef ROLKY_WINDOWS
		std::filesystem::path basePath = "";
		
		// Find the Program Files folder
		TCHAR pf[MAX_PATH];
		SHGetSpecialFolderPath(
		nullptr,
		pf,
		CSIDL_PROGRAM_FILES,
		FALSE);

		basePath = pf;
		basePath /= "dotnet/host/fxr/";

		auto searchPaths = std::array
		{
			basePath
		};
#elif defined(ROLKY_APPLE)
		auto searchPaths = std::array
		{
			std::filesystem::path("/usr/local/share/dotnet/host/fxr/"),
			std::filesystem::path("/usr/share/dotnet/host/fxr/")
		};
#else
		auto searchPaths = std::array
		{
			std::filesystem::path("/usr/local/lib/dotnet/host/fxr/"),
			std::filesystem::path("/usr/local/lib64/dotnet/host/fxr/"),
			std::filesystem::path("/usr/local/share/dotnet/host/fxr/"),

			std::filesystem::path("/usr/lib/dotnet/host/fxr/"),
			std::filesystem::path("/usr/lib64/dotnet/host/fxr/"),
			std::filesystem::path("/usr/share/dotnet/host/fxr/")
		};
#endif

		for (const auto& path : searchPaths)
		{
			if (!std::filesystem::exists(path))
				continue;

			for (const auto& dir : std::filesystem::recursive_directory_iterator(path))
			{
				if (!dir.is_directory())
					continue;

				auto dirPath = dir.path().filename();
				char version = dirPath.string()[0];

				if (version != '9')
					continue;

				auto res = dir / std::filesystem::path(ROLKY_HOSTFXR_NAME);
				ROLKY_VERIFY(std::filesystem::exists(res));
				return res;
			}
		}

		return "";
	}

	bool HostInstance::LoadHostFXR() const
	{
		// Retrieve the file path to the CoreCLR library
		auto hostfxrPath = GetHostFXRPath();

		if (hostfxrPath.empty())
		{
			return false;
		}

		// Load the CoreCLR library
		void* libraryHandle = nullptr;

#ifdef ROLKY_WINDOWS
	#ifdef ROLKY_WIDE_CHARS
		libraryHandle = LoadLibraryW(hostfxrPath.c_str());
	#else
		libraryHandle = LoadLibraryA(hostfxrPath.string().c_str());
	#endif
#else
		libraryHandle = dlopen(hostfxrPath.string().data(), RTLD_NOW | RTLD_GLOBAL);
#endif

		if (libraryHandle == nullptr)
		{
			return false;
		}

		// Load CoreCLR functions
		s_CoreCLRFunctions.SetHostFXRErrorWriter = LoadFunctionPtr<hostfxr_set_error_writer_fn>(libraryHandle, "hostfxr_set_error_writer");
		s_CoreCLRFunctions.SetRuntimePropertyValue = LoadFunctionPtr<hostfxr_set_runtime_property_value_fn>(libraryHandle, "hostfxr_set_runtime_property_value");
		s_CoreCLRFunctions.InitHostFXRForRuntimeConfig = LoadFunctionPtr<hostfxr_initialize_for_runtime_config_fn>(libraryHandle, "hostfxr_initialize_for_runtime_config");
		s_CoreCLRFunctions.GetRuntimeDelegate = LoadFunctionPtr<hostfxr_get_runtime_delegate_fn>(libraryHandle, "hostfxr_get_runtime_delegate");
		s_CoreCLRFunctions.CloseHostFXR = LoadFunctionPtr<hostfxr_close_fn>(libraryHandle, "hostfxr_close");

		return true;
	}
	
	bool HostInstance::InitializeRolkyManaged()
	{
		// Fetch load_assembly_and_get_function_pointer_fn from CoreCLR
		{
			auto runtimeConfigPath = std::filesystem::path(m_Settings.RolkyDirectory) / "Rolky.Managed.runtimeconfig.json";

			if (!std::filesystem::exists(runtimeConfigPath))
			{
				MessageCallback("Failed to find Rolky.Managed.runtimeconfig.json", MessageLevel::Error);
				return false;
			}

			int status = s_CoreCLRFunctions.InitHostFXRForRuntimeConfig(runtimeConfigPath.c_str(), nullptr, &m_HostFXRContext);
			ROLKY_VERIFY(status == StatusCode::Success || status == StatusCode::Success_HostAlreadyInitialized || status == StatusCode::Success_DifferentRuntimeProperties);
			ROLKY_VERIFY(m_HostFXRContext != nullptr);

			std::filesystem::path coralDirectoryPath = m_Settings.RolkyDirectory;
			s_CoreCLRFunctions.SetRuntimePropertyValue(m_HostFXRContext, ROLKY_STR("APP_CONTEXT_BASE_DIRECTORY"), coralDirectoryPath.c_str());

			status = s_CoreCLRFunctions.GetRuntimeDelegate(m_HostFXRContext, hdt_load_assembly_and_get_function_pointer, (void**) &s_CoreCLRFunctions.GetManagedFunctionPtr);
			ROLKY_VERIFY(status == StatusCode::Success);
		}

		using InitializeFn = void(*)(void(*)(String, MessageLevel), void(*)(String));
		InitializeFn coralManagedEntryPoint = nullptr;
		coralManagedEntryPoint = LoadRolkyManagedFunctionPtr<InitializeFn>(ROLKY_STR("Rolky.Managed.ManagedHost, Rolky.Managed"), ROLKY_STR("Initialize"));

		LoadRolkyFunctions();

		coralManagedEntryPoint([](String InMessage, MessageLevel InLevel)
		{
			if (MessageFilter & InLevel)
			{
				std::string message = InMessage;
				MessageCallback(message, InLevel);
			}
		},
		[](String InMessage)
		{
			std::string message = InMessage;
			if (!ExceptionCallback)
			{
				MessageCallback(message, MessageLevel::Error);
				return;
			}
			
			ExceptionCallback(message);
		});

		ExceptionCallback = m_Settings.ExceptionCallback;

		return true;
	}

	void HostInstance::LoadRolkyFunctions()
	{
		s_ManagedFunctions.CreateAssemblyLoadContextFptr = LoadRolkyManagedFunctionPtr<CreateAssemblyLoadContextFn>(ROLKY_STR("Rolky.Managed.AssemblyLoader, Rolky.Managed"), ROLKY_STR("CreateAssemblyLoadContext"));
		s_ManagedFunctions.UnloadAssemblyLoadContextFptr = LoadRolkyManagedFunctionPtr<UnloadAssemblyLoadContextFn>(ROLKY_STR("Rolky.Managed.AssemblyLoader, Rolky.Managed"), ROLKY_STR("UnloadAssemblyLoadContext"));
		s_ManagedFunctions.LoadAssemblyFptr = LoadRolkyManagedFunctionPtr<LoadAssemblyFn>(ROLKY_STR("Rolky.Managed.AssemblyLoader, Rolky.Managed"), ROLKY_STR("LoadAssembly"));
		s_ManagedFunctions.LoadAssemblyFromMemoryFptr = LoadRolkyManagedFunctionPtr<LoadAssemblyFromMemoryFn>(ROLKY_STR("Rolky.Managed.AssemblyLoader, Rolky.Managed"), ROLKY_STR("LoadAssemblyFromMemory"));
		s_ManagedFunctions.UnloadAssemblyLoadContextFptr = LoadRolkyManagedFunctionPtr<UnloadAssemblyLoadContextFn>(ROLKY_STR("Rolky.Managed.AssemblyLoader, Rolky.Managed"), ROLKY_STR("UnloadAssemblyLoadContext"));
		s_ManagedFunctions.GetLastLoadStatusFptr = LoadRolkyManagedFunctionPtr<GetLastLoadStatusFn>(ROLKY_STR("Rolky.Managed.AssemblyLoader, Rolky.Managed"), ROLKY_STR("GetLastLoadStatus"));
		s_ManagedFunctions.GetAssemblyNameFptr = LoadRolkyManagedFunctionPtr<GetAssemblyNameFn>(ROLKY_STR("Rolky.Managed.AssemblyLoader, Rolky.Managed"), ROLKY_STR("GetAssemblyName"));

		s_ManagedFunctions.RunMSBuildFptr = LoadRolkyManagedFunctionPtr<RunMSBuildFn>(ROLKY_STR("Rolky.Managed.MSBuildRunner, Rolky.Managed"), ROLKY_STR("Run"));

		s_ManagedFunctions.GetAssemblyTypesFptr = LoadRolkyManagedFunctionPtr<GetAssemblyTypesFn>(ROLKY_STR("Rolky.Managed.TypeInterface, Rolky.Managed"), ROLKY_STR("GetAssemblyTypes"));
		s_ManagedFunctions.GetFullTypeNameFptr = LoadRolkyManagedFunctionPtr<GetFullTypeNameFn>(ROLKY_STR("Rolky.Managed.TypeInterface, Rolky.Managed"), ROLKY_STR("GetFullTypeName"));
		s_ManagedFunctions.GetAssemblyQualifiedNameFptr = LoadRolkyManagedFunctionPtr<GetAssemblyQualifiedNameFn>(ROLKY_STR("Rolky.Managed.TypeInterface, Rolky.Managed"), ROLKY_STR("GetAssemblyQualifiedName"));
		s_ManagedFunctions.GetBaseTypeFptr = LoadRolkyManagedFunctionPtr<GetBaseTypeFn>(ROLKY_STR("Rolky.Managed.TypeInterface, Rolky.Managed"), ROLKY_STR("GetBaseType"));
		s_ManagedFunctions.GetInterfaceTypeCountFptr = LoadRolkyManagedFunctionPtr<GetInterfaceTypeCountFn>(ROLKY_STR("Rolky.Managed.TypeInterface, Rolky.Managed"), ROLKY_STR("GetInterfaceTypeCount"));
		s_ManagedFunctions.GetInterfaceTypesFptr = LoadRolkyManagedFunctionPtr<GetInterfaceTypesFn>(ROLKY_STR("Rolky.Managed.TypeInterface, Rolky.Managed"), ROLKY_STR("GetInterfaceTypes"));
		s_ManagedFunctions.GetTypeSizeFptr = LoadRolkyManagedFunctionPtr<GetTypeSizeFn>(ROLKY_STR("Rolky.Managed.TypeInterface, Rolky.Managed"), ROLKY_STR("GetTypeSize"));
		s_ManagedFunctions.IsTypeSubclassOfFptr = LoadRolkyManagedFunctionPtr<IsTypeSubclassOfFn>(ROLKY_STR("Rolky.Managed.TypeInterface, Rolky.Managed"), ROLKY_STR("IsTypeSubclassOf"));
		s_ManagedFunctions.IsTypeAssignableToFptr = LoadRolkyManagedFunctionPtr<IsTypeAssignableToFn>(ROLKY_STR("Rolky.Managed.TypeInterface, Rolky.Managed"), ROLKY_STR("IsTypeAssignableTo"));
		s_ManagedFunctions.IsTypeAssignableFromFptr = LoadRolkyManagedFunctionPtr<IsTypeAssignableFromFn>(ROLKY_STR("Rolky.Managed.TypeInterface, Rolky.Managed"), ROLKY_STR("IsTypeAssignableFrom"));
		s_ManagedFunctions.IsTypeSZArrayFptr = LoadRolkyManagedFunctionPtr<IsTypeSZArrayFn>(ROLKY_STR("Rolky.Managed.TypeInterface, Rolky.Managed"), ROLKY_STR("IsTypeSZArray"));
		s_ManagedFunctions.GetElementTypeFptr = LoadRolkyManagedFunctionPtr<GetElementTypeFn>(ROLKY_STR("Rolky.Managed.TypeInterface, Rolky.Managed"), ROLKY_STR("GetElementType"));
		s_ManagedFunctions.GetTypeMethodsFptr = LoadRolkyManagedFunctionPtr<GetTypeMethodsFn>(ROLKY_STR("Rolky.Managed.TypeInterface, Rolky.Managed"), ROLKY_STR("GetTypeMethods"));
		s_ManagedFunctions.GetTypeFieldsFptr = LoadRolkyManagedFunctionPtr<GetTypeFieldsFn>(ROLKY_STR("Rolky.Managed.TypeInterface, Rolky.Managed"), ROLKY_STR("GetTypeFields"));
		s_ManagedFunctions.GetTypePropertiesFptr = LoadRolkyManagedFunctionPtr<GetTypePropertiesFn>(ROLKY_STR("Rolky.Managed.TypeInterface, Rolky.Managed"), ROLKY_STR("GetTypeProperties"));
		s_ManagedFunctions.HasTypeAttributeFptr = LoadRolkyManagedFunctionPtr<HasTypeAttributeFn>(ROLKY_STR("Rolky.Managed.TypeInterface, Rolky.Managed"), ROLKY_STR("HasTypeAttribute"));
		s_ManagedFunctions.GetTypeAttributesFptr = LoadRolkyManagedFunctionPtr<GetTypeAttributesFn>(ROLKY_STR("Rolky.Managed.TypeInterface, Rolky.Managed"), ROLKY_STR("GetTypeAttributes"));
		s_ManagedFunctions.GetTypeManagedTypeFptr = LoadRolkyManagedFunctionPtr<GetTypeManagedTypeFn>(ROLKY_STR("Rolky.Managed.TypeInterface, Rolky.Managed"), ROLKY_STR("GetTypeManagedType"));
		s_ManagedFunctions.InvokeStaticMethodFptr = LoadRolkyManagedFunctionPtr<InvokeStaticMethodFn>(ROLKY_STR("Rolky.Managed.ManagedObject, Rolky.Managed"), ROLKY_STR("InvokeStaticMethod"));
		s_ManagedFunctions.InvokeStaticMethodRetFptr = LoadRolkyManagedFunctionPtr<InvokeStaticMethodRetFn>(ROLKY_STR("Rolky.Managed.ManagedObject, Rolky.Managed"), ROLKY_STR("InvokeStaticMethodRet"));

		s_ManagedFunctions.GetMethodInfoNameFptr = LoadRolkyManagedFunctionPtr<GetMethodInfoNameFn>(ROLKY_STR("Rolky.Managed.TypeInterface, Rolky.Managed"), ROLKY_STR("GetMethodInfoName"));
		s_ManagedFunctions.GetMethodInfoReturnTypeFptr = LoadRolkyManagedFunctionPtr<GetMethodInfoReturnTypeFn>(ROLKY_STR("Rolky.Managed.TypeInterface, Rolky.Managed"), ROLKY_STR("GetMethodInfoReturnType"));
		s_ManagedFunctions.GetMethodInfoParameterTypesFptr = LoadRolkyManagedFunctionPtr<GetMethodInfoParameterTypesFn>(ROLKY_STR("Rolky.Managed.TypeInterface, Rolky.Managed"), ROLKY_STR("GetMethodInfoParameterTypes"));
		s_ManagedFunctions.GetMethodInfoAccessibilityFptr = LoadRolkyManagedFunctionPtr<GetMethodInfoAccessibilityFn>(ROLKY_STR("Rolky.Managed.TypeInterface, Rolky.Managed"), ROLKY_STR("GetMethodInfoAccessibility"));
		s_ManagedFunctions.GetMethodInfoAttributesFptr = LoadRolkyManagedFunctionPtr<GetMethodInfoAttributesFn>(ROLKY_STR("Rolky.Managed.TypeInterface, Rolky.Managed"), ROLKY_STR("GetMethodInfoAttributes"));

		s_ManagedFunctions.GetFieldInfoNameFptr = LoadRolkyManagedFunctionPtr<GetFieldInfoNameFn>(ROLKY_STR("Rolky.Managed.TypeInterface, Rolky.Managed"), ROLKY_STR("GetFieldInfoName"));
		s_ManagedFunctions.GetFieldInfoTypeFptr = LoadRolkyManagedFunctionPtr<GetFieldInfoTypeFn>(ROLKY_STR("Rolky.Managed.TypeInterface, Rolky.Managed"), ROLKY_STR("GetFieldInfoType"));
		s_ManagedFunctions.GetFieldInfoAccessibilityFptr = LoadRolkyManagedFunctionPtr<GetFieldInfoAccessibilityFn>(ROLKY_STR("Rolky.Managed.TypeInterface, Rolky.Managed"), ROLKY_STR("GetFieldInfoAccessibility"));
		s_ManagedFunctions.GetFieldInfoAttributesFptr = LoadRolkyManagedFunctionPtr<GetFieldInfoAttributesFn>(ROLKY_STR("Rolky.Managed.TypeInterface, Rolky.Managed"), ROLKY_STR("GetFieldInfoAttributes"));

		s_ManagedFunctions.GetPropertyInfoNameFptr = LoadRolkyManagedFunctionPtr<GetPropertyInfoNameFn>(ROLKY_STR("Rolky.Managed.TypeInterface, Rolky.Managed"), ROLKY_STR("GetPropertyInfoName"));
		s_ManagedFunctions.GetPropertyInfoTypeFptr = LoadRolkyManagedFunctionPtr<GetPropertyInfoTypeFn>(ROLKY_STR("Rolky.Managed.TypeInterface, Rolky.Managed"), ROLKY_STR("GetPropertyInfoType"));
		s_ManagedFunctions.GetPropertyInfoAttributesFptr = LoadRolkyManagedFunctionPtr<GetPropertyInfoAttributesFn>(ROLKY_STR("Rolky.Managed.TypeInterface, Rolky.Managed"), ROLKY_STR("GetPropertyInfoAttributes"));

		s_ManagedFunctions.GetAttributeFieldValueFptr = LoadRolkyManagedFunctionPtr<GetAttributeFieldValueFn>(ROLKY_STR("Rolky.Managed.TypeInterface, Rolky.Managed"), ROLKY_STR("GetAttributeFieldValue"));
		s_ManagedFunctions.GetAttributeTypeFptr = LoadRolkyManagedFunctionPtr<GetAttributeTypeFn>(ROLKY_STR("Rolky.Managed.TypeInterface, Rolky.Managed"), ROLKY_STR("GetAttributeType"));

		s_ManagedFunctions.SetInternalCallsFptr = LoadRolkyManagedFunctionPtr<SetInternalCallsFn>(ROLKY_STR("Rolky.Managed.Interop.InternalCallsManager, Rolky.Managed"), ROLKY_STR("SetInternalCalls"));
		s_ManagedFunctions.CreateObjectFptr = LoadRolkyManagedFunctionPtr<CreateObjectFn>(ROLKY_STR("Rolky.Managed.ManagedObject, Rolky.Managed"), ROLKY_STR("CreateObject"));
		s_ManagedFunctions.CopyObjectFptr = LoadRolkyManagedFunctionPtr<CopyObjectFn>(ROLKY_STR("Rolky.Managed.ManagedObject, Rolky.Managed"), ROLKY_STR("CopyObject"));
		s_ManagedFunctions.InvokeMethodFptr = LoadRolkyManagedFunctionPtr<InvokeMethodFn>(ROLKY_STR("Rolky.Managed.ManagedObject, Rolky.Managed"), ROLKY_STR("InvokeMethod"));
		s_ManagedFunctions.InvokeMethodRetFptr = LoadRolkyManagedFunctionPtr<InvokeMethodRetFn>(ROLKY_STR("Rolky.Managed.ManagedObject, Rolky.Managed"), ROLKY_STR("InvokeMethodRet"));
		s_ManagedFunctions.SetFieldValueFptr = LoadRolkyManagedFunctionPtr<SetFieldValueFn>(ROLKY_STR("Rolky.Managed.ManagedObject, Rolky.Managed"), ROLKY_STR("SetFieldValue"));
		s_ManagedFunctions.GetFieldValueFptr = LoadRolkyManagedFunctionPtr<GetFieldValueFn>(ROLKY_STR("Rolky.Managed.ManagedObject, Rolky.Managed"), ROLKY_STR("GetFieldValue"));
		s_ManagedFunctions.SetPropertyValueFptr = LoadRolkyManagedFunctionPtr<SetFieldValueFn>(ROLKY_STR("Rolky.Managed.ManagedObject, Rolky.Managed"), ROLKY_STR("SetPropertyValue"));
		s_ManagedFunctions.GetPropertyValueFptr = LoadRolkyManagedFunctionPtr<GetFieldValueFn>(ROLKY_STR("Rolky.Managed.ManagedObject, Rolky.Managed"), ROLKY_STR("GetPropertyValue"));
		s_ManagedFunctions.DestroyObjectFptr = LoadRolkyManagedFunctionPtr<DestroyObjectFn>(ROLKY_STR("Rolky.Managed.ManagedObject, Rolky.Managed"), ROLKY_STR("DestroyObject"));
		s_ManagedFunctions.GetObjectTypeIdFptr = LoadRolkyManagedFunctionPtr<GetObjectTypeIdFn>(ROLKY_STR("Rolky.Managed.ManagedObject, Rolky.Managed"), ROLKY_STR("GetObjectTypeId"));

		s_ManagedFunctions.CollectGarbageFptr = LoadRolkyManagedFunctionPtr<CollectGarbageFn>(ROLKY_STR("Rolky.Managed.GarbageCollector, Rolky.Managed"), ROLKY_STR("CollectGarbage"));
		s_ManagedFunctions.WaitForPendingFinalizersFptr = LoadRolkyManagedFunctionPtr<WaitForPendingFinalizersFn>(ROLKY_STR("Rolky.Managed.GarbageCollector, Rolky.Managed"), ROLKY_STR("WaitForPendingFinalizers"));
	}

	void* HostInstance::LoadRolkyManagedFunctionPtr(const std::filesystem::path& InAssemblyPath, const UCChar* InTypeName, const UCChar* InMethodName, const UCChar* InDelegateType) const
	{
		void* funcPtr = nullptr;

		int status = s_CoreCLRFunctions.GetManagedFunctionPtr(InAssemblyPath.c_str(), InTypeName, InMethodName, InDelegateType, nullptr, &funcPtr);
		if(status != StatusCode::Success || !funcPtr) {
			std::cerr << "Failed to retrieve managed function pointer `" << InTypeName << "`::`" << InMethodName << "` from `" << InAssemblyPath << "`" << std::endl;
			ROLKY_VERIFY(false);
		}

		return funcPtr;
	}
}
