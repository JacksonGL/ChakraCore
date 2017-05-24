//-------------------------------------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE.txt file in the project root for full license information.
//-------------------------------------------------------------------------------------------------------
#include "RuntimeDebugPch.h"

#if ENABLE_ALLOC_TRACING

namespace AllocTracing
{
    void AllocDataWriter::WriteObjectId(Js::RecyclableObject* value)
    {
        asdf;
    }

    void AllocDataWriter::WriteInt(int64 value)
    {
        asdf;
    }

    void AllocDataWriter::WriteChar(char16 c)
    {
        asdf;
    }

    void AllocDataWriter::WriteLiteralString(const char* str)
    {
        asdf;
    }

    void AllocDataWriter::WriteString(const char16* str, size_t length)
    {
        asdf;
    }

    SourceLocation::SourceLocation(Js::JavascriptFunction* function, uint32 line, uint32 column)
        : m_function(function), m_line(line), m_column(column)
    {
        ;
    }

    bool SourceLocation::SameAsOtherLocation(Js::JavascriptFunction* function, uint32 line, uint32 column) const
    {
        return (this->m_function == function) & (this->m_line == line) & (this->m_column == column);
    }

    void SourceLocation::JSONWriteLocationData(AllocDataWriter& writer) const
    {
        Js::JavascriptString* name = this->m_function->GetDisplayName();

        writer.WriteLiteralString("src: { ");
        writer.WriteLiteralString("file: '");
        writer.WriteString(name->GetSz(), name->GetLength());
        writer.WriteLiteralString("', line: ");
        writer.WriteInt(this->m_line + 1);
        writer.WriteLiteralString(", column: ");
        writer.WriteInt(this->m_column);
        writer.WriteLiteralString(" }");
    }

    AllocSiteStats::AllocSiteStats(ThreadContext* allocationContext)
        : m_threadContext(allocationContext), m_allocationCount(0), m_allocationLiveSet()
    {
        Recycler* recycler = this->m_threadContext->GetRecycler();
        this->m_allocationLiveSet.Root(RecyclerNew(recycler, AllocPinSet, recycler), recycler);
    }

    AllocSiteStats::~AllocSiteStats()
    {
        if(this->m_allocationLiveSet != nullptr)
        {
            this->m_allocationLiveSet.Unroot(this->m_threadContext->GetRecycler());
        }
    }

    void AllocSiteStats::AddAllocation(Js::RecyclableObject* obj)
    {
        this->m_allocationCount++;
        this->m_allocationLiveSet->Add(obj, true);
    }

    void AllocSiteStats::EstimateMemoryUseInfo(size_t& liveCount, size_t& liveSize) const
    {
        size_t regSize = 0;
        this->m_allocationLiveSet->Map([&](Js::RecyclableObject* key, bool, const RecyclerWeakReference<Js::RecyclableObject>*)
        {
            size_t osize = 0;
            Js::TypeId tid = key->GetTypeId();
            if(Js::StaticType::Is(tid))
            {
                osize = ALLOC_TRACING_STATIC_SIZE_DEFAULT;
                if(tid == Js::TypeIds_String)
                {
                    osize += Js::JavascriptString::FromVar(key)->GetLength() * sizeof(char16);
                }
            }
            else
            {
                osize = ALLOC_TRACING_DYNAMIC_SIZE_DEFAULT + (key->GetPropertyCount() * ALLOC_TRACING_DYNAMIC_ENTRY_SIZE);

                //TODO: add v-call for arrays etc to add to the size estimate
            }

            liveCount++;
            liveSize += osize;
        });
    }

    bool AllocSiteStats::IsInterestingSite(size_t countLimit, size_t sizeLimit) const
    {
        size_t liveCount = 0;
        size_t liveSize = 0;
        this->EstimateMemoryUseInfo(liveCount, liveSize);

        return (liveCount >= countLimit) | (liveSize >= sizeLimit);
    }

    void AllocSiteStats::JSONWriteSiteData(AllocDataWriter& writer) const
    {
        const uint32 sampleMax = 20;
        uint32 sampleControl = sampleMax;

        bool first = true;
        writer.WriteChar('[');
        this->m_allocationLiveSet->Map([&](Js::RecyclableObject* key, bool, const RecyclerWeakReference<Js::RecyclableObject>*)
        {
            if(sampleControl != 0 && (rand() % sampleMax) < sampleControl)
            {
                sampleControl--;

                if(!first)
                {
                    writer.WriteLiteralString(", ");
                }
                writer.WriteObjectId(key);
            }
        });
        writer.WriteChar(']');
    }
}

#endif
