/*
 * This file is part of OpenTTD.
 * OpenTTD is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 2.
 * OpenTTD is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details. You should have received a copy of the GNU General Public License along with OpenTTD. If not, see <http://www.gnu.org/licenses/>.
 */

/**
 * @file tbtr_template_vehicle.h
 * The class definitions for template trains, template replacements and virtual trains.
 */

#ifndef TBTR_TEMPLATE_VEHICLE_H
#define TBTR_TEMPLATE_VEHICLE_H

#include "stdafx.h"
#include "core/pool_func.hpp"
#include "core/pool_type.hpp"

#include "company_func.h"
#include "group.h"
#include "newgrf_spritegroup.h"
#include "train.h"
#include "vehicle_base.h"
#include "vehicle_func.h"

typedef uint16 TemplateID;

#define INVALID_TEMPLATE 0xFFFF

class TemplateVehicle;

#define FOR_ALL_TEMPLATES_FROM(var, start) FOR_ALL_ITEMS_FROM(TemplateVehicle, template_index, var, start)
#define FOR_ALL_TEMPLATES(var) FOR_ALL_TEMPLATES_FROM(var, 0)

/** A pool allowing to store up to ~64k templates */
typedef Pool<TemplateVehicle, TemplateID, 512, 0x10000> TemplatePool;
extern TemplatePool _template_pool;

/** A type of cache for engine cargo capacities */
typedef std::pair<EngineID, CargoID> EngineCargo;
typedef std::map<std::pair<EngineID, CargoID>, uint16> EngineCargoCapacities;

enum TBTR_REPLACEMENT_OPTS {
	TBTR_OPT_REUSE_DEPOT_VEHICLES,
	TBTR_OPT_REFIT_VEHICLE,
	TBTR_OPT_KEEP_REMAINDERS,
};

/** Main Template Vehicle class
 *
 * A template vehicle is basically like a train with a minimal set of attributes. I.e. it is a chain
 * of template vehicles, like a train.
 *
 * All templates are stored in their own pool so that they don't interfere with a company's allowed number of
 * trains. */
struct TemplateVehicle : TemplatePool::PoolItem<&_template_pool>, BaseVehicle {
	/* static members */
public:
	static TemplateID last_template;    ///< remember the ID of the template vehicle that was created last
	static EngineCargoCapacities engine_cargo_cap; ///< A type of cache for engine cargo capacities

	/* non-static members */
public:
	TemplateVehicle();
	TemplateVehicle(EngineID);
	~TemplateVehicle();
	TemplateID index;                   ///< Vehicle index

	TemplateVehicle* next;              ///< pointer to the next template vehicle in the chain
	TemplateVehicle* prev;              ///< pointer to the previous template vehicle in the chain
	TemplateVehicle* first;             ///< pointer to the first template vehicle in the chain
	TemplateVehicle* last;              ///< pointer to the last template vehicle in the chain

	/** essential template info */
	Owner owner;                        ///< template owner

	/** Vehicle type + cargo info */
	EngineID engine_type;               ///< The type of engine used for this vehicle.
	byte subtype;                       ///< The vehicle subtype
	RailTypeByte railtype;              ///< The railtype of this vehicle
	uint16 max_speed;
	uint16 power;
	uint16 weight;
	uint16 max_te;

	CargoID cargo_type;                 ///< type of cargo this vehicle is carrying
	uint16 cargo_cap;                   ///< total capacity

	/** Vehicle drawing information */
	uint16 real_length;                 ///< template length in tile units, for drawing in the gui
	uint32 sprite_height;               ///< used for drawing in a GUI
	uint32 sprite_width;                ///< used for drawing in a GUI
	int sprite_xoff;                    ///< used for drawing in a GUI
	int sprite_yoff;                    ///< used for drawing in a GUI
	bool cached_sprite_size;            ///< whether the sprite dimensions have already been cached for this template

	void Init(EngineID);

public:
	inline TemplateVehicle* Next() const {return this->next;}

	inline uint16 GetRealLength() const {return real_length;}
	inline bool HasOwner(Owner owner) const {return this->owner == owner;}
	inline bool IsPrimaryVehicle() const {return HasBit(this->subtype, GVSF_FRONT);}
	inline bool IsFreeWagonChain() const {return HasBit(this->subtype, GVSF_FREE_WAGON);}

	Money CalculateCost() const;
	void CloneFromTrain(const Train*, TemplateVehicle*);
	bool ContainsRailType(RailType) const;
	/* Count the number of groups which use this template vehicle */
	int CountGroups() const;
	TemplateVehicle* GetNextUnit() const;
	uint GetChainDisplayLength();       ///< the sum of the sprite lengths of this template and all following chain members

	void Draw(uint, uint, int, int, uint16, int, TemplateID);

	bool TrainNeedsReplacement(Train*);

	void SetCargoCapacity();

	void UpdateLastVehicle(TemplateVehicle*);
	void UpdateZoom();					///< take the changed UI zoom into account
};

TemplateID FindTemplateIndexForGroup(GroupID);


/* Command functions */
CommandCost CmdTemplateAddEngine(TileIndex, DoCommandFlag, uint32, uint32, char const*);
CommandCost CmdTemplateReplacement(TileIndex, DoCommandFlag, uint32, uint32, char const*);
CommandCost CmdStartStopTbtr(TileIndex, DoCommandFlag, uint32, uint32, char const*);
CommandCost CmdCloneTemplateFromTrain(TileIndex, DoCommandFlag, uint32, uint32, char const*);

TemplateVehicle* GetTemplateForTrain(Train*);

void ResetTemplateVehicles();

#endif /* !TBTR_TEMPLATE_VEHICLE_H */
