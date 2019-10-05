/* $Id$ */

/*
 * This file is part of OpenTTD.
 * OpenTTD is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 2.
 * OpenTTD is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details. You should have received a copy of the GNU General Public License along with OpenTTD. If not, see <http://www.gnu.org/licenses/>.
 */

/** @file tbtr_template_vehicle.cpp Implementation of the TemplateVehicle class. */

#include "stdafx.h"
#include "window_gui.h"

#include "tbtr_template_vehicle.h"
#include "engine_gui.h"

TemplatePool _template_pool("Template");
INSTANTIATE_POOL_METHODS(Template)

TemplateID TemplateVehicle::last_template = INVALID_TEMPLATE;
EngineCargoCapacities TemplateVehicle::engine_cargo_cap;

/*
 * Default CTOR
 */
TemplateVehicle::TemplateVehicle()
{
	this->Init(INVALID_ENGINE);
}

/*
 * CTOR: initialize this template vehicle with a given engine id
 *
 * @param eid: the engine id for this template
 */
TemplateVehicle::TemplateVehicle(EngineID eid)
{
	this->Init(eid);
}

/*
 * Default desctructor
 */
TemplateVehicle::~TemplateVehicle()
{
	/*
	 * NOP
	 *
	 * Template consistency ensured outside of this scope, e.g. when a part of a template is removed, the
	 * according part of the code needs to take care that prev/next/first/last pointers are all still valid or
	 * be invalidated.
	 *
	 * A single template engine always consists of only one object, i.e. articulated vehicles and such are
	 * resembled as a single template object.
	 */
}

/**
 * Calculate the total cost of buying all vehicles in this template.
 *
 * @return: the money value of the calculated cost
 */
Money TemplateVehicle::CalculateCost() const
{
	Money val = 0;
	const TemplateVehicle* tv = this;
	for (; tv; tv=tv->GetNextUnit())
		val += (Engine::Get(tv->engine_type))->GetCost();
	return val;
}

/**
 * Make this template vehicle match a train. This assumes that this template is not yet part
 * of any chain.
 *
 * @param train:  the (first vehicle of the) train which acts as preimage for the template
 */
void TemplateVehicle::CloneFromTrain(const Train* train, TemplateVehicle* chainHead)
{
	if ( !train )
		return;

	int len = CountVehiclesInChain(train);
	if ( !TemplateVehicle::CanAllocateItem(len) )
		return;

	this->first = chainHead ? chainHead : this;
	this->first->last = this;
	this->engine_type = train->engine_type;
	this->subtype = train->subtype;
	this->railtype = train->railtype;
	this->cargo_type = train->cargo_type;
	this->cargo_cap = train->cargo_cap;
	const GroundVehicleCache* gcache = train->GetGroundVehicleCache();
	this->max_speed = train->GetDisplayMaxSpeed();
	this->power = gcache->cached_power;
	this->weight = gcache->cached_weight;
	this->max_te = gcache->cached_max_te / 1000;

	if ( train->GetNextUnit() ) {
		TemplateVehicle* tv = new TemplateVehicle();
		if ( chainHead == NULL )
			chainHead = this;
		tv->CloneFromTrain(train->GetNextUnit(), chainHead);
		tv->prev = this;
		this->next = tv;
	}

	return;
}

/**
 * Check if this template vehicle contains any locos or wagons of the given rail type.
 *
 * @param railtype: the rail type to check for
 * @return:         true, if there is a unit of the given rail type in this template
 */
bool TemplateVehicle::ContainsRailType(RailType railtype) const
{
	/* filtering disabled */
	if ( railtype == INVALID_RAILTYPE )
		return true;

	const TemplateVehicle* tv = this;
	/* For non-electrified rail engines, the whole chain must not contain any electrified engines or wagons */
	if ( railtype == RAILTYPE_BEGIN || railtype == RAILTYPE_RAIL ) {
		while ( tv ) {
		if ( tv->railtype != railtype )
			return false;
		tv = tv->GetNextUnit();
		}
		return true;
	}
	/* For electrified rail engines, non-electrified engines or wagons are also allowed */
	while ( tv ) {
		if ( tv->railtype == railtype )
			return true;
		tv = tv->GetNextUnit();
	}
	return false;
}

/**
 * Count all groups that are using this template.
 *
 * @return: the count
 */
int TemplateVehicle::CountGroups() const
{
	int count = 0;
	Group* g;
	FOR_ALL_GROUPS(g) {
		if (g->owner == this->owner && g->template_id == this->index)
			++count;
	}
	return count;
}

/**
 * Draw a template
 *
 * @param left:     left border of the bounding box
 * @param right:    right border of the bounding box
 * @param y:        y-coordinate of the bounding box
 * @param x_offset: how many pixels to skip at the start of the template before starting to draw any engine
 *					this is used when the template has to be horizontally scrolled into view
 */
void TemplateVehicle::Draw(uint left, uint right, int y, int y_top, uint16 height, int x_offset=0, TemplateID tid_selected_template_part=-1)
{
	/* cache the sprite dimensions for this template's engine */
	if ( this->cached_sprite_size == false ) {
		GetTrainSpriteSize(this->engine_type, this->sprite_width, this->sprite_height, this->sprite_xoff, this->sprite_yoff, EIT_PURCHASE);
		this->cached_sprite_size = true;
	}

	/* don't draw outside of the bounding box'es area */
	if ( this->sprite_width + left >= right )
		return;

	/* draw this + rest of the chain */
	if ( x_offset <= 0 ) {
		DrawVehicleEngine(left, right, left, y, this->engine_type, GetEnginePalette(this->engine_type, this->owner), EIT_PURCHASE);

		if ( this->index == tid_selected_template_part )
			DrawFrameRect(left, y_top, left+this->sprite_width, y_top+height, COLOUR_WHITE, FR_BORDERONLY);

		left += this->sprite_width;
	}
	TemplateVehicle* next = this->GetNextUnit();
	if ( next )
		next->Draw(left, right, y, y_top, height, x_offset-this->sprite_width, tid_selected_template_part);
}

/**
 * Calculate the sum of all sprite widths of this template and the rest of the chain
 * Not const, might cache the sprite dimensions of a vehicle if it has not already been done.
 */
uint TemplateVehicle::GetChainDisplayLength()
{
	uint sum = 0;
	for ( TemplateVehicle* tmp=this; tmp; tmp=tmp->next ) {
		if ( tmp->cached_sprite_size == false )
			GetTrainSpriteSize(tmp->engine_type, tmp->sprite_width, tmp->sprite_height, tmp->sprite_xoff, tmp->sprite_yoff, EIT_PURCHASE);
		sum += tmp->sprite_width;
	}
	return sum;
}

/*
 * Return the next 'real' unit following this template, i.e. disregarding articulated parts.
 *
 * @return:    the next template vehicle following *this in the consist.
 */
TemplateVehicle* TemplateVehicle::GetNextUnit() const
{
	TemplateVehicle* tv = this->next;
	while ( tv && HasBit(tv->subtype, GVSF_ARTICULATED_PART) ) tv = tv->next;
	if ( tv && HasBit(tv->subtype, GVSF_MULTIHEADED) && !HasBit(tv->subtype, GVSF_ENGINE) )
		tv = tv->next;
	return tv;
}

/**
 * Initialize this template vehicle with default values.
 *
 * @param eid: the engine id for this template
 */
void TemplateVehicle::Init(EngineID eid)
{
	this->next = NULL;
	this->prev = NULL;
	this->first = this;
	this->last = this;

	this->engine_type = eid;
	this->owner = _current_company;
	this->real_length = 0;
	this->cached_sprite_size = false;
}

/**
 * Set this vehicle's cargo capacity.
 *
 * The engine's default capacity might not be suitable because this value may be altered by NewGRF callbacks.
 */
void TemplateVehicle::SetCargoCapacity()
{
	if ( this->engine_type == INVALID_ENGINE || Engine::Get(this->engine_type)->CanCarryCargo() == false )
		return;

	/* the needed engine-id + cargo-id combination */
	EngineCargo ec = EngineCargo(this->engine_type, this->cargo_type);

	/* the cargo capacity for this type of engine + cargo might already be cached */
	auto itca = TemplateVehicle::engine_cargo_cap.find(ec);
	if ( itca != TemplateVehicle::engine_cargo_cap.end() ) {
		this->cargo_cap = itca->second;
		return;
	}

	/* try to find a train with this type of engine + cargo and get the capacity from it */
	const Train* t = NULL;
	FOR_ALL_TRAINS(t) {
		if ( t->engine_type == this->engine_type && t->cargo_type == this->cargo_type ) {
			/* cache it */
			TemplateVehicle::engine_cargo_cap[ec] = t->cargo_cap;
			this->cargo_cap = t->cargo_cap;
			return;
		}
	}

	/* try to create a new train with the needed engine + cargo to determine its capacity */
	if ( Train::CanAllocateItem() ) {
		Train *t = new Train();
		t->engine_type = this->engine_type;
		t->cargo_type = this->cargo_type;
		const Engine* e = Engine::Get(this->engine_type);
		this->cargo_cap = e->DetermineCapacity(t);
		/* cache it */
		TemplateVehicle::engine_cargo_cap[ec] = this->cargo_cap;
		delete t;
		return;
	}

	/* give up and use the engine's default cargo capacity */
	this->cargo_cap = Engine::Get(this->engine_type)->GetDisplayDefaultCapacity();
}

/**
 * Return whether a given train will be treated by template replacement.
 *
 * @t:      the train to check
 * @return: true, if it will be considered for template replacement
 */
bool TemplateVehicle::TrainNeedsReplacement(Train* t)
{
	TemplateVehicle* tv = this;
	while ( tv && t ) {
		if ( t->engine_type != tv->engine_type )
			return true;
		if ( t->subtype != tv->subtype )
			return true;
		if ( t->cargo_type != tv->cargo_type )
			return true;
		tv = tv->GetNextUnit();
		t = t->GetNextUnit();
	}
	/* check if one chain ended before the other */
	return (!tv && t) || (tv && !t);
}

/**
 * Update the "first" pointer on each member of this chain
 *
 * @param last:   the new last vehicle
 */
void TemplateVehicle::UpdateFirstVehicle(TemplateVehicle* first_new)
{
	TemplateVehicle* tmp = this->first;
	while ( tmp ) {
		tmp->first = first_new;
		tmp = tmp->next;
	}
}

/**
 * Update the "last" pointer on each member of this chain
 *
 * @param last:   the new last vehicle
 */
void TemplateVehicle::UpdateLastVehicle(TemplateVehicle* last_new)
{
	TemplateVehicle* tmp = this->first;
	while ( tmp ) {
		tmp->last = last_new;
		tmp = tmp->next;
	}
}

/**
 *	Update things according to the current UI zoom level
 */
void TemplateVehicle::UpdateZoom()
{
	this->cached_sprite_size = false;
}

void ResetTemplateVehicles()
{
	TemplateVehicle::engine_cargo_cap.clear();
}

