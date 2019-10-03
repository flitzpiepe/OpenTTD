/* $Id: build_vehicle_gui.cpp 23792 2012-01-12 19:23:00Z yexo $ */

/*
 * This file is part of OpenTTD.
 * OpenTTD is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 2.
 * OpenTTD is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details. You should have received a copy of the GNU General Public License along with OpenTTD. If not, see <http://www.gnu.org/licenses/>.
 */

/** @file tbtr_template_vehicle_sl.cpp Save and load template vehicles. */

#include "../stdafx.h"

#include "../tbtr_template_vehicle.h"

#include "saveload.h"

const SaveLoad* GetTemplateDescription() {

	static const SaveLoad _template_vehicle_desc[] = {
		SLE_VAR(TemplateVehicle, index,                     SLE_UINT16),
		SLE_REF(TemplateVehicle, next,                      REF_TEMPLATE_VEHICLE),
		SLE_REF(TemplateVehicle, prev,                      REF_TEMPLATE_VEHICLE),
		SLE_REF(TemplateVehicle, first,                     REF_TEMPLATE_VEHICLE),
		SLE_REF(TemplateVehicle, last,                      REF_TEMPLATE_VEHICLE),
		SLE_VAR(TemplateVehicle, owner,                     SLE_UINT32),

		SLE_VAR(TemplateVehicle, engine_type,               SLE_UINT16),
		SLE_VAR(TemplateVehicle, max_speed,                 SLE_UINT16),
		SLE_VAR(TemplateVehicle, power,                     SLE_UINT16),
		SLE_VAR(TemplateVehicle, weight,                    SLE_UINT16),
		SLE_VAR(TemplateVehicle, max_te,                    SLE_UINT16),

		SLE_VAR(TemplateVehicle, cargo_type,                SLE_UINT8),
		SLE_VAR(TemplateVehicle, cargo_cap,                 SLE_UINT16),
		SLE_VAR(TemplateVehicle, subtype,                   SLE_UINT8),
		SLE_VAR(TemplateVehicle, railtype,                  SLE_UINT8),

		SLE_VAR(TemplateVehicle, real_length,               SLE_UINT16),

		SLE_END()
	};

	static const SaveLoad * const _ret[] = {
		_template_vehicle_desc,
	};

	return _ret[0];
}

static void Save_TMPLS()
{
	TemplateVehicle *tv;

	FOR_ALL_TEMPLATES(tv) {
		SlSetArrayIndex(tv->index);
		SlObject(tv, GetTemplateDescription());
	}
}

static void Load_TMPLS()
{
	int index;

	while ((index = SlIterateArray()) != -1) {
		TemplateVehicle *tv = new (index) TemplateVehicle();
		SlObject(tv, GetTemplateDescription());
	}
}

static void Ptrs_TMPLS()
{
	TemplateVehicle *tv;
	FOR_ALL_TEMPLATES(tv) {
		SlObject(tv, GetTemplateDescription());
	}
}

extern const ChunkHandler _template_vehicle_chunk_handlers[] = {
	{'TBTR', Save_TMPLS, Load_TMPLS, Ptrs_TMPLS, NULL, CH_ARRAY|CH_LAST},
};
