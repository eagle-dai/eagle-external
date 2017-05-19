﻿//---------------------------------------------------------------------
// <copyright file="edm_model.cpp" company="Microsoft">
//      Copyright (C) Microsoft Corporation. All rights reserved. See License.txt in the project root for license information.
// </copyright>
//---------------------------------------------------------------------

#include "odata/edm/odata_edm.h"

namespace odata { namespace edm
{

std::shared_ptr<edm_entity_container> edm_schema::find_container(::odata::utility::string_t name) const
{
    if (name.empty())
    {
        for (auto ent = m_entity_containers.begin(); ent != m_entity_containers.end(); ++ent)
        {
			if (ent->second->is_default_container())
			{
				return ent->second;
			}
        }
    }
    else
    {
        for (auto ent = m_entity_containers.begin(); ent != m_entity_containers.end(); ++ent)
        {
			if (ent->second->get_name() == name)
			{
				return ent->second;
			}
        }
    }

    return nullptr;
}

std::shared_ptr<edm_entity_type> edm_model::find_entity_type(::odata::utility::string_t name) const
{
    for (auto sc = m_schemata.cbegin(); sc != m_schemata.cend(); ++sc)
    {
        auto et_ptr = (*sc)->find_entity_type(name);
        if (et_ptr)
		{
            return et_ptr;
		}
    }

    return nullptr;
}

std::shared_ptr<edm_complex_type> edm_model::find_complex_type(::odata::utility::string_t name) const
{
    for (auto sc = m_schemata.cbegin(); sc != m_schemata.cend(); ++sc)
    {
        auto cp_ptr = (*sc)->find_complex_type(name);
        if (cp_ptr)
		{
            return cp_ptr;
		}
    }

    return nullptr;
}

std::shared_ptr<edm_enum_type> edm_model::find_enum_type(::odata::utility::string_t name) const
{
    for (auto sc = m_schemata.cbegin(); sc != m_schemata.cend(); ++sc)
    {
        auto en_ptr = (*sc)->find_enum_type(name);
        if (en_ptr)
		{
            return en_ptr;
		}
    }

    return nullptr;
}

std::shared_ptr<edm_operation_type> edm_model::find_operation_type(::odata::utility::string_t name) const
{
    for (auto sc = m_schemata.cbegin(); sc != m_schemata.cend(); ++sc)
    {
        auto op_ptr = (*sc)->find_operation_type(name);
        if (op_ptr)
		{
            return op_ptr;
		}
    }

    return nullptr;
}

std::shared_ptr<edm_entity_container> edm_model::find_container(::odata::utility::string_t name) const
{
    for (auto sc = m_schemata.cbegin(); sc != m_schemata.cend(); ++sc)
    {
        auto cn_ptr = (*sc)->find_container(name);
        if (cn_ptr)
		{
            return cn_ptr;
		}
    }

    return nullptr;
}

}}