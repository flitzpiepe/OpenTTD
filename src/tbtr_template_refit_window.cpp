/*
 * This file is part of OpenTTD.
 * OpenTTD is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 2.
 * OpenTTD is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details. You should have received a copy of the GNU General Public License along with OpenTTD. If not, see <http://www.gnu.org/licenses/>.
 */

/** @file tbtr_template_refit_window.h Window for setting the refit for a template chain */
#include "stdafx.h"
#include "window_func.h"
#include "window_gui.h"
#include "window_type.h"
#include "engine_base.h"
#include "engine_gui.h"

#include "cargotype.h"

#include "tbtr_template_vehicle.h"
#include "tbtr_template_refit_window.h"

// TODO rm
#include "tbtr_debug.h"

enum TemplateRefitWindowWidgets {
	TRFW_CAPTION,
	TRFW_MATRIX_REFITS,
	TRFW_SCROLLBAR_REFITS,
	TRFW_BUTTON_REFIT,
};

static const NWidgetPart _widgets[] = {
	/* Title bar */
	NWidget(NWID_HORIZONTAL),
		NWidget(WWT_CLOSEBOX, COLOUR_GREY),
		NWidget(WWT_CAPTION, COLOUR_GREY, TRFW_CAPTION), SetDataTip(STR_TEMPLATE_REFIT_TITLE, STR_TOOLTIP_WINDOW_TITLE_DRAG_THIS),
		NWidget(WWT_SHADEBOX, COLOUR_GREY),
		NWidget(WWT_STICKYBOX, COLOUR_GREY),
	EndContainer(),
	/* UI components */
	NWidget(NWID_VERTICAL),
		/* Refit list */
		NWidget(NWID_HORIZONTAL),
			NWidget(WWT_MATRIX, COLOUR_GREY, TRFW_MATRIX_REFITS), SetFill(1,14), SetResize(1,1), SetScrollbar(TRFW_SCROLLBAR_REFITS),
			NWidget(NWID_VSCROLLBAR, COLOUR_GREY, TRFW_SCROLLBAR_REFITS),
		EndContainer(),
		NWidget(NWID_HORIZONTAL),
			NWidget(WWT_TEXTBTN, COLOUR_GREY, TRFW_BUTTON_REFIT), SetResize(1,0), SetFill(1,1), SetDataTip(STR_TEMPLATE_REFIT_BUTTON, STR_TEMPLATE_REFIT_BUTTON),
			NWidget(WWT_RESIZEBOX, COLOUR_GREY),
		EndContainer(),
	EndContainer(),
};

static WindowDesc _tbtr_refit_window_desc(
	WDP_AUTO,
	"Template Refit Window",
	200, 300,
	WC_TBTR_TEMPLATE_REFIT_WINDOW,
	WC_NONE,
	WDF_CONSTRUCTION,
	_widgets, lengthof(_widgets)
);

/*
 * Constructor
 */
TemplateRefitWindow::TemplateRefitWindow(WindowDesc* wdesc) : Window(wdesc)
{
	CreateNestedTree(wdesc);
	this->vscroll_refits = GetScrollbar(TRFW_SCROLLBAR_REFITS);
	FinishInitNested(VEH_TRAIN);

	this->vscroll_refits->SetStepSize(1);
	this->vscroll_refits->SetCount(this->num_cargo_types);
}

void TemplateRefitWindow::UpdateWidgetSize(int widget, Dimension* size, const Dimension &padding, Dimension* fill, Dimension* resize)
{
	switch (widget) {
		case TRFW_MATRIX_REFITS:
			resize->height = GetEngineListHeight(VEH_TRAIN);
			size->height = 8 /*(_gui_zoom==0?3:8)*/ * resize->height;
			break;
	}
}

// TODO comment
// TODO mv
void TemplateRefitWindow::CreateCargoList() {
	this->cargo_specs.Clear();
	this->cargo_specs.Reset();
	if ( this->selected_template ) {
		EngineID eid = this->selected_template->last->engine_type;
		const Engine* e = Engine::Get(eid);
		const CargoSpec* cs;
		FOR_ALL_CARGOSPECS(cs) {
			if ( HasBit(e->info.refit_mask, cs->bitnum) ) {
				*this->cargo_specs.Append(1) = cs;
			}
		}
	}
}

// TODO comment
void TemplateRefitWindow::DrawWidget(const Rect& r, int widget) const {
	switch (widget) {
		case TRFW_MATRIX_REFITS: {
			int y = 20;
			for ( unsigned int i=0; i<this->cargo_specs.Length(); ++i ) {
				DrawString(r.left, r.right, y, this->cargo_specs[i]->name, TC_BLACK);
				y += 14;
			}
		}
	}
}

void TemplateRefitWindow::UpdateTemplateVehicle(TemplateVehicle* tv)
{
	this->selected_template = tv;
	CreateCargoList();
	this->SetDirty();
}

/*
 * Update GUI components on resize
 */
void TemplateRefitWindow::OnResize()
{
	NWidgetCore* nwi = this->GetWidget<NWidgetCore>(TRFW_MATRIX_REFITS);
	this->vscroll_refits->SetCapacityFromWidget(this, TRFW_MATRIX_REFITS);
	nwi->widget_data = (this->vscroll_refits->GetCapacity() << MAT_ROW_START) + (1 << MAT_COL_START);
}

/*
 * Show the template refit window
 */
void ShowTemplateRefitWindow()
{
	Window* w = FindWindowByClass(WC_TBTR_TEMPLATE_REFIT_WINDOW);
	if ( w )
		return;
	new TemplateRefitWindow(&_tbtr_refit_window_desc);
}
