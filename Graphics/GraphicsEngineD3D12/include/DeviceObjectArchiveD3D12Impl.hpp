/*
 *  Copyright 2019-2021 Diligent Graphics LLC
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

#pragma once

/// \file
/// Declaration of Diligent::DeviceObjectArchiveD3D12Impl class

#include "Dearchiver.h"

#include "EngineD3D12ImplTraits.hpp"
#include "DeviceObjectArchiveBase.hpp"

namespace Diligent
{

/// Device object archive object implementation in Direct3D12 backend.
class DeviceObjectArchiveD3D12Impl final : public DeviceObjectArchiveBase
{
public:
    DeviceObjectArchiveD3D12Impl(IReferenceCounters* pRefCounters, IArchive* pSource);
    ~DeviceObjectArchiveD3D12Impl();

    RefCntAutoPtr<IPipelineResourceSignature> UnpackResourceSignature(const ResourceSignatureUnpackInfo& DeArchiveInfo) override final;
};

template <SerializerMode Mode>
struct PSOSerializerD3D12
{
    template <typename T>
    using TQual = typename Serializer<Mode>::template TQual<T>;

    static void SerializePRSDesc(Serializer<Mode>&                                    Ser,
                                 TQual<PipelineResourceSignatureSerializedDataD3D12>& Serialized,
                                 DynamicLinearAllocator*                              Allocator);
};

DECL_TRIVIALLY_SERIALIZABLE(PipelineResourceAttribsD3D12);
DECL_TRIVIALLY_SERIALIZABLE(PipelineResourceImmutableSamplerAttribsD3D12);

} // namespace Diligent
