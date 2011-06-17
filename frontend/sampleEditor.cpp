/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) <year>  <name of author>

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

*/

#include "sampleEditor.h"

#include "espina.h"
#include "products.h"

#include <QDebug>

SampleEditor::SampleEditor(QWidget* parent, Qt::WindowFlags f)
        : QDialog(parent, f)
{
    setupUi(this);

    connect(m_unit,SIGNAL(currentIndexChanged(int)),this,SLOT(unitChanged(int)));
    connect(pushButton, SIGNAL(clicked(bool)), this, SLOT(updateSpacing()));
   /* connect(EspINA::instance(), SIGNAL(segmentationCreated(Segmentation*)), 
            this, SLOT(close()));*/
}

void SampleEditor::setSample(Sample* sample)
{
    m_sample = sample;
    double spacing[3];

    sample->spacing(spacing);

    m_xSize->setValue(spacing[0]);
    m_ySize->setValue(spacing[1]);
    m_zSize->setValue(spacing[2]);

    // If the Sample has segmentations the spacing cannot change
    if ( !sample->segmentations().empty() && pushButton->isEnabled())
    {
        pushButton->setDisabled(true);
        pushButton->setToolTip("The spacing could not be changed if the sample has segmentations");
    }
}

void SampleEditor::spacing(double value[3])
{
  value[0] = m_xSize->cleanText().toFloat();
  value[1] = m_ySize->cleanText().toFloat();
  value[2] = m_zSize->cleanText().toFloat();
}



void SampleEditor::unitChanged(int unitIndex)
{
    m_xSize->setSuffix(m_unit->currentText());
    m_ySize->setSuffix(m_unit->currentText());
    m_zSize->setSuffix(m_unit->currentText());
}

void SampleEditor::updateSpacing()
{
  setResult(QDialog::Accepted);
  accept();
}



