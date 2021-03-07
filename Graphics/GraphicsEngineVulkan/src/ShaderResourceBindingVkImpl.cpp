/*
 *  Copyright 2019-2021 Diligent Graphics LLC
 *  Copyright 2015-2019 Egor Yusov
 *  
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *  
 *      http://www.apache.org/licenses/LICENSE-2.0
 *  
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *
 *  In no event and under no legal theory, whether in tort (including negligence), 
 *  contract, or otherwise, unless required by applicable law (such as deliberate 
 *  and grossly negligent acts) or agreed to in writing, shall any Contributor be
 *  liable for any damages, including any direct, indirect, special, incidental, 
 *  or consequential damages of any character arising as a result of this License or 
 *  out of the use or inability to use the software (including but not limited to damages 
 *  for loss of goodwill, work stoppage, computer failure or malfunction, or any and 
 *  all other commercial damages or losses), even if such Contributor has been advised 
 *  of the possibility of such damages.
 */

#include "pch.h"
#include "ShaderResourceBindingVkImpl.hpp"
#include "PipelineStateVkImpl.hpp"
#include "ShaderVkImpl.hpp"
#include "RenderDeviceVkImpl.hpp"
#include "FixedLinearAllocator.hpp"

namespace Diligent
{

ShaderResourceBindingVkImpl::ShaderResourceBindingVkImpl(IReferenceCounters*              pRefCounters,
                                                         PipelineResourceSignatureVkImpl* pPRS) :
    // clang-format off
    TBase
    {
        pRefCounters,
        pPRS
    },
    m_ShaderResourceCache{ResourceCacheContentType::SRB}
// clang-format on
{
    try
    {
        const auto NumShaders = GetNumShaders();

        FixedLinearAllocator MemPool{GetRawAllocator()};
        MemPool.AddSpace<ShaderVariableManagerVk>(NumShaders);
        MemPool.Reserve();
        m_pShaderVarMgrs = MemPool.ConstructArray<ShaderVariableManagerVk>(NumShaders, std::ref(*this), std::ref(m_ShaderResourceCache));

        // The memory is now owned by ShaderResourceBindingVkImpl and will be freed by Destruct().
        auto* Ptr = MemPool.ReleaseOwnership();
        VERIFY_EXPR(Ptr == m_pShaderVarMgrs);
        (void)Ptr;

        // It is important to construct all objects before initializing them because if an exception is thrown,
        // destructors will be called for all objects

        // This will only allocate memory and initialize descriptor sets in the resource cache
        // Resources will be initialized by InitializeResourceMemoryInCache()
        auto& SRBMemAllocator            = pPRS->GetSRBMemoryAllocator();
        auto& ResourceCacheDataAllocator = SRBMemAllocator.GetResourceCacheDataAllocator(0);
        pPRS->InitSRBResourceCache(m_ShaderResourceCache, ResourceCacheDataAllocator, pPRS->GetDesc().Name);

        for (Uint32 s = 0; s < NumShaders; ++s)
        {
            const auto ShaderType = pPRS->GetActiveShaderStageType(s);
            const auto ShaderInd  = GetShaderTypePipelineIndex(ShaderType, pPRS->GetPipelineType());
            const auto MgrInd     = m_ActiveShaderStageIndex[ShaderInd];
            VERIFY_EXPR(MgrInd >= 0 && MgrInd < static_cast<int>(NumShaders));

            auto& VarDataAllocator = SRBMemAllocator.GetShaderVariableDataAllocator(s);

            // Create shader variable manager in place
            // Initialize vars manager to reference mutable and dynamic variables
            // Note that the cache has space for all variable types
            const SHADER_RESOURCE_VARIABLE_TYPE VarTypes[] = {SHADER_RESOURCE_VARIABLE_TYPE_MUTABLE, SHADER_RESOURCE_VARIABLE_TYPE_DYNAMIC};
            m_pShaderVarMgrs[MgrInd].Initialize(*pPRS, VarDataAllocator, VarTypes, _countof(VarTypes), ShaderType);
        }
    }
    catch (...)
    {
        Destruct();
        throw;
    }
}

ShaderResourceBindingVkImpl::~ShaderResourceBindingVkImpl()
{
    Destruct();
}

void ShaderResourceBindingVkImpl::Destruct()
{
    if (m_pShaderVarMgrs != nullptr)
    {
        auto& SRBMemAllocator = GetSignature()->GetSRBMemoryAllocator();
        for (Uint32 s = 0; s < GetNumShaders(); ++s)
        {
            auto& VarDataAllocator = SRBMemAllocator.GetShaderVariableDataAllocator(s);
            m_pShaderVarMgrs[s].Destroy(VarDataAllocator);
            m_pShaderVarMgrs[s].~ShaderVariableManagerVk();
        }

        GetRawAllocator().Free(m_pShaderVarMgrs);
        m_pShaderVarMgrs = nullptr;
    }
}

void ShaderResourceBindingVkImpl::BindResources(Uint32 ShaderFlags, IResourceMapping* pResMapping, Uint32 Flags)
{
    BindResourcesImpl(ShaderFlags, pResMapping, Flags, m_pShaderVarMgrs);
}

Uint32 ShaderResourceBindingVkImpl::GetVariableCount(SHADER_TYPE ShaderType) const
{
    return GetVariableCountImpl(ShaderType, m_pShaderVarMgrs);
}

IShaderResourceVariable* ShaderResourceBindingVkImpl::GetVariableByName(SHADER_TYPE ShaderType, const Char* Name)
{
    return GetVariableByNameImpl(ShaderType, Name, m_pShaderVarMgrs);
}

IShaderResourceVariable* ShaderResourceBindingVkImpl::GetVariableByIndex(SHADER_TYPE ShaderType, Uint32 Index)
{
    return GetVariableByIndexImpl(ShaderType, Index, m_pShaderVarMgrs);
}

} // namespace Diligent
