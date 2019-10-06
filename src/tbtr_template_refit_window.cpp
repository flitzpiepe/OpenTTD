/*
 * This file is part of OpenTTD.
 * OpenTTD is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 2.
 * OpenTTD is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details. You should have received a copy of the GNU General Public License along with OpenTTD. If not, see <http://www.gnu.org/licenses/>.
 */

/** @file tbtr_template_refit_window.h Window for setting the refit for a template chain */
#include "stdafx.h"
#include "command_func.h"

#include "tbtr_gui.h"
#include "tbtr_template_refit_window.h"

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
		NWidget(WWT_CAPTION, COLOUR_GREY, TRFW_CAPTION), SetDataTip(STR_TBTR_REFIT_UI_TITLE, STR_TOOLTIP_WINDOW_TITLE_DRAG_THIS),
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
			NWidget(WWT_PUSHTXTBTN, COLOUR_GREY, TRFW_BUTTON_REFIT), SetResize(1,0), SetFill(1,1), SetDataTip(STR_ORDER_REFIT, STR_ORDER_REFIT),
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
TemplateRefitWindow::TemplateRefitWindow(WindowDesc* wdesc, Window* p) : Window(wdesc)
{
	CreateNestedTree(wdesc);
	this->vscroll_refits = GetScrollbar(TRFW_SCROLLBAR_REFITS);
	FinishInitNested(VEH_TRAIN);

	this->parent = p;
	this->main_ui = (TbtrGui*)p;
	this->vscroll_refits->SetStepSize(1);
	this->vscroll_refits->SetCount(this->num_cargo_types);
}

/**
 * Build the list of cargo types to show in the refit window
 */
void TemplateRefitWindow::CreateCargoList() {
	this->cargo_specs.Clear();
	this->cargo_specs.Reset();

	if ( this->selected_template ) {
		/* gather all available refits for the current template chain */
		CargoTypes template_refit_mask = 0x0;
		const TemplateVehicle* tmp = this->selected_template->first;
		while ( tmp ) {
			const Engine* e = Engine::Get(tmp->engine_type);
			template_refit_mask |= e->info.refit_mask;
			tmp = tmp->next;
		}
		/* add all available refits to the list of cargo specs */
		const CargoSpec* cs;
		FOR_ALL_CARGOSPECS(cs) {
			if ( HasBit(template_refit_mask, cs->bitnum) ) {
				*this->cargo_specs.Append(1) = cs;
			}
		}
	}
}

/*
 * Draw a widget of this GUI
 */
void TemplateRefitWindow::DrawWidget(const Rect& r, int widget) const {
	switch (widget) {
		case TRFW_MATRIX_REFITS: {
			int y = r.top;
			for ( unsigned int i=0; i<this->cargo_specs.Length(); ++i ) {
				/* fill the cell of the currently selected refit */
				if ( this->index_selected_refit == (int32)i ) {
					GfxFillRect(r.left, y, r.right, y+this->main_ui->height_cell_templates, _colour_gradient[COLOUR_GREY][3]);
				}
				/* cargo name for the refit */
				DrawString(r.left, r.right, y+this->main_ui->pos_string_hi, this->cargo_specs[i]->name, TC_BLACK);
				y += this->main_ui->height_cell_templates;
			}
		}
	}
}

/*
 * Handle mouse clicks on the GUI
 */
void TemplateRefitWindow::OnClick(Point pt, int widget, int click_count)
{
	switch (widget) {
		case TRFW_MATRIX_REFITS: {
			uint16 index_new = this->vscroll_refits->GetScrolledRowFromWidget(pt.y, this, widget);

			/* clicked empty cell */
			if ( index_new >= this->cargo_specs.Length() )
				this->index_selected_refit = -1;

			/* clicked currently selected cell */
			else if ( index_new == this->index_selected_refit )
				this->index_selected_refit = -1;

			/* clicked cell containing another refit */
			else
				this->index_selected_refit = index_new;

			this->SetDirty();
			break;
		}
		case TRFW_BUTTON_REFIT: {
			if ( this->index_selected_refit < this->cargo_specs.Length() ) {
				/* refit */
				const CargoSpec* cs = *(this->cargo_specs.Get(this->index_selected_refit));
				CargoID cid = cs->Index();
				DoCommandP(0, this->selected_template->index, cid|(this->refit_single_engine<<8), CMD_REFIT_TEMPLATE);

				/* propagate top the main ui */
				Window* tbtr_gui = FindWindowByClass(WC_TBTR_GUI);
				if ( tbtr_gui ) tbtr_gui->SetDirty();
			}
			break;
		}
	}
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

/**
 * Update the template vehicle to refit
 *
 * @param tv:    Pointer to the template vehicle that should be refitted
 */
void TemplateRefitWindow::UpdateTemplateVehicle(const TemplateVehicle* tv, bool single_engine)
{
	this->selected_template = tv;
	this->refit_single_engine = single_engine;
	CreateCargoList();
	this->SetDirty();
}

/**
 * Recalculate the size of the window's components
 */
void TemplateRefitWindow::UpdateWidgetSize(int widget, Dimension* size, const Dimension &padding, Dimension* fill, Dimension* resize)
{
	switch (widget) {
		case TRFW_MATRIX_REFITS:
			resize->height = GetEngineListHeight(VEH_TRAIN);
			size->height = 8 /*(_gui_zoom==0?3:8)*/ * resize->height;
			break;
	}
}

/*
 * Show the template refit window
 */
void ShowTemplateRefitWindow(Window* parent)
{
	Window* w = FindWindowByClass(WC_TBTR_TEMPLATE_REFIT_WINDOW);
	if ( w )
		return;
	new TemplateRefitWindow(&_tbtr_refit_window_desc, parent);
}
