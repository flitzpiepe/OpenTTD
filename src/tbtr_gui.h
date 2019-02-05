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

/*
 * TBTR's main window - for managing templates and setting up train groups for replacement.
 */
class TbtrGui : public Window {
public:
	TbtrGui(WindowDesc*);
	virtual void UpdateWidgetSize(int, Dimension*, const Dimension&, Dimension*, Dimension*);
	virtual void DrawWidget(const Rect&, int) const;
	virtual void OnClick(Point, int, int);
	virtual void OnPaint();
	virtual void OnResize();
	virtual bool OnVehicleSelect(const Vehicle*);
	virtual void OnInvalidateData(int, bool);
	void DeselectTemplate() {this->index_selected_template=-1;}

private:
	void BuildGroupList();
	void BuildTemplateList();
	void BuildTemplateEngineList();
	void CalculateTemplatesHScroll();
	void DrawEngines(const Rect&) const;
	void DrawGroups(const Rect&) const;
	void DrawTemplateInfo(const Rect&) const;
	void DrawTemplates(const Rect&) const;
	uint FindLongestTemplateDisplayWidth() const;
	int FindTemplateIndexInGui(TemplateID) const;

	Scrollbar* vscroll_engines;         ///< Scrollbar for the engines list
	Scrollbar* vscroll_groups;          ///< Scrollbar for the group list
	Scrollbar* hscroll_templates;       ///< Horizontal scrollbar for the template list
	Scrollbar* vscroll_templates;       ///< Vertical scrollbar for the template list
	uint16 line_height = 26;            ///< step size for the matrix widgets
	uint16 template_x_offset = 50;      ///< LHS spacing for templates in the GUI
	int index_selected_engine = -1;
	int index_selected_group = -1;
	int index_selected_template = -1;
	GUIEngineList engines;              ///< List of new engines to add to the templates
	GUIGroupList groups;                ///< List of groups
	GUITemplateList templates;          ///< List of templates
	RailType railtype;
};

void ShowTbtrGui();

int CountTrainsToReplace(const Group*);

#endif /* !TBTR_GUI_H */
