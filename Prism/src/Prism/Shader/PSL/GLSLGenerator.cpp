#include "prpch.h"
#include "GLSLGenerator.h"
#include "IncludeExpander.h"

#include <algorithm>
#include <fstream>
#include <filesystem>

namespace Prism::PSL::GLSLGen
{
    using namespace Prism::PSL;
    static Setting s_Setting = {};
    static const char* BINDINGS_FILE = "Bindings.glsl";
    static const char* FRAME_UBO_MARKER = "PrismFrame.glsl";
    static const char* OBJECT_UBO_MARKER = "PrismObject.glsl";
    static const char* SHADOW_UBO_MARKER = "PrismShadow.glsl";
    static void replaceInsert(std::string& source, const std::string& replacement, uint32_t id)
    {
        std::string marker = "[Prism::Insert:" + std::to_string(id) + "]";
        size_t pos = source.find(marker);
        if (pos == std::string::npos) return;
        source.erase(pos, marker.size());
        source.insert(pos, replacement);
    }


    static void GenerateAttribute(std::string& source, const AST::VertexAttribute& attr)
    {
        std::string line = "layout(location = " + std::to_string(Prism::PSL::SemanticToLocation(attr.Semantic)) + ") in " + attr.Type + " " + attr.Name + ";\n";
        line += "#line " + std::to_string(attr.Loc.Line) + " \"" + std::string(attr.Loc.FilePath) + "\"\n";
        replaceInsert(source, line, attr.InsertID);
    }
    static void GenerateInclude(std::string& source, const AST::IncludeDef& include)
    {
        std::string line = "#line 1 \"" + include.Path + "\"\n";
        line += ExpandIncludesRecursive(include.Path, s_Setting.IncludeRoot);
        line += "#line " + std::to_string(include.Loc.Line) + " \"" + std::string(include.Loc.FilePath) + "\"\n";
        replaceInsert(source, line, include.InsertID);
    }
    static void GenerateEngineHeader(std::string& source, const AST::GLSLCode& glsl)
    {
        std::string line = "#line 1 \"" + std::string(BINDINGS_FILE) + "\"\n";
        line += ExpandIncludesRecursive(BINDINGS_FILE, s_Setting.EngineRoot);
        line += "#line 1 \"" + std::string(FRAME_UBO_MARKER) + "\"\n";
        line += ExpandIncludesRecursive(FRAME_UBO_MARKER, s_Setting.EngineRoot);
        line += "#line 1 \"" + std::string(OBJECT_UBO_MARKER) + "\"\n";
        line += ExpandIncludesRecursive(OBJECT_UBO_MARKER, s_Setting.EngineRoot);
        line += "#line 1 \"" + std::string(SHADOW_UBO_MARKER) + "\"\n";
        line += ExpandIncludesRecursive(SHADOW_UBO_MARKER, s_Setting.EngineRoot);
        line += "#line " + std::to_string(glsl.Loc.Line) + " \"" + std::string(glsl.Loc.FilePath) + "\"\n";
        source = line + source;
    }
    static void GeneratePragma(std::string& source, const AST::PragmaDef& pragma)
    {
        std::string line = "#line " + std::to_string(pragma.Loc.Line) + " \"" + std::string(pragma.Loc.FilePath) + "\"\n";
        replaceInsert(source, line, pragma.InsertID);
    }
    static void GenerateVarying(std::string& source, const AST::VaryingBlock& varying, const bool isVertex)
    {
        std::string line{}, prefix = isVertex ? "out " : "in ";
        if (varying.IsStruct)
        {
            line += "struct " + varying.StructName + "\n{\n";
            for (const auto& member : varying.Members)
                line += "    " + member.Type + " " + member.Name + ";\n";
            line += "};\n";
            line += prefix + varying.StructName + " " + varying.InstanceName + ";\n";
        }
        else
        {
            line += prefix + varying.Type + " " + varying.InstanceName + ";\n";
        }
        line += "#line " + std::to_string(varying.Loc.Line) + " \"" + std::string(varying.Loc.FilePath) + "\"\n";
        replaceInsert(source, line, varying.InsertID);
    }
    static void GenerateVertexCode(std::string& source, const AST::GLSLCode& glsl)
    {
        for (const auto& attr : glsl.Attributes)
            GenerateAttribute(source, attr);
        for (const auto& varying : glsl.Varyings)
            GenerateVarying(source, varying, true);
        std::string line = "#line " + std::to_string(glsl.Vertex.Loc.Line - 1) + " \"" + std::string(glsl.Vertex.Loc.FilePath) + "\"\n";
        line += "void main()\n";
        line += glsl.Vertex.Source + "\n";
        source += line;
    }
    static void GenerateFragmentCode(std::string& source, const AST::GLSLCode& glsl)
    {
        for (const auto& varying : glsl.Varyings)
            GenerateVarying(source, varying, false);
        std::string line = "#line " + std::to_string(glsl.Fragment.Loc.Line - 1) + " \"" + std::string(glsl.Fragment.Loc.FilePath) + "\"\n";
        line += "void main()\n";
        line += glsl.Fragment.Source + "\n";
        source += line;
    }

    Output PRISM_API Generate(const AST::GLSLCode& glsl, const std::vector<AST::PropertyDef>& properties, const std::string& filePath)
    {
        std::string sharedSource = glsl.SharedSource;
        Output output{};
        GenerateEngineHeader(sharedSource, glsl);
        for (const auto& pragma : glsl.Pragmas)
            GeneratePragma(sharedSource, pragma);
        for (const auto& include : glsl.Includes)
            GenerateInclude(sharedSource, include);
        output.Vertex = sharedSource;
        output.Fragment = sharedSource;
        GenerateVertexCode(output.Vertex, glsl);
        GenerateFragmentCode(output.Fragment, glsl);
        return output;
    }


    void PRISM_API SetSetting(const Setting& setting)
    {
        s_Setting = setting;
    }
} // namespace Prism::PSL
