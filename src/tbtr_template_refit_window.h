/*
 * This file is part of OpenTTD.
 * OpenTTD is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 2.
 * OpenTTD is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details. You should have received a copy of the GNU General Public License along with OpenTTD. If not, see <http://www.gnu.org/licenses/>.
 */

/** @file tbtr_template_refit_window.h Window for setting the refit for a template chain */
#ifndef TBTR_TEMPLATE_REFIT_WINDOW_H
#define TBTR_TEMPLATE_REFIT_WINDOW_H

#include "stdafx.h"
#include "window_gui.h"

class TemplateVehicle;
class TbtrGui;

class TemplateRefitWindow : public Window {
	friend TbtrGui;
public:
	TemplateRefitWindow(WindowDesc*, Window*);
private:
	virtual void DrawWidget(const Rect&, int) const;
	virtual void OnClick(Point, int, int);
	virtual void OnResize();
	virtual void UpdateWidgetSize(int, Dimension*, const Dimension&, Dimension*, Dimension*);

	void CreateCargoList();

	void UpdateTemplateVehicle(const TemplateVehicle*, bool);

	Scrollbar* vscroll_refits;
	uint16 num_cargo_types = 11; // this should be the highest cargotype ID; check enum CargoType in cargo_type.h
	SmallVector<const CargoSpec*, 64> cargo_specs;
	uint16 index_selected_refit = -1;
	const TemplateVehicle* selected_template = NULL;
	bool refit_single_engine = false;
	TbtrGui* main_ui;
};

void ShowTemplateRefitWindow(Window*);

#endif /* !TBTR_TEMPLATE_REFIT_WINDOW_H */
