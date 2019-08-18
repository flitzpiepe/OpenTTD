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

class TemplateRefitWindow : public Window {
public:
	TemplateRefitWindow(WindowDesc*);
private:
	virtual void OnResize();
	virtual void UpdateWidgetSize(int, Dimension*, const Dimension&, Dimension*, Dimension*);

	uint16 num_cargo_types = 11; // TODO
};

void ShowTemplateRefitWindow();

#endif /* !TBTR_TEMPLATE_REFIT_WINDOW_H */
