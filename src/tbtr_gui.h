/* $Id$ */

/*
 * This file is part of OpenTTD.
 * OpenTTD is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 2.
 * OpenTTD is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details. You should have received a copy of the GNU General Public License along with OpenTTD. If not, see <http://www.gnu.org/licenses/>.
 */

/** @file tbtr_template_gui_main.h Main window for Template-based train replacement patch. */

#ifndef TBTR_GUI_H
#define TBTR_GUI_H

#include "stdafx.h"
#include "strings_func.h"
#include "window_gui.h"
#include "vehicle_gui_base.h"
#include "engine_gui.h"

#include "group.h"
#include "company_func.h"
#include "tilehighlight_func.h"

#include "tbtr_template_vehicle.h"

typedef GUIList<const Group*> GUIGroupList;
typedef GUIList<const TemplateVehicle*> GUITemplateList;

enum UpdateGuiMode {
	ENGINE_ADDED,
	ENGINE_DELETED,
	TEMPLATE_CLONED,
	TEMPLATE_DELETED,
};

class TemplateRefitWindow;

/*
 * TBTR's main window - for managing templates and setting up train groups for replacement.
 */
class TbtrGui : public Window {
friend TemplateRefitWindow;
public:
	TbtrGui(WindowDesc*);
	virtual void UpdateWidgetSize(int, Dimension*, const Dimension&, Dimension*, Dimension*);
	virtual void DrawWidget(const Rect&, int) const;
	virtual void OnClick(Point, int, int);
	virtual void OnPaint();
	virtual void OnResize();
	virtual bool OnVehicleSelect(const Vehicle*);
	virtual void OnInvalidateData(int, bool);
	void UpdateGUI(UpdateGuiMode);

private:
	void BuildGroupList();
	void BuildTemplateList();
	void BuildTemplateEngineList();
	void CalculateTemplatesHScroll();
	TemplateID CheckClickedTemplatePart(Point&, uint16) const;
	void DrawEngines(const Rect&) const;
	void DrawGroups(const Rect&) const;
	void DrawTemplateInfo(const Rect&) const;
	void DrawTemplates(const Rect&) const;
	uint FindLongestTemplateDisplayWidth() const;
	int FindTemplateIndexInGui(TemplateID) const;
	int FindNewestTemplateInGui() const;
	bool HandleClickGroupList(Point, int, uint16);

	void UpdateButtonState();
	void UpdateRefitWindow();
	void UpdateZoom();

	Scrollbar* vscroll_engines;                                   ///< Scrollbar for the engines list
	Scrollbar* vscroll_groups;                                    ///< Scrollbar for the group list
	Scrollbar* hscroll_templates;                                 ///< Horizontal scrollbar for the template list
	Scrollbar* vscroll_templates;                                 ///< Vertical scrollbar for the template list

	uint16 height_cell_engines;                                   ///< the height of a line in the engines matrix
	uint16 height_cell_groups;                                    ///< the height of a line in the groups matrix
	uint16 height_cell_templates;                                 ///< the height of a line in the templates matrix
	uint16 pos_string_usedepot;                                   ///< horizontal position of the use-depot option
	uint16 pos_string_keepremainders;                             ///< horizontal position of the keep-remainders option
	uint16 pos_string_userefit;                                   ///< horizontal position of the use-refit option
	uint16 pos_string_lo;                                         ///< low vertical string position in a GUI cell
	uint16 pos_string_med;                                        ///< medium vertical string position in a GUI cell
	uint16 pos_string_hi;                                         ///< high vertical string position in a GUI cell

	uint16 template_x_offset = 50;                                ///< LHS spacing for templates in the GUI
	int index_selected_engine = -1;                               ///< index into the GUIEngineList "engines"
	int index_selected_group = -1;                                ///< index into the GUIGroupList "groups"
	int index_selected_template = -1;                             ///< index into the GUITemplateList "templates"
	TemplateID id_selected_template_part = INVALID_TEMPLATE;      ///< ID of the selected engine of the currently selected template
	const TemplateVehicle* new_chain_head = NULL;                 ///< A pointer to remember what the new chain head will be after deleting the leading vehicle
	GUIEngineList engines;                                        ///< List of new engines to add to the templates
	GUIGroupList groups;                                          ///< List of groups
	GUITemplateList templates;                                    ///< List of templates
	RailType railtype;
	ZoomLevelByte gui_zoom;                                       ///< GUI Zoom level
};

void ShowTbtrGui();

int CountTrainsToReplace(const Group*);

#endif /* !TBTR_GUI_H */
