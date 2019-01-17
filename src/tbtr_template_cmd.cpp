#include "stdafx.h"

#include "autoreplace_func.h"
#include "command_func.h"
#include "core/random_func.hpp"

#include "tbtr_template_vehicle.h"

/**
 * Check if a TemplateVehicle and a Train share the same refit settings.
 *
 * @param tv: the TemplateVehicle
 * @param t:  the Train
 * @return:   true if all refit related settings are the same, false otherwise
 */
bool CheckRefit(const TemplateVehicle* tv, const Train* t)
{
	return tv->cargo_type==t->cargo_type && tv->cargo_subtype==t->cargo_subtype;
}

/**
 * Return the template vehicle that is assigned to a train's group.
 *
 * @param t:	the train
 */
TemplateVehicle* GetTemplateForTrain(Train* t)
{
	GroupID gid = t->group_id;
	if ( gid == DEFAULT_GROUP )
		return NULL;

	TemplateID tid = Group::Get(gid)->template_id;
	if ( tid == INVALID_TEMPLATE )
		return NULL;

	return TemplateVehicle::Get(tid);
}

/**
 * Get the correct subtype for an engine for a new template
 *
 * @param e:         the engine
 * @param is_head:   true if the new template will be the head of a new chain
 */
byte DetermineSubtype(const Engine* e, bool is_head)
{
	byte subtype = 0;
	if ( e->u.rail.railveh_type == RAILVEH_WAGON ) {
		subtype = 4;
		if ( is_head )
			subtype |= (1<<GVSF_FREE_WAGON);
	}
	else {
		subtype = 8;
		if ( is_head )
			subtype |= (1<<GVSF_FRONT);
	}

	return subtype;
}

/**
 * Neutralize a train's status (group, orders, etc).
 * @param train:	the train to be neutralized
 */
CommandCost NeutralizeStatus(Train* train)
{
	CommandCost cc = CommandCost();

	/* remove from current group */
	cc.AddCost(DoCommand(train->tile, DEFAULT_GROUP, train->index, DC_EXEC, CMD_ADD_VEHICLE_GROUP));

	/* reset orders and statistics and such for the train */
	train->current_order = INVALID_ORDER;
	train->current_order_time = 0;
	train->lateness_counter = 0;
	train->timetable_start = 0;
	train->service_interval = 0;
	train->cur_real_order_index = INVALID_VEH_ORDER_ID ;
	train->cur_implicit_order_index = INVALID_VEH_ORDER_ID;
	train->vehicle_flags = 0;
	train->profit_this_year = 0;
	train->profit_last_year = 0;

	/* unshare and delete */
	cc.AddCost(DoCommand(train->tile, train->index, -1, DC_EXEC, CMD_DELETE_ORDER));

	/* make sure the vehicle is stopped */
	train->vehstatus |= VS_STOPPED;

	return cc;
}

/**
 * Break up a train into neutral chains inside the depot
 *
 * Engines are moved onto a new line each while the wagons will from a FreeWagonChain.
 * Orders and group assignment will be removed from the primary vehicles and it is ensured that they are
 * stopped in the depot.
 *
 * @param train:	the chain to be processed
 */
CommandCost NeutralizeRemainderChain(Train* train) {
	CommandCost cc = CommandCost();
	Train* nextVeh = train->GetNextUnit();
	while ( train != NULL ) {
		if ( HasBit(train->subtype, GVSF_ENGINE) ) {
			cc.AddCost(DoCommand(train->tile, train->index, INVALID_VEHICLE, DC_EXEC, CMD_MOVE_RAIL_VEHICLE));
			NeutralizeStatus(train);
		}
		train = nextVeh;
		if (nextVeh != NULL)
			nextVeh = nextVeh->GetNextUnit();
	}
	return cc;
}

/**
 * Ensure that a train is not part of another given chain.
 *
 * @param t:     the train
 * @param chain: the chain that must not include t
 * @return:      true if t is NOT a part of chain
 */
bool NotInChain(const Train* t, const Train* chain)
{
	while ( chain ) {
		if ( t == chain )
			return false;
		chain = chain->GetNextUnit();
	}
	return true;
}

/**
 * Transfer as much cargo from a given train onto another train.
 *
 * The cargo shall be moved as far as it fits onto the new train.
 * No priority is given to any type of cargo, i.e. the first cargo that is found in the src chain will be transfered first.
 *
 * @param src:  the train from which the cargo will be moved away
 * @param dest: the train from which the cargo will be moved to
 */
void TransferCargo(Train* src, Train* dest)
{
	assert(dest->IsPrimaryVehicle());

	while ( src ) {
		CargoID _cargo_type = src->cargo_type;
		byte _cargo_subtype = src->cargo_subtype;

		/* how much cargo has to be moved (if possible) */
		uint remainingAmount = src->cargo.TotalCount();
		/* each vehicle in the new chain shall be given as much of the old cargo as possible, until none is left */
		for (Train* tmp=dest; tmp!=NULL && remainingAmount>0; tmp=tmp->GetNextUnit()) {
			if (tmp->cargo_type == _cargo_type && tmp->cargo_subtype == _cargo_subtype) {
				/* calculate the free space for new cargo on the current vehicle */
				uint curCap = tmp->cargo_cap - tmp->cargo.TotalCount();
				uint moveAmount = min(remainingAmount, curCap);
				/* move (parts of) the old vehicle's cargo onto the current vehicle of the new chain */
				if (moveAmount > 0) {
					src->cargo.Shift(moveAmount, &tmp->cargo);
					remainingAmount -= moveAmount;
				}
			}
		}
		src = src->GetNextUnit();
	}

	/* Update train weight etc */
	dest->ConsistChanged(ConsistChangeFlags::CCF_LOADUNLOAD);
}

/**
 * Find the first, best matching vehicle of a train for a given template vehicle.
 *
 * In any case the train must match the template's engine type. Among all of those we select the one
 * that also matches the refit and among those the one with the maximum amount of cargo.
 *
 * @param tv:     the template we are looking for
 * @param train:  the train we are looking in
 * @return:       pointer to the train we found, may be null
 */
Train* FindMatchingTrainInChain(TemplateVehicle* tv, Train* train)
{
	Train* found = NULL;
	bool check_refit = tv->first->refit_as_template;
	for ( Train* tmp=train; tmp!=NULL; tmp=tmp->GetNextUnit() ) {
		/* minimal matching condition: by engine_type */
		if ( tmp->engine_type == tv->engine_type ) {
			/* first matching engine_type we take! */
			if ( found == NULL )
				found = tmp;
			/* in case we're also interested in the refit setting */
			if ( check_refit && CheckRefit(tv,tmp)==true ) {
				/* the previously selected train had the wrong refit so this one is definitely better */
				if ( CheckRefit(tv,found) == false ) {
					found = tmp;
				}
				/* or both refits match but the current one has more cargo */
				else if ( tmp->cargo.StoredCount() > found->cargo.StoredCount() ) {
					found = tmp;
				}
			}
		}
	}
	return found;
}

/**
 * Find the first, best matching vehicle inside a depot for a given template vehicle.
 *
 * In any case the train must match the template's engine type. Among all of those we select the one
 * that also matches the refit and among those the one with the maximum amount of cargo.
 * Only trains which are not in a specific group will be considered.
 *
 * @param tv:     the template vehicle which's configuration we are looking for
 * @param tile:   the tile of the depot
 * @param ignore: vehicle must not be in this chain, default is NULL
 * @return:       pointer to the train we found, may be null
 */
Train* FindMatchingTrainInDepot(TemplateVehicle* tv, TileIndex tile, Train* ignore=NULL)
{
	Train* found = NULL;
	Train* train;
	bool check_refit = tv->first->refit_as_template;
	FOR_ALL_TRAINS(train) {
		/* If the veh belongs to a chain, wagons will not return true on IsStoppedInDepot(),
		 * only primary vehicles will in case of t not a primary veh, we demand it to be a
		 * free wagon to consider it for replacement */
		if ( train->tile == tile
				&& ((train->IsPrimaryVehicle() && train->IsStoppedInDepot()) || train->IsFreeWagon())
				&& train->engine_type == tv->engine_type
				&& train->group_id == DEFAULT_GROUP
				&& (ignore==NULL || NotInChain(train, ignore)) ) {
			/* already found a matching vehicle, keep checking for matching refit + cargo amount */
			if ( found != NULL && check_refit == true) {
				if ( train->cargo_type==tv->cargo_type && train->cargo_subtype==tv->cargo_subtype )
					/* find something with a minimal amount of cargo, so that we can transfer more
					 * from the original chain into it later */
					if ( train->cargo.StoredCount() < found->cargo.StoredCount() )
						found = train;
			}
			else
				found = train;
		}
	}
	return found;
}

/**
 * Perform the actual template replacement, or just simulate it. Return the overall cost for the whole replacement in any case.
 *
 * @param tile:     the tile of the incoming train
 * @param flags:    command flags
 * @param p1:       first parameter list
 * @param p2:       second parameter list
 * @param msg:      command message
 */
CommandCost CmdTemplateReplacement(TileIndex ti, DoCommandFlag flags, uint32 p1, uint32 p2, char const* msg)
{
	VehicleID id_inc = GB(p1, 0, 20);
	Train* incoming = Train::GetIfValid(id_inc);
	if ( incoming == NULL ) return CMD_ERROR;
	if ( incoming->vehstatus & VS_CRASHED ) return CMD_ERROR;

	CommandCost ret = CheckOwnership(incoming->owner);
	if ( ret.Failed() ) return ret;

	Train *new_chain=0;
	TileIndex tile = incoming->tile;
	TemplateVehicle* template_vehicle = GetTemplateForTrain(incoming);
	CommandCost cc(EXPENSES_NEW_VEHICLES);

	bool stayInDepot = p2;
	bool sellRemainders = !template_vehicle->keep_remaining_vehicles;
	bool refit_train = template_vehicle->refit_as_template;

	/* first some tests on necessity and sanity */
	if ( template_vehicle == NULL )
		return CMD_ERROR;

	/* remember for CopyHeadSpecificThings() */
	Train* old_head = incoming;

	/* taken from autoreplace_cmd.cpp:
	 *
	 * We have to construct the new vehicle chain to test whether it is valid.
	 * Vehicle construction needs random bits, so we have to save the random seeds
	 * to prevent desyncs and to replay newgrf callbacks during DC_EXEC */
	SavedRandomSeeds saved_seeds;
	SaveRandomSeeds(&saved_seeds);

	/* backup the incoming chain in order to possibly restore it */
	std::list<VehicleID> backup_incoming;
	/* remember the IDs of vehicles that were bought during the replacement process */
	std::list<VehicleID> backup_new_vehicles;
	if ( flags == DC_EXEC ) {
		Train* tmp = incoming;
		while ( tmp ) {
			backup_incoming.push_back(tmp->index);
			tmp = tmp->GetNextUnit();
		}
	}

	/*
	 * Procedure
	 *
	 * For each vehicle in the template, create a matching train part.
	 * The new vehicle may be obtained by:
	 * 		- finding a matching one in the incoming train
	 * 		- finding a matching one in the depot (if the template says so)
	 * 		- buying a new one
	 * The new vehicle is refitted as the template (if the template says so).
	 */
	for ( TemplateVehicle* cur_tmpl=template_vehicle ; cur_tmpl!=NULL ; cur_tmpl=cur_tmpl->GetNextUnit() ) {
		/* try to find a matching vehicle in the incoming train */
		Train* new_vehicle = FindMatchingTrainInChain(cur_tmpl, incoming);

		/* nothing found -> try to find a matching vehicle in the depot */
		if ( new_vehicle == NULL && template_vehicle->reuse_depot_vehicles )
			new_vehicle = FindMatchingTrainInDepot(cur_tmpl, tile, new_chain);

		/* found a matching vehicle somewhere: use it ... */
		if ( new_vehicle != NULL ) {
			/* find the first vehicle in incoming, which is != new_vehicle */
			incoming = (new_vehicle == incoming) ? incoming->GetNextUnit() : incoming;

			if ( new_chain == NULL ) {
				/* move the vehicle from the old chain to the new */
				CommandCost ccMove = DoCommand(tile, new_vehicle->index, INVALID_VEHICLE, flags, CMD_MOVE_RAIL_VEHICLE);
				if ( flags == DC_EXEC )
					cc.AddCost(ccMove);
				new_chain = new_vehicle;
			 }
			 else {
				CommandCost ccMove = DoCommand(tile, new_vehicle->index, new_chain->Last()->index, flags, CMD_MOVE_RAIL_VEHICLE);
				if ( flags == DC_EXEC )
					cc.AddCost(ccMove);
			 }
		}
		/* ... otherwise buy a new one */
		else {
			CommandCost ccBuild = DoCommand(tile, cur_tmpl->engine_type, 0, flags, CMD_BUILD_VEHICLE);
			cc.AddCost(ccBuild);
			new_vehicle = Train::Get(_new_vehicle_id);
			/* remember this vehicle in case we want to restore the original train later */
			if ( flags == DC_EXEC )
				backup_new_vehicles.push_back(new_vehicle->index);

			/* form the new chain */
			if ( new_chain == NULL ) {
				new_chain = new_vehicle;
				CommandCost ccMove = DoCommand(tile, new_chain->index, INVALID_VEHICLE, flags, CMD_MOVE_RAIL_VEHICLE);
				/* a move to INVALID_VEHICLE will fail under flags==DC_NONE */
				if ( flags == DC_EXEC )
					cc.AddCost(ccMove);
			}
			/* or just append to it, if it already exists */
			else {
				CommandCost ccMove = DoCommand(tile, new_vehicle->index, new_chain->Last()->index, flags, CMD_MOVE_RAIL_VEHICLE);
				if ( flags == DC_EXEC )
					cc.AddCost(ccMove);
			}
		}

		/* maybe refit as template */
		if ( refit_train ) {
			CargoID cargo_type = cur_tmpl->cargo_type;
			byte cargo_subtype = cur_tmpl->cargo_subtype;
			CommandCost ccRefit = DoCommand(0, new_vehicle->index, cargo_type | (cargo_subtype<<8) | (1<<16), flags, GetCmdRefitVeh(new_vehicle));
			if ( flags==DC_EXEC )
				cc.AddCost(ccRefit);
		}

		/* restore seeds from before the replacement */
		RestoreRandomSeeds(saved_seeds);
	}

	/* restore backup */
	if ( flags == DC_EXEC && cc.Succeeded() == false ) {
		CommandCost ccRestore = CommandCost();
		auto it = backup_incoming.begin();

		/* re-create the original chain */
		DoCommand(tile, *it, INVALID_VEHICLE, flags, CMD_MOVE_RAIL_VEHICLE);
		Train* restore_chain = Train::Get(*it);
		for ( ++it; it != backup_incoming.end(); ++it )
			DoCommand(tile, *it, restore_chain->Last()->index, flags, CMD_MOVE_RAIL_VEHICLE);

		/* sell all vehicles that have been bought during the replacement */
		for ( auto bought=backup_new_vehicles.begin(); bought!=backup_new_vehicles.end(); ++bought ) {
			CommandCost sell = DoCommand(tile, *bought, 0, flags, CMD_SELL_VEHICLE);
			ccRestore.AddCost(sell);
			_new_vehicle_id = 0;
		}

		/* launch the original train again */
		restore_chain->vehstatus &= ~VS_STOPPED;
		return CMD_ERROR;
	}

	/* some postprocessing steps */
	if ( flags == DC_EXEC ) {
		/* train orders, group, etc. */
		CommandCost ccCopy = CopyHeadSpecificThings(old_head, new_chain, flags);
		cc.AddCost(ccCopy);

		/* cargo */
		if ( incoming )
			TransferCargo(incoming, new_chain);

		/* make the remainders sit peacefully in the depot */
		if ( !sellRemainders ) {
			if ( incoming && incoming != new_chain )
				incoming->unitnumber = GetFreeUnitNumber(incoming->type);
			if ( incoming )
				NeutralizeRemainderChain(incoming);
		}

		/* launch new chain */
		if ( !stayInDepot )
			new_chain->vehstatus &= ~VS_STOPPED;
	}

	/* sell remainders */
	if ( sellRemainders && incoming!=NULL )
		cc.AddCost(DoCommand(incoming->tile, incoming->index|(1<<20), 0, flags, CMD_SELL_VEHICLE));

	return cc;
}

/**
 * Append a new engine to a template vehicle; if it doesn't exist yet, create a new template chain
 *
 * @param ti:   not used
 * @param p1:   pointer to the template vehicle, this can be any member of a template train or even NULL, in
 *              which case a new chain will be created
 * @param p2:   engine ID to be added
 * @param msg:  not used
 *
 * @return:     either a default CommandCost object or CMD_ERROR
 */
CommandCost CmdTemplateAddEngine(TileIndex ti, DoCommandFlag flags, uint32 p1, uint32 p2, char const* msg)
{
	TemplateID tid = static_cast<TemplateID>(p1);
	const Engine* engine = Engine::Get(p2);

	if ( flags == DC_EXEC) {
		if (!TemplateVehicle::CanAllocateItem())
			return CMD_ERROR;
		TemplateVehicle* tv = new TemplateVehicle(p2);

		TemplateVehicle* head = TemplateVehicle::GetIfValid(tid);
		if ( head ) {
			head = head->first;
			head->last->next = tv;
			tv->prev = head->last;
			head->UpdateLastVehicle(tv);
			tv->first = head;
			tv->subtype = DetermineSubtype(engine, false);
		}
		else {
			tv->subtype = DetermineSubtype(engine, true);
		}

		tv->railtype = engine->u.rail.railtype;
		tv->cargo_type = engine->GetDefaultCargoType();
		tv->cargo_subtype = 0;
		tv->cargo_cap = engine->GetDisplayDefaultCapacity();
		tv->max_speed = engine->GetDisplayMaxSpeed();
		tv->power = engine->GetPower();
		tv->weight = engine->GetDisplayWeight();
		tv->max_te = engine->GetDisplayMaxTractiveEffort();
	}

	return CommandCost();
}

/**
 * Delete the last engine of a template
 *
 * @param tile:  not used
 * @param p1:    template id, this is assumed to be the head of the template chain
 * @param p2:    not used
 * @param msg:   not used
 */
CommandCost CmdTemplateDeleteEngine(TileIndex tile, DoCommandFlag flags, uint32 p1, uint32 p2, char const* msg)
{
	TemplateVehicle* tv = TemplateVehicle::Get(p1);
	if ( tv == NULL )
		return CMD_ERROR;

	if ( flags == DC_EXEC ) {
		if ( tv == tv->last ) {
			delete tv;
			return CommandCost();
		}
		TemplateVehicle* last = tv->last;
		last->prev->next = NULL;
		tv->last = last->prev;
		delete last;
		tv->UpdateLastVehicle(tv->last);
	}

	return CommandCost();
}

/**
 * Start or stop the template relacement for a given group by assigning a template to it
 *
 * @param tile:     unused
 * @param flags:    command flags
 * @param p1:       ID of the group, bit 16: whether to start the replacement
 * @param p2:       ID of the template
 * @param msg:      unused
 */
CommandCost CmdStartStopTbtr(TileIndex ti, DoCommandFlag flags, uint32 p1, uint32 p2, char const* msg)
{
	GroupID gid = (GroupID)p1;
	Group* g = Group::Get(gid);
	if ( g == NULL )
		return CMD_ERROR;

	bool start_replacement = HasBit(p1, 16);

	if ( start_replacement ) {
		TemplateID tid = (TemplateID)p2;
		g->template_id = tid;
	}
	else {
		g->template_id = INVALID_TEMPLATE;
	}
	return CommandCost();
}

/**
 * Toggle an option for a given template, e.g. whether to sell or keep the remainders
 *
 * @param tile:     unused
 * @param flags:    command flags
 * @param p1:       ID of the template
 * @param p2:       TBTR_REPLACEMENT_OPTS option to toggle
 * @param msg:      unused
 */
CommandCost CmdToggleTemplateOption(TileIndex ti, DoCommandFlag flags, uint32 p1, uint32 p2, char const* msg)
{
	TemplateVehicle* tv = TemplateVehicle::Get(p1);
	if ( tv == NULL )
		return CMD_ERROR;

	if ( flags == DC_EXEC ) {
		switch (p2) {
			case TBTR_OPT_KEEP_REMAINDERS: {
				tv->keep_remaining_vehicles = !tv->keep_remaining_vehicles;
				break;
			}
			case TBTR_OPT_REFIT_VEHICLE: {
				tv->refit_as_template = !tv->refit_as_template;
				break;
			}
			case TBTR_OPT_REUSE_DEPOT_VEHICLES: {
				tv->reuse_depot_vehicles = !tv->reuse_depot_vehicles;
				break;
			}
		}
	}
	return CommandCost();
}

/**
 * Clone a template vehicle from an existing train
 *
 * @param tile:     unused
 * @param flags:    command flags
 * @param p1:       the Train to be cloned
 * @param p2:       unused
 * @param msg:      unused
 */
CommandCost CmdCloneTemplateFromTrain(TileIndex ti, DoCommandFlag flags, uint32 p1, uint32 p2, char const* msg)
{
	Train* train = Train::Get(p1);
	if ( train == NULL )
		return CMD_ERROR;

	if (!TemplateVehicle::CanAllocateItem())
		return CMD_ERROR;

	if ( flags == DC_EXEC ) {
		TemplateVehicle* tv  = new TemplateVehicle(train->engine_type);
		tv->CloneFromTrain(train, NULL);
		tv->real_length = CeilDiv(train->gcache.cached_total_length * 10, TILE_SIZE);
	}

	return CommandCost();
}

/**
 * Delete a TemplateVehicle
 *
 * @param tile:     unused
 * @param flags:    command flags
 * @param p1:       ID of the template to delete
 * @param p2:       unused
 * @param msg:      unused
 */
CommandCost CmdDeleteTemplate(TileIndex ti, DoCommandFlag flags, uint32 p1, uint32 p2, char const* msg)
{
	TemplateID tid = p1;
	TemplateVehicle* tv = TemplateVehicle::Get(tid);
	if ( tv == NULL )
		return CMD_ERROR;

	if ( flags == DC_EXEC ) {
		Group* g;
		FOR_ALL_GROUPS(g) {
			if ( g->template_id == tid )
				g->template_id = INVALID_TEMPLATE;
		}
		delete tv;
	}

	return CommandCost();
}
