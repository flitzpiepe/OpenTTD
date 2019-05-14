/* $Id$ */

/*
 * This file is part of OpenTTD.
 * OpenTTD is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 2.
 * OpenTTD is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details. You should have received a copy of the GNU General Public License along with OpenTTD. If not, see <http://www.gnu.org/licenses/>.
 */

/** @file tbtr_gui.cpp Main GUI for the Template-based train replacement patch. */

#include "stdafx.h"
#include "string_func.h"
#include "tbtr_gui.h"
#include "command_func.h"
#include "engine_gui.h"
#include "zoom_func.h"

enum TemplateReplaceWindowWidgets {
	TRW_CAPTION,

	TRW_WIDGET_MATRIX_GROUPS,
	TRW_WIDGET_SCROLLBAR_GROUPS,

	TRW_WIDGET_MATRIX_TEMPLATES,
	TRW_WIDGET_SCROLLBAR_TEMPLATES,
	TRW_WIDGET_SCROLLBAR_TEMPLATES_HORIZ,

	TRW_WIDGET_MATRIX_ENGINES,
	TRW_WIDGET_SCROLLBAR_ENGINES,

	TRW_WIDGET_TMPL_INFO_PANEL,

	TRW_WIDGET_TMPL_BUTTONS_ADD,
	TRW_WIDGET_TMPL_BUTTONS_CLONE,
	TRW_WIDGET_TMPL_BUTTONS_DELETE,
	TRW_WIDGET_TMPL_BUTTONS_DELETE_LAST_VEH,
	TRW_WIDGET_TMPL_BUTTON_FLUFF,
	TRW_WIDGET_TMPL_BUTTONS_EDIT_RIGHTPANEL,

	TRW_WIDGET_TITLE_INFO_GROUP,
	TRW_WIDGET_TITLE_INFO_TEMPLATE,

	TRW_WIDGET_INFO_GROUP,
 	TRW_WIDGET_INFO_TEMPLATE,

	TRW_WIDGET_TMPL_BUTTONS_SPACER,

	TRW_WIDGET_START,
	TRW_WIDGET_TRAIN_FLUFF,
	TRW_WIDGET_STOP,

	TRW_WIDGET_SEL_TMPL_DISPLAY_CREATE,
};

#define MIN_WIDTH_LEFT 216
#define MIN_WIDTH_RIGHT 400

static const NWidgetPart _widgets[] = {
	/* Title bar */
	NWidget(NWID_HORIZONTAL),
		NWidget(WWT_CLOSEBOX, COLOUR_GREY),
		NWidget(WWT_CAPTION, COLOUR_GREY, TRW_CAPTION), SetDataTip(STR_TBTR_UI_TITLE, STR_TOOLTIP_WINDOW_TITLE_DRAG_THIS),
		NWidget(WWT_SHADEBOX, COLOUR_GREY),
		NWidget(WWT_STICKYBOX, COLOUR_GREY),
	EndContainer(),

	/* Toplevel container */
	NWidget(NWID_VERTICAL),

		/* Matrixes */
		NWidget(NWID_HORIZONTAL),
			/* New engines */
			NWidget(NWID_VERTICAL),

				NWidget(WWT_PANEL, COLOUR_GREY),
					NWidget(WWT_LABEL, COLOUR_GREY), SetDataTip(STR_TBTR_UI_LABEL_ENGINES, STR_TBTR_UI_LABEL_ENGINES), SetFill(1, 0), SetMinimalSize(0, 12), SetResize(1, 0),
				EndContainer(),
				NWidget(NWID_HORIZONTAL),
					NWidget(WWT_MATRIX, COLOUR_GREY, TRW_WIDGET_MATRIX_ENGINES), SetMinimalSize(MIN_WIDTH_LEFT,12), SetFill(1,1), SetDataTip(0x1, STR_TBTR_UI_TOOLTIP_ENGINES), SetResize(1, 0), SetScrollbar(TRW_WIDGET_SCROLLBAR_ENGINES),
					NWidget(NWID_VSCROLLBAR, COLOUR_GREY, TRW_WIDGET_SCROLLBAR_ENGINES),
				EndContainer(),
			EndContainer(),
			/* Template Ctrl */
			NWidget(NWID_VERTICAL),
				/* Groups Display */
				NWidget(NWID_VERTICAL),
					NWidget(WWT_PANEL, COLOUR_GREY),
						NWidget(WWT_LABEL, COLOUR_GREY), SetDataTip(STR_TBTR_UI_LABEL_GROUPS, 0x0), SetFill(1, 0), SetMinimalSize(0, 12), SetResize(1, 0),
					EndContainer(),
					NWidget(NWID_HORIZONTAL),
						NWidget(WWT_MATRIX, COLOUR_GREY, TRW_WIDGET_MATRIX_GROUPS), SetMinimalSize(MIN_WIDTH_RIGHT, 0), SetFill(1, 1), SetDataTip(0x1, STR_TBTR_UI_TOOLTIP_GROUPS), SetResize(1, 0), SetScrollbar(TRW_WIDGET_SCROLLBAR_GROUPS),
						NWidget(NWID_VSCROLLBAR, COLOUR_GREY, TRW_WIDGET_SCROLLBAR_GROUPS),
					EndContainer(),
				EndContainer(),
				/* Template Display */
				NWidget(NWID_VERTICAL),
					NWidget(WWT_PANEL, COLOUR_GREY),
						NWidget(WWT_LABEL, COLOUR_GREY), SetDataTip(STR_TBTR_UI_LABEL_TEMPLATES, STR_TBTR_UI_LABEL_TEMPLATES), SetFill(1, 0), SetMinimalSize(0, 12), SetResize(1, 0),
					EndContainer(),
					NWidget(NWID_VERTICAL),
						NWidget(NWID_HORIZONTAL),
							NWidget(WWT_MATRIX, COLOUR_GREY, TRW_WIDGET_MATRIX_TEMPLATES), SetMinimalSize(MIN_WIDTH_RIGHT, 0), SetFill(1, 1), SetDataTip(0x1, STR_TBTR_UI_TOOLTIP_TEMPLATES), SetResize(1, 1), SetScrollbar(TRW_WIDGET_SCROLLBAR_TEMPLATES),
							NWidget(NWID_VSCROLLBAR, COLOUR_GREY, TRW_WIDGET_SCROLLBAR_TEMPLATES),
						EndContainer(),
						NWidget(NWID_HSCROLLBAR, COLOUR_GREY, TRW_WIDGET_SCROLLBAR_TEMPLATES_HORIZ),
					EndContainer(),
				EndContainer(),
				/* Info Area */
				NWidget(NWID_VERTICAL),
					NWidget(WWT_PANEL, COLOUR_GREY),
						NWidget(WWT_LABEL, COLOUR_GREY), SetDataTip(STR_TBTR_UI_LABEL_TEMPLATE_INFO, 0x0), SetFill(1, 0), SetMinimalSize(0, 12), SetResize(1, 0),
					EndContainer(),
					NWidget(WWT_PANEL, COLOUR_GREY, TRW_WIDGET_TMPL_INFO_PANEL), SetMinimalSize(MIN_WIDTH_RIGHT,50), SetResize(1,0), EndContainer(),
				EndContainer(),
			EndContainer(),	// END Template Ctrl
		EndContainer(), // END Matrixes

		/* Buttons Area */
		NWidget(NWID_VERTICAL),
			/* Edit buttons */
			NWidget(NWID_HORIZONTAL),
				NWidget(WWT_PUSHTXTBTN, COLOUR_GREY, TRW_WIDGET_TMPL_BUTTONS_ADD), SetMinimalSize(100,12), SetResize(1,0), SetDataTip(STR_TBTR_UI_BUTTON_ADD_ENGINE, STR_TBTR_UI_TOOLTIP_ADD_ENGINE),
				NWidget(WWT_PUSHTXTBTN, COLOUR_GREY, TRW_WIDGET_TMPL_BUTTONS_DELETE_LAST_VEH), SetMinimalSize(75,12), SetResize(1,0), SetDataTip(STR_TBTR_UI_BUTTON_DELETE_ENGINE, STR_TBTR_UI_TOOLTIP_DELETE_ENGINE),
				NWidget(WWT_PANEL, COLOUR_GREY, TRW_WIDGET_TMPL_BUTTONS_EDIT_RIGHTPANEL), SetMinimalSize(50,12), SetResize(1,0), EndContainer(),
				NWidget(WWT_TEXTBTN, COLOUR_GREY, TRW_WIDGET_TMPL_BUTTONS_CLONE), SetMinimalSize(75,12), SetResize(1,0), SetDataTip(STR_TBTR_UI_BUTTON_CLONE_TEMPLATE, STR_TBTR_UI_TOOLTIP_CLONE_TEMPLATE),
				NWidget(WWT_PUSHTXTBTN, COLOUR_GREY, TRW_WIDGET_TMPL_BUTTONS_DELETE), SetMinimalSize(75,12), SetResize(1,0), SetDataTip(STR_TBTR_UI_BUTTON_DELETE_TEMPLATE, STR_TBTR_UI_TOOLTIP_DELETE_TEMPLATE),
			EndContainer(),
		EndContainer(),
		/* Start/Stop buttons */
		NWidget(NWID_HORIZONTAL),
			NWidget(WWT_PUSHTXTBTN, COLOUR_GREY, TRW_WIDGET_START), SetMinimalSize(150, 12), SetResize(1,0), SetDataTip(STR_TBTR_UI_BUTTON_START_REPLACEMENT, STR_TBTR_UI_TOOLTIP_START_REPLACEMENT),
			NWidget(WWT_PANEL, COLOUR_GREY, TRW_WIDGET_TRAIN_FLUFF), SetMinimalSize(15, 12), SetResize(1,0), EndContainer(),
			NWidget(WWT_PUSHTXTBTN, COLOUR_GREY, TRW_WIDGET_STOP), SetMinimalSize(150, 12), SetResize(1,0), SetDataTip(STR_TBTR_UI_BUTTON_STOP_REPLACEMENT, STR_TBTR_UI_TOOLTIP_STOP_REPLACEMENT),
			NWidget(WWT_RESIZEBOX, COLOUR_GREY),
		EndContainer(),

	EndContainer(), // END Toplevel container
};

static WindowDesc _tbtr_gui_desc(
	WDP_AUTO,
	"TBTR Gui",
	456, 156,
	WC_TBTR_GUI,
	WC_NONE,
	WDF_CONSTRUCTION,
	_widgets, lengthof(_widgets)
);

/** Sort the groups by their name */
static int CDECL GroupNameSorter(const Group * const *a, const Group * const *b)
{
	static char         last_name[2][64] = { "", "" };
	static const Group *last_group[2] = { NULL, NULL };

	if (*a != last_group[0]) {
		last_group[0] = *a;
		SetDParam(0, (*a)->index);
		GetString(last_name[0], STR_GROUP_NAME, lastof(last_name[0]));
	}

	if (*b != last_group[1]) {
		last_group[1] = *b;
		SetDParam(0, (*b)->index);
		GetString(last_name[1], STR_GROUP_NAME, lastof(last_name[1]));
	}
	int r = strnatcmp(last_name[0], last_name[1]); // Sort by name (natural sorting).
	if (r == 0) return (*a)->index - (*b)->index;
	return r;
}

/* Sorting functions copied from build_vehicle_gui.cpp */

/**
 * Determines order of engines by engineID
 * @param *a first engine to compare
 * @param *b second engine to compare
 * @return for descending order: returns < 0 if a < b and > 0 for a > b. Vice versa for ascending order and 0 for equal
 */
static int CDECL EngineNumberSorter(const EngineID *a, const EngineID *b)
{
	int r = Engine::Get(*a)->list_position - Engine::Get(*b)->list_position;

	return _engine_sort_direction ? -r : r;
}

/**
 * Determines order of train engines by engine / wagon
 * @param *a first engine to compare
 * @param *b second engine to compare
 * @return for descending order: returns < 0 if a < b and > 0 for a > b. Vice versa for ascending order and 0 for equal
 */
static int CDECL TrainEnginesThenWagonsSorter(const EngineID* a, const EngineID* b)
{
	int val_a = (RailVehInfo(*a)->railveh_type == RAILVEH_WAGON ? 1 : 0);
	int val_b = (RailVehInfo(*b)->railveh_type == RAILVEH_WAGON ? 1 : 0);
	int r = val_a - val_b;

	/* Use EngineID to sort instead since we want consistent sorting */
	if (r == 0) return EngineNumberSorter(a, b);
	return _engine_sort_direction ? -r : r;
}


/**
 * CcTemplateEngineAdded
 *
 * Command callback when a new engine has been added to an existing template or while creating a new template.
 */
void CcTemplateEngineAdded(const CommandCost &result, TileIndex tile, uint32 p1, uint32 p2)
{
	if ( result.Succeeded() ) {
		TbtrGui* tbtrGui = static_cast<TbtrGui*>(FindWindowByClass(WC_TBTR_GUI));
		tbtrGui->UpdateGUI(ENGINE_ADDED);
	}
}

/**
 * CcTemplateEngineDeleted
 *
 * Command callback when an engine has been removed from a template.
 */
void CcTemplateEngineDeleted(const CommandCost &result, TileIndex tile, uint32 p1, uint32 p2)
{
	if ( result.Succeeded() ) {
		TbtrGui* tbtrGui = static_cast<TbtrGui*>(FindWindowByClass(WC_TBTR_GUI));
		tbtrGui->UpdateGUI(ENGINE_DELETED);

	}
}

/**
 * CcTemplateClonedFromTrain
 *
 * Command callback when a template has been cloned from an existing train.
 */
void CcTemplateClonedFromTrain(const CommandCost &result, TileIndex tile, uint32 p1, uint32 p2)
{
	if ( result.Succeeded() ) {
		TbtrGui* tbtrGui = static_cast<TbtrGui*>(FindWindowByClass(WC_TBTR_GUI));
		tbtrGui->UpdateGUI(TEMPLATE_CLONED);
	}
}

/**
 * CcTemplateDeleted
 *
 * Command callback when a template has been deleted.
 */
void CcTemplateDeleted(const CommandCost &result, TileIndex tile, uint32 p1, uint32 p2)
{
	if ( result.Succeeded() ) {
		TbtrGui* tbtrGui = static_cast<TbtrGui*>(FindWindowByClass(WC_TBTR_GUI));
		tbtrGui->UpdateGUI(TEMPLATE_DELETED);
	}
}

/**
 * Constructor, initialize GUI with a window descriptor
 */
TbtrGui::TbtrGui(WindowDesc* wdesc) : Window(wdesc)
{
	CreateNestedTree(wdesc);
	this->vscroll_engines = GetScrollbar(TRW_WIDGET_SCROLLBAR_ENGINES);
	this->vscroll_groups = GetScrollbar(TRW_WIDGET_SCROLLBAR_GROUPS);
	this->hscroll_templates = GetScrollbar(TRW_WIDGET_SCROLLBAR_TEMPLATES_HORIZ);
	this->vscroll_templates = GetScrollbar(TRW_WIDGET_SCROLLBAR_TEMPLATES);
	/* VEH_TRAIN should be 0; we want only 1 instance of this GUI to be present at the same time anyway, so
	 * this should be ok */
	FinishInitNested(VEH_TRAIN);

	this->vscroll_engines->SetStepSize(this->height_cell_engines);
	this->vscroll_groups->SetStepSize(this->height_cell_groups);
	this->vscroll_templates->SetStepSize(this->height_cell_templates);

	/* will be used to build the internal group and template lists
	 *
	 * NOTE: has to be set after FinishInitNested(...) because this function will set the owner back to
	 * INVALID_OWNER again */
	this->owner = _local_company;
	this->railtype = INVALID_RAILTYPE;

	this->engines.ForceRebuild();

	this->groups.ForceRebuild();
	this->groups.NeedResort();
	this->BuildGroupList();
	this->groups.Sort(&GroupNameSorter);

	BuildTemplateList();
}

// TODO comment
void TbtrGui::UpdateGUI(UpdateGuiMode mode)
{ 
	uint num_templates = this->templates.Length();
	this->BuildTemplateList();
	switch (mode) {
		case ENGINE_ADDED:
			/* if no template was selected, select the newly created chain */
			if ( this->index_selected_template == -1 )
				this->index_selected_template = this->templates.Length() - 1;
			break;
		case ENGINE_DELETED:
			/* last engine removed => unselect template */
			if ( this->templates.Length() < num_templates )
				this->index_selected_template = -1;
			break;
		case TEMPLATE_CLONED:
			this->ToggleWidgetLoweredState(TRW_WIDGET_TMPL_BUTTONS_CLONE);
			ResetObjectToPlace();
			this->SetDirty();
			break;
		case TEMPLATE_DELETED:
			this->index_selected_template = -1;
			break;
	}
	this->CalculateTemplatesHScroll();
}

/**
 * Build the list of engines that can be selected for new or existing templates
 */
void TbtrGui::BuildTemplateEngineList()
{
	if (!this->engines.NeedRebuild()) {
		return;
	}
	this->engines.Clear();
	const Engine* e;
	FOR_ALL_ENGINES_OF_TYPE(e, VEH_TRAIN) {
		if ( e->IsEnabled() )
			if ( HasBit(e->company_avail, this->owner) )
				*(this->engines).Append() = e->index;
	}
	this->engines.Compact();
	this->engines.RebuildDone();
	this->vscroll_engines->SetCount(this->engines.Length());
	this->engines.Sort(&TrainEnginesThenWagonsSorter);
}

/*
 * Update the list of groups to display for a given owner.
 * @param owner:  the owner of the groups to display, should the current company when the GUI is opened
 */
void TbtrGui::BuildGroupList()
{
	if (!this->groups.NeedRebuild()) {
		return;
	}

	this->groups.Clear();
	const Group *g;
	FOR_ALL_GROUPS(g) {
		if (g->owner == this->owner) {
			*(this->groups).Append() = g;
		}
	}

	this->groups.Compact();
	this->groups.Sort(&GroupNameSorter);
	this->groups.RebuildDone();
	this->index_selected_group = -1;
}

/*
 * Update the list of templates to display for a given owner and rail type.
 */
void TbtrGui::BuildTemplateList()
{
	this->templates.Clear();
	const TemplateVehicle *tv;

	FOR_ALL_TEMPLATES(tv) {
		if (tv->HasOwner(this->owner) && (tv->IsPrimaryVehicle() || tv->IsFreeWagonChain()) && tv->ContainsRailType(railtype))
			*(this->templates.Append()) = tv;
	}

	this->templates.RebuildDone();
	this->vscroll_templates->SetCount(this->templates.Length());
	this->CalculateTemplatesHScroll();
}

/**
 * Calculate and set the size of the template's horizontal scrollbar, based on the maximum length of all templates.
 */
void TbtrGui::CalculateTemplatesHScroll()
{
	this->hscroll_templates->SetCount(this->FindLongestTemplateDisplayWidth() + this->template_x_offset);
}

/*
 * Draw a widget of this GUI
 */
void TbtrGui::DrawWidget(const Rect& r, int widget) const
{
	switch(widget) {
		case TRW_WIDGET_MATRIX_GROUPS: {
			this->DrawGroups(r);
			break;
		}
		case TRW_WIDGET_MATRIX_TEMPLATES: {
			this->DrawTemplates(r);
			break;
		}
		case TRW_WIDGET_MATRIX_ENGINES: {
			this->DrawEngines(r);
			break;
		}
		case TRW_WIDGET_TMPL_INFO_PANEL: {
			DrawTemplateInfo(r);
			break;
		}
	}
}

/*
 * Draw all engines
 */
void TbtrGui::DrawEngines(const Rect& r) const
{
	uint y = r.top;
	int step_size = this->height_cell_engines;
	uint max = min(vscroll_engines->GetPosition() + vscroll_engines->GetCapacity(), this->engines.Length());
	for ( uint i = vscroll_engines->GetPosition(); i<max; ++i ) {

		/* Fill the background of the current cell in a darker tone for the currently selected engine */
		if ( this->index_selected_engine == (int)i ) {
			GfxFillRect(r.left, y, r.right, y+step_size, _colour_gradient[COLOUR_GREY][3]);
		}
		/* Draw the engine's image */
		EngineID eid = (this->engines)[i];
		const Engine* engine = Engine::Get(eid);
		DrawVehicleEngine(r.left+10, r.right, r.left, y+step_size/2, engine->index, GetEnginePalette(engine->index, this->owner), EIT_PURCHASE);

		/* Draw the engine's name
		 * Depending on the interface zoom level, the engine names are shifted
		 * to the right by 200, 120 or 60 pixels */
		uint offset_x = _gui_zoom==0 ? 200 : 120 / _gui_zoom;
		DrawString(r.left+offset_x, r.right, y+step_size/4, engine->info.string_id, TC_BLACK);

		y += step_size;
	}
}

/*
 * Draw all train groups
 */
void TbtrGui::DrawGroups(const Rect& r) const
{
	int left = r.left + WD_MATRIX_LEFT;
	int right = r.right - WD_MATRIX_RIGHT;
	int y = r.top;
	int max = min(this->vscroll_groups->GetPosition() + this->vscroll_groups->GetCapacity(), this->groups.Length());
	int step_size = this->height_cell_groups;
	int offset = step_size / 4;

	/* Then treat all groups defined by/for the current company */
	for ( int i=this->vscroll_groups->GetPosition(); i<max; ++i ) {
		const Group* group = (this->groups)[i];
		short group_id = group->index;

		/* Fill the background of the current cell in a darker tone for the currently selected group */
		if ( this->index_selected_group == i ) {
			GfxFillRect(left, y, right, y+step_size, _colour_gradient[COLOUR_GREY][3]);
		}

		/* Draw the group name */
		SetDParam(0, group_id);
		StringID str = STR_GROUP_NAME;
		DrawString(left+30, right, y+offset, str, TC_BLACK);

		/* Draw the index of the selected template for this group
		 * Note, that this is the index into the group list, not the template's ID. */
		if ( group->template_id != INVALID_TEMPLATE ) {
			SetDParam(0, FindTemplateIndexInGui(group->template_id));
			DrawString(left, right, y+offset, STR_TBTR_INFO_GROUP_USING_TEMPLATE, TC_BLACK, SA_HOR_CENTER);
		}

		/* Draw the number of trains that still need to be treated */
		int num_trains = CountTrainsToReplace(group);
		/* Draw text */
		TextColour color = TC_GREY;
		if ( num_trains ) color = TC_BLACK;
		DrawString(left, right-16, y+offset, STR_TBTR_INFO_TRAINS_NEED_REPLACEMENT, color, SA_RIGHT);
		/* Draw number */
		if ( num_trains ) color = TC_ORANGE;
		else color = TC_GREY;
		SetDParam(0, num_trains);
		DrawString(left, right-4, y+offset, STR_JUST_INT, color, SA_RIGHT);

		y += step_size;
	}
}

/**
 * Draw template info, like cost, length, etc.
 */
void TbtrGui::DrawTemplateInfo(const Rect &r) const
{
	if ( this->index_selected_template == -1 || (short)this->templates.Length() <= this->index_selected_template )
		return;

	const TemplateVehicle *tmp = this->templates[this->index_selected_template];

	/* Draw vehicle performance info */
	SetDParam(2, tmp->max_speed);
	SetDParam(1, tmp->power);
	SetDParam(0, tmp->weight);
	SetDParam(3, tmp->max_te);
	DrawString(r.left+8, r.right, r.top+4, STR_VEHICLE_INFO_WEIGHT_POWER_MAX_SPEED_MAX_TE);

	/* Draw cargo summary */
	short top = r.top + 24;
	short left = r.left + 8;
	short count_rows = 0;
	short max_rows = 2;

	CargoArray cargo_caps;
	for ( ; tmp; tmp=tmp->Next() )
		cargo_caps[tmp->cargo_type] += tmp->cargo_cap;
	int y = top;
	for (CargoID i = 0; i < NUM_CARGO; ++i) {
		if ( cargo_caps[i] > 0 ) {
			count_rows++;
			SetDParam(0, i);
			SetDParam(1, cargo_caps[i]);
			SetDParam(2, _settings_game.vehicle.freight_trains);
			DrawString(left, r.right, y, FreightWagonMult(i) > 1 ? STR_TBTR_INFO_CARGO_SUMMARY_MULTI : STR_TBTR_INFO_CARGO_SUMMARY, TC_WHITE, SA_LEFT);
			y += this->height_cell_templates;
			if ( count_rows % max_rows == 0 ) {
				y = top;
				left += 150;
			}
		}
	}
}

/**
 * Draw all templates in the GUI
 */
void TbtrGui::DrawTemplates(const Rect& r) const
{
	int left = r.left;
	int right = r.right;
	int y = r.top;
    int ypos_lo = y + this->height_cell_templates/2 + this->height_cell_templates/6;
	int ypos_hi = y+ScaleGUITrad(7);

	uint max = min(vscroll_templates->GetPosition() + vscroll_templates->GetCapacity(), this->templates.Length());
	TemplateVehicle* tv;
	for ( uint i = vscroll_templates->GetPosition(); i<max; ++i) {
		tv = TemplateVehicle::Get((this->templates)[i]->index);

		/* Fill the background of the current cell in a darker tone for the currently selected template */
		if ( this->index_selected_template == (int32)i ) {
			GfxFillRect(left, y, right, y+this->height_cell_templates, _colour_gradient[COLOUR_GREY][3]);
		}

		/* Draw a notification string for chains that are not runnable */
		if ( tv->IsFreeWagonChain() ) {
			DrawString(left, right-2, ypos_lo, STR_TBTR_WARN_FREE_WAGON, TC_RED, SA_RIGHT);
		}

		/* Draw the template's length in tile-units */
		SetDParam(0, tv->GetRealLength());
		SetDParam(1, 1);
		DrawString(left, right-4, ypos_hi, STR_TINY_BLACK_DECIMAL, TC_BLACK, SA_RIGHT);

		/* Draw the template */
		tv->Draw(left+this->template_x_offset, right, ypos_hi, hscroll_templates->GetPosition());

		/* Buying cost */
		SetDParam(0, tv->CalculateCost());
		DrawString(left+35, right, ypos_lo, STR_TBTR_INFO_TEMPLATE_VALUE_notinyfont, TC_BLUE, SA_LEFT);

		/* Index of current template vehicle in the list of all templates for its company */
		SetDParam(0, i);
		DrawString(left+5, left+25, ypos_hi, STR_BLACK_INT, TC_BLACK, SA_RIGHT);

		/* Draw whether the current template is in use by any group */
		int n_groups = tv->CountGroups();
		if ( n_groups > 0 ) {
			SetDParam(0, n_groups);
			uint _left = left + ScaleGUITrad(170);
			DrawString(_left, right, ypos_lo, STR_TBTR_INFO_TEMPLATE_IN_USE, TC_GREEN, SA_LEFT);
		}

		/* Draw information about template configuration settings */
		TextColour color;
		if ( tv->IsSetReuseDepotVehicles() ) color = TC_LIGHT_BLUE;
		else color = TC_GREY;
		DrawString(left+50+ScaleGUITrad(150), right, ypos_hi, STR_TBTR_CONFIG_USE_DEPOT, color, SA_LEFT);
		if ( tv->IsSetKeepRemainingVehicles() ) color = TC_LIGHT_BLUE;
		else color = TC_GREY;
		DrawString(left+60+ScaleGUITrad(215), right, ypos_hi, STR_TBTR_CONFIG_KEEP_REMAINDERS, color, SA_LEFT);
		if ( tv->IsSetRefitAsTemplate() ) color = TC_LIGHT_BLUE;
		else color = TC_GREY;
		DrawString(left+80+ScaleGUITrad(270), right, ypos_hi, STR_TBTR_CONFIG_USE_REFIT, color, SA_LEFT);

		y += this->height_cell_templates;
		ypos_lo += this->height_cell_templates;
		ypos_hi += this->height_cell_templates;
	}
}

/**
 * Find the longest template with respect to the combined sprite width of the whole chain
 */
uint TbtrGui::FindLongestTemplateDisplayWidth() const
{
	uint max_len = 0;
	for ( uint i=0; i<this->templates.Length(); ++i ) {
		TemplateVehicle* tv = TemplateVehicle::Get(this->templates[i]->index);
		uint len = tv->GetChainDisplayLength();
		if ( len > max_len )
			max_len = len;
	}
	return max_len;
}

/**
 * Return the index at which a given template is stored in the GUI
 *
 * @param tid: template id to look up
 * @return:    index in the gui, default -1
 */
int TbtrGui::FindTemplateIndexInGui(TemplateID tid) const
{
	for ( uint i=0; i<templates.Length(); ++i )
		if ( templates[i]->index == tid )
			return i;
	return -1;
}

/*
 * Handle mouse clicks on the GUI
 */
void TbtrGui::OnClick(Point pt, int widget, int click_count)
{
	switch (widget) {
		case TRW_WIDGET_TMPL_BUTTONS_CLONE: {
			this->SetWidgetDirty(TRW_WIDGET_TMPL_BUTTONS_CLONE);
			this->ToggleWidgetLoweredState(TRW_WIDGET_TMPL_BUTTONS_CLONE);

			if (this->IsWidgetLowered(TRW_WIDGET_TMPL_BUTTONS_CLONE)) {
				static const CursorID clone_icon =	SPR_CURSOR_CLONE_TRAIN;
				SetObjectToPlaceWnd(clone_icon, PAL_NONE, HT_VEHICLE, this);
			} else {
				ResetObjectToPlace();
			}
			break;
		}
		case TRW_WIDGET_TMPL_BUTTONS_DELETE: {
			TemplateID tid = this->templates[this->index_selected_template]->index;
			DoCommandP(0, tid, 0, CMD_DELETE_TEMPLATE, CcTemplateDeleted);
			BuildTemplateList();
			this->CalculateTemplatesHScroll();

			break;
		}
		case TRW_WIDGET_MATRIX_ENGINES: {
			uint16 index_new = (uint16)((pt.y - this->nested_array[TRW_WIDGET_MATRIX_ENGINES]->pos_y) / (this->height_cell_engines) ) + this->vscroll_engines->GetPosition();
			if ( index_new >= this->engines.Length() )
				this->index_selected_engine = -1;
			else if ( this->index_selected_engine == index_new )
				this->index_selected_engine = -1;
			else
				this->index_selected_engine = index_new;
			break;
		}
		case TRW_WIDGET_MATRIX_GROUPS: {
			uint16 index_new = (uint16)((pt.y - this->nested_array[TRW_WIDGET_MATRIX_GROUPS]->pos_y) / (this->height_cell_groups) ) + this->vscroll_groups->GetPosition();
			if ( index_new >= this->groups.Length() )
				this->index_selected_group = -1;
			else if ( this->index_selected_group == index_new )
				this->index_selected_group = -1;
			else
				this->index_selected_group = index_new;
			break;
		}
		case TRW_WIDGET_MATRIX_TEMPLATES: {
			uint16 index_new = (uint16)((pt.y - this->nested_array[TRW_WIDGET_MATRIX_TEMPLATES]->pos_y) / (this->height_cell_templates) ) + this->vscroll_templates->GetPosition();
			uint16 click_y_incell = (pt.y - nested_array[widget]->pos_y) % (this->height_cell_templates);

			if ( index_new >= this->templates.Length() ) {
				this->index_selected_template = -1;
				break;
			}

			if ( index_new == this->index_selected_template )
				this->index_selected_template = -1;

			else
				this->index_selected_template = index_new;

			int str_usedepot_left = nested_array[widget]->pos_x + 50 + ScaleGUITrad(150);
			int str_keeprem_left = nested_array[widget]->pos_x + 60 + ScaleGUITrad(215);
			int str_userefit_left = nested_array[widget]->pos_x + 80 + ScaleGUITrad(270);
			int str_pos_hi = ScaleGUITrad(7);
			Dimension str_usedepot_bb = GetStringBoundingBox(STR_TBTR_CONFIG_USE_DEPOT);
			Dimension str_keeprem_bb = GetStringBoundingBox(STR_TBTR_CONFIG_KEEP_REMAINDERS);
			Dimension str_userefit_bb = GetStringBoundingBox(STR_TBTR_CONFIG_USE_REFIT);
			uint str_height = str_usedepot_bb.height;       // string height is assumed to be the same for all config option strings

			/* clicked on one of the template config option strings select the template and toggle the config
			* option */
			if ( click_y_incell >= str_pos_hi && click_y_incell <= str_pos_hi + str_height ) {
				if ( pt.x >= str_usedepot_left && pt.x <= str_usedepot_left + (int)str_usedepot_bb.width ) {
					this->index_selected_template = index_new;
                    TemplateID template_index = ((this->templates)[index_new])->index;
                    DoCommandP(0, template_index, TBTR_OPT_REUSE_DEPOT_VEHICLES, CMD_TOGGLE_TEMPLATE_OPTION);
				}
				else if ( pt.x >= str_keeprem_left && pt.x <= str_keeprem_left + (int)str_keeprem_bb.width ) {
					this->index_selected_template = index_new;
                    TemplateID template_index = ((this->templates)[index_new])->index;
                    DoCommandP(0, template_index, TBTR_OPT_KEEP_REMAINDERS, CMD_TOGGLE_TEMPLATE_OPTION);
				}
				else if ( pt.x >= str_userefit_left && pt.x <= str_userefit_left + (int)str_userefit_bb.width ) {
					this->index_selected_template = index_new;
                    TemplateID template_index = ((this->templates)[index_new])->index;
                    DoCommandP(0, template_index, TBTR_OPT_REFIT_VEHICLE, CMD_TOGGLE_TEMPLATE_OPTION);
				}
			}

			break;
		}
		case TRW_WIDGET_START: {
			if ( this->index_selected_group>=0 && this->index_selected_template>=0 ) {
				const TemplateVehicle* tv = *(this->templates.Get(this->index_selected_template));
				const Group* g = *(this->groups.Get(this->index_selected_group));
				DoCommandP(0, g->index | (1 << 16), tv->index, CMD_START_STOP_TBTR);
			}
			break;
		}
		case TRW_WIDGET_STOP: {
			if ( this->index_selected_group>=0 ) {
				DoCommandP(0, this->index_selected_group, 0, CMD_START_STOP_TBTR);
			}
			break;
		}
		case TRW_WIDGET_TMPL_BUTTONS_ADD: {
			/* get the selected engine */
			if ( index_selected_engine == -1 )
				return;

			/* selected engine */
			EngineID eid = this->engines[index_selected_engine];

			/* selected template */
			TemplateID tid = INVALID_TEMPLATE;
			if ( index_selected_template >= 0 )
				tid = this->templates[index_selected_template]->index;

			/* add the engine */
			DoCommandP(0, tid, eid, CMD_TEMPLATE_ADD_ENGINE, CcTemplateEngineAdded);
			break;
		}
		case TRW_WIDGET_TMPL_BUTTONS_DELETE_LAST_VEH: {
			/* get the currently selected template */
			TemplateID tid = INVALID_TEMPLATE;
			if ( index_selected_template >= 0 )
				tid = this->templates[index_selected_template]->index;
			else
				return;

			/* delete the last engine */
			DoCommandP(0, tid, 0, CMD_TEMPLATE_DELETE_ENGINE, CcTemplateEngineDeleted);
			break;
		}
	}
	this->SetDirty();
}

/**
 * Extra actions when the window needs to be redrawn
 */
void TbtrGui::OnInvalidateData(int data = 0, bool gui_scope = true)
{
	this->groups.ForceRebuild();
	this->templates.ForceRebuild();
}

/*
 * Draw this GUI
 */
void TbtrGui::OnPaint()
{
	if ( _gui_zoom != this->gui_zoom ) {
		this->gui_zoom = _gui_zoom;
		this->UpdateZoom();
	}
	this->BuildTemplateEngineList();
	this->BuildGroupList();
	this->BuildTemplateList();
	this->DrawWidgets();
}

/*
 * Update GUI components on resize
 */
void TbtrGui::OnResize()
{
    this->height_cell_engines = this->resize.step_height;
    this->height_cell_groups = this->resize.step_height;
    this->height_cell_templates = this->resize.step_height * 2;

	/* Top Matrix */
	NWidgetCore* nwi = this->GetWidget<NWidgetCore>(TRW_WIDGET_MATRIX_GROUPS);
	this->vscroll_groups->SetCapacityFromWidget(this, TRW_WIDGET_MATRIX_GROUPS);
	nwi->widget_data = (this->vscroll_groups->GetCapacity() << MAT_ROW_START) + (1 << MAT_COL_START);

	/* Bottom Matrix */
	NWidgetCore* nwi2 = this->GetWidget<NWidgetCore>(TRW_WIDGET_MATRIX_TEMPLATES);
	this->vscroll_templates->SetCapacityFromWidget(this, TRW_WIDGET_MATRIX_TEMPLATES);
	this->hscroll_templates->SetCapacityFromWidget(this, TRW_WIDGET_MATRIX_TEMPLATES);
	nwi2->widget_data = (this->vscroll_templates->GetCapacity() << MAT_ROW_START) + (1 << MAT_COL_START);

	/* Engines List */
	NWidgetCore* nwi3 = this->GetWidget<NWidgetCore>(TRW_WIDGET_MATRIX_ENGINES);
	this->vscroll_engines->SetCapacityFromWidget(this, TRW_WIDGET_MATRIX_ENGINES);
	nwi3->widget_data = (this->vscroll_engines->GetCapacity() << MAT_ROW_START) + (1 << MAT_COL_START);
}

/*
 * Handle the selection when a train in the game world has been clicked,
 * This is used for cloning a train into a template vehicle chain.
 *
 * @param train:  pointer to the train that was clicked on, assumes that this is the first vehicle
 *                of the train
 */
bool TbtrGui::OnVehicleSelect(const Vehicle* v)
{
	if (v->type != VEH_TRAIN)
		return false;
	DoCommandP(0, v->index, 0, CMD_CLONE_TEMPLATE_FROM_TRAIN);
	return true;
}

/**
 * Update TBTR things according to the current UI zoom level
 */
void TbtrGui::UpdateZoom()
{
	this->gui_zoom = _gui_zoom;
	TemplateVehicle* tv;
	FOR_ALL_TEMPLATES(tv) {
		tv->UpdateZoom();
	}
}

/*
 * Recalculate the size of the window's components
 */
void TbtrGui::UpdateWidgetSize(int widget, Dimension *size, const Dimension &padding, Dimension *fill, Dimension *resize)
{
	switch (widget) {
		case TRW_WIDGET_MATRIX_TEMPLATES:
			resize->height = GetEngineListHeight(VEH_TRAIN) * 2;
			size->height = 3 * resize->height;
			break;
		case TRW_WIDGET_MATRIX_GROUPS:
		case TRW_WIDGET_MATRIX_ENGINES:
			resize->height = GetEngineListHeight(VEH_TRAIN);
			size->height = 3 * resize->height;
			break;
	}
	this->CalculateTemplatesHScroll();
}

/*
 * Show the TBTR Gui
 */
void ShowTbtrGui()
{
	new TbtrGui(&_tbtr_gui_desc);
}

/**
 * Count the number of trains (chains) that need to be treated for a given group
 *
 * @param group:	the group for which we want the count
 * @return:			int, number of chains to be treated, i.e. not the invidual vehicles
 */
int CountTrainsToReplace(const Group* group)
{
	int count = 0;
	if ( group->template_id == INVALID_TEMPLATE )
		return count;
	TemplateVehicle* tv = TemplateVehicle::Get(group->template_id);
	Train* t;
	FOR_ALL_TRAINS(t) {
		if ( t->IsPrimaryVehicle() && t->group_id == group->index && tv && tv->TrainNeedsReplacement(t) )
			++count;
	}
	return count;
}
