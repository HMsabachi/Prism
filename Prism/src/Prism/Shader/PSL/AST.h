#pragma once

#include "Prism/Shader/ShaderCommon.h"
#include "Prism/Shader/Property/PropertyType.h"
#include "Prism/Shader/Pipeline/PipelineState.h"
#include "Prism/Utilities/Variant.h"
#include "Prism/Shader/Property/VertexType.h"

#include <string>
#include <vector>
#include <unordered_map>
#include <optional>

namespace Prism::PSL::AST
{

struct VertexAttribute
{
    std::string Type;
    std::string Name;
    Prism::PSL::VertexSemantic Semantic;
    uint32_t InsertID = 0;
    SourceLocation Loc;
};

struct VaryingMember
{
    std::string Type;
    std::string Name;
};

struct VaryingBlock
{
    bool IsStruct = false;
    std::string StructName;
    std::string InstanceName;
    std::vector<VaryingMember> Members;
    std::string Type;
    uint32_t InsertID = 0;
    SourceLocation Loc;
};

struct PragmaDef
{
    bool IsMultiCompile = false;
    bool IsShaderFeature = false;
    std::vector<std::string> Keywords;
    uint32_t InsertID = 0;
    SourceLocation Loc;
};

struct EntryPointSource
{
    std::string Source;
    SourceLocation Loc;
};

struct IncludeDef
{
    std::string Path;
    uint32_t InsertID = 0;
    SourceLocation Loc;
};

struct GLSLCode
{
    std::string SharedSource;
    EntryPointSource Vertex;
    EntryPointSource Fragment;
    SourceLocation Loc;

    std::vector<VertexAttribute> Attributes;
    std::vector<VaryingBlock> Varyings;
    std::vector<PragmaDef> Pragmas;
    std::vector<IncludeDef> Includes;
};

struct PropertyDef
{
    std::string Name;
    std::string DisplayName;
    PropertyType Type;
    Variant DefaultValue;
    std::vector<std::string> EnumOptions;
    SourceLocation Loc;
};

struct PassDef
{
    std::string Name;
    std::unordered_map<std::string, std::string> Tags;
    std::optional<PipelineState> RenderState;
    GLSLCode Glsl;
};

struct ShaderDocument
{
    std::string ShaderName;
    int LOD = 200;
    std::vector<PropertyDef> Properties;
    std::optional<PipelineState> RenderState;
    std::vector<PassDef> Passes;
};

} // namespace Prism::PSL::AST
