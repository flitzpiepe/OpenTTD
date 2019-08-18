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

#include "tbtr_template_refit_window.h"

enum TemplateRefitWindowWidgets {
	TRFW_CAPTION,
	TRFW_MATRIX_REFITS,
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
		/* Template Panel */
		NWidget(WWT_PANEL, COLOUR_GREY),
			SetFill(1,1), SetMinimalSize(0,20), SetResize(1,0),
		EndContainer(),
		/* Refit list */
		NWidget(WWT_MATRIX, COLOUR_GREY, TRFW_MATRIX_REFITS), SetFill(1,1), SetResize(1,1),
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
	FinishInitNested(VEH_TRAIN);
}

void TemplateRefitWindow::UpdateWidgetSize(int widget, Dimension* size, const Dimension &padding, Dimension* fill, Dimension* resize)
{
	switch (widget) {
		case TRFW_MATRIX_REFITS:
			resize->height = 20;
			size->height = 40;
			break;
	}
}

/*
 * Update GUI components on resize
 */
void TemplateRefitWindow::OnResize()
{
	NWidgetCore* nwi = this->GetWidget<NWidgetCore>(TRFW_MATRIX_REFITS);
	nwi->widget_data = (this->num_cargo_types << MAT_ROW_START) + (1 << MAT_COL_START);
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
