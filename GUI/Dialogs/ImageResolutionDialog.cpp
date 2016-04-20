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

// ESPINA
#include "ImageResolutionDialog.h"

// Qt
#include <QDialog>
#include <QSpinBox>
#include <QDebug>

ESPINA::ImageResolutionDialog::ImageResolutionDialog(QWidget *parent,
		int width, int height)
{
	setupUi(this);

	m_width = width;
	m_height = height;
	m_ratio = float(width)/height;
	width_spinBox->setValue(width);
	height_spinBox->setValue(height);
	connectSignals();
}

int ESPINA::ImageResolutionDialog::getMagnifcation()
{
	return 4096.0/m_width+0.5;
}

void ESPINA::ImageResolutionDialog::heightChanged(int value)
{
	disconnectSignals();
	m_height = value; //height_spinBox->value();
	updateWidth();
	connectSignals();
}

void ESPINA::ImageResolutionDialog::widthChanged(int value)
{
	disconnectSignals();
	m_width = value; //weight_spinBox->value();
	updateHeight();
	connectSignals();
}

void ESPINA::ImageResolutionDialog::updateHeight()
{
	m_height = m_width / m_ratio;
	height_spinBox->setValue(m_height);
}

void ESPINA::ImageResolutionDialog::updateWidth()
{
	m_width = m_height * m_ratio;
	width_spinBox->setValue(m_width);
}

void ESPINA::ImageResolutionDialog::connectSignals()
{
	connect(width_spinBox, SIGNAL(valueChanged(int)), this,
			SLOT(widthChanged(int)));
	connect(height_spinBox, SIGNAL(valueChanged(int)), this,
			SLOT(heightChanged(int)));
}

void ESPINA::ImageResolutionDialog::disconnectSignals()
{
	disconnect(width_spinBox, SIGNAL(valueChanged(int)), this,
			SLOT(widthChanged(int)));
	disconnect(height_spinBox, SIGNAL(valueChanged(int)), this,
			SLOT(heightChanged(int)));
}
