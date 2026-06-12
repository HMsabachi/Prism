#include "prpch.h"
#include "PipelineState.h"

namespace Prism::PSL
{

PipelineState PipelineState::Default()
{
    return PipelineState{};
}

void PipelineState::Merge(const PipelineState& passState)
{
    BlendEnabled = passState.BlendEnabled;
    SrcFactor = passState.SrcFactor;
    DstFactor = passState.DstFactor;
    SrcAlpha = passState.SrcAlpha;
    DstAlpha = passState.DstAlpha;
    DepthTest = passState.DepthTest;
    DepthWrite = passState.DepthWrite;
    DepthCompare = passState.DepthCompare;
    Cull = passState.Cull;
    WriteMask = passState.WriteMask;
    DepthBiasFactor = passState.DepthBiasFactor;
    DepthBiasUnits = passState.DepthBiasUnits;
}

} // namespace Prism::PSL
