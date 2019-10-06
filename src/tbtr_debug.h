/**
 * Debugging features for TBTR
 *
 * I want to keep them out of the git history, so they are just added and removed on the project level.
 * Don't forget to exclude them from commits!
 */

#ifndef TBTR_DEBUG_H
#define TBTR_DEBUG_H

#include <iostream>
#include <iomanip>

#include "stdafx.h"
#include "tbtr_template_vehicle.h"
#include "train.h"
#include "vehicle_base.h"
#include "tbtr_gui.h"

namespace td
{
	void sv(TemplateVehicle* t)
	{
		std::cout << "  ix:" << std::setw(3) << t->index
			<< "  e:" << std::setw(3) << t->engine_type
			<< "  ty:" << std::setw(1) << t->type
			<< "  st:" << std::setw(1) << (short)(t->subtype)
			<< "  ct:" << std::setw(1) << (short)(t->cargo_type)
			<< "  h:" << std::setw(14) << (t->first)
			<< "  p:" << std::setw(14) << (t->prev)
			<< "  n:" << std::setw(14) << (t->next)
			<< "  l:" << std::setw(14) << (t->last)
			<< "  " << t
			<< std::endl;
	}

	void v(TemplateVehicle* t)
	{
		if ( !t ) return;

		if (t->first == t)
			std::cout << "template: (ix):" << t->index << std::endl;

		sv(t);

		v(t->Next());
	}

	void v(Train* t)
	{
		if ( !t ) return;

		if (t->First() == t)
			std::cout << "train: (ix):" << t->index << "  (unit):" << t->unitnumber << std::endl;

		std::cout << "  ix:" << t->index
			<< "  e:" << t->engine_type
			<< "  ty:" << t->type
			<< "  st:" << (short)(t->subtype)
			<< "  ct:" << (short)(t->cargo_type)
			<< "  cs:" << (short)(t->cargo_subtype)
			<< "  ca:" << t->cargo.TotalCount()
			<< "  " << t
			<< std::endl;

		v(t->Next());
	}

	void all_trains()
	{
		Train* t;
		FOR_ALL_TRAINS(t)
		{
			if (t->First() == t)
				v(t);
		}
	}

	void all_template_chains()
	{
		TemplateVehicle* tv;
		FOR_ALL_TEMPLATES(tv)
		{
			if (tv->first == tv)
				v(tv);
		}
	}

	void all_templates()
	{
		TemplateVehicle* tv;
		FOR_ALL_TEMPLATES(tv)
		{
			sv(tv);
		}
	}

	void ecc()
	{
		std::cout << "ecc:" << std::endl;
		for ( auto it = TemplateVehicle::engine_cargo_cap.cbegin();
				it != TemplateVehicle::engine_cargo_cap.cend();
				++it )
		{
			std::cout << it->first.first << "," << (int)it->first.second << "," << it->second << std::endl;
		}
	}

	TbtrGui* gui()
	{
		return (TbtrGui*)FindWindowByClass(WC_TBTR_GUI);
	}
}

#endif
