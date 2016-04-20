/*
 * Copyright (C) 2016, Rafael Juan Vicente Garcia <rafaelj.vicente@gmail.com>
 *
 * This file is part of ESPINA.
 *
 * ESPINA is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef ESPINA_IMAGE_SIZE_DIALOG_H
#define ESPINA_IMAGE_SIZE_DIALOG_H

#include "GUI/EspinaGUI_Export.h"

// ESPINA
#include "ui_ImageResolutionDialog.h"

// Qt
#include <QDialog>

namespace ESPINA
{

class EspinaGUI_EXPORT ImageResolutionDialog: public QDialog,
		private Ui::ImageResolutionDialog
{
Q_OBJECT

public:
	ImageResolutionDialog(QWidget *parent, int width, int height);
	virtual ~ImageResolutionDialog(){};
	int getMagnifcation();

private slots:
	void heightChanged(int value);
	void widthChanged(int value);

private:
	double m_ratio;
	int m_height, m_width;
	void updateHeight();
	void updateWidth();
	void connectSignals();
	void disconnectSignals();
};
}

#endif // ESPINA_CATEGORY_SELECTOR_DIALOG_H
