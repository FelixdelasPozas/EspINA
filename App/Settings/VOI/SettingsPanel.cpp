/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) 2012  <copyright holder> <email>

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


#include "SettingsPanel.h"
#include <Toolbars/VOI/Settings.h>
#include <Core/Model/EspinaModel.h>
#include <Core/Model/Taxonomy.h>

#include <QMessageBox>

using namespace EspINA;

//------------------------------------------------------------------------
RectangularVOI::SettingsPanel::SettingsPanel(EspinaModelPtr model,
                                             RectangularVOI::Settings *settings)
: m_model(model)
, m_settings(settings)
, m_activeTaxonomy(NULL)
{
  setupUi(this);

  m_xSize->setValue(m_settings->xSize());
  m_ySize->setValue(m_settings->ySize());
  m_zSize->setValue(m_settings->zSize());

  m_taxonomySelector->setModel(m_model);

  connect(m_taxonomySelector, SIGNAL(activated(QModelIndex)),
          this, SLOT(updateTaxonomyVOI(QModelIndex)));

  m_taxonomySelector->setRootModelIndex(m_model->taxonomyRoot());
}

//------------------------------------------------------------------------
RectangularVOI::SettingsPanel::~SettingsPanel()
{
//   qDebug() << "Destroy Settings Panel";
}

//------------------------------------------------------------------------
void RectangularVOI::SettingsPanel::acceptChanges()
{
  m_settings->setXSize(m_xSize->value());
  m_settings->setYSize(m_ySize->value());
  m_settings->setZSize(m_zSize->value());

  writeTaxonomyProperties();
}

//------------------------------------------------------------------------
void RectangularVOI::SettingsPanel::rejectChanges()
{

}

//------------------------------------------------------------------------
bool RectangularVOI::SettingsPanel::modified() const
{
  return m_xSize->value() != m_settings->xSize()
      || m_ySize->value() != m_settings->ySize()
      || m_zSize->value() != m_settings->zSize()
      || taxonomyVOIModified();
}

//------------------------------------------------------------------------
ISettingsPanelPtr RectangularVOI::SettingsPanel::clone()
{
  return ISettingsPanelPtr(new SettingsPanel(m_model, m_settings));
}

//------------------------------------------------------------------------
bool RectangularVOI::SettingsPanel::taxonomyVOIModified() const
{
  bool modified = false;

  if (m_activeTaxonomy)
  {
    QVariant xOldSize = m_activeTaxonomy->property(TaxonomyElement::X_DIM);
    QVariant yOldSize = m_activeTaxonomy->property(TaxonomyElement::Y_DIM);
    QVariant zOldSize = m_activeTaxonomy->property(TaxonomyElement::Z_DIM);

    modified = modified || xOldSize.toInt() != m_xTaxSize->value();
    modified = modified || yOldSize.toInt() != m_yTaxSize->value();
    modified = modified || zOldSize.toInt() != m_zTaxSize->value();
  }

  return modified;
}

//------------------------------------------------------------------------
void RectangularVOI::SettingsPanel::writeTaxonomyProperties()
{
  if (m_activeTaxonomy)
  {
    m_activeTaxonomy->addProperty(TaxonomyElement::X_DIM, m_xTaxSize->value());
    m_activeTaxonomy->addProperty(TaxonomyElement::Y_DIM, m_yTaxSize->value());
    m_activeTaxonomy->addProperty(TaxonomyElement::Z_DIM, m_zTaxSize->value());
  }
}

//------------------------------------------------------------------------
void RectangularVOI::SettingsPanel::updateTaxonomyVOI(const QModelIndex& index)
{
  if (!index.isValid())
    return;

  ModelItemPtr item = indexPtr(index);
  if (EspINA::TAXONOMY != item->type())
    return;

  TaxonomyElementPtr elem = taxonomyElementPtr(item);
  if (m_activeTaxonomy && m_activeTaxonomy != elem)
  {
    // Check for changes
    if (taxonomyVOIModified())
    {
      QMessageBox msg;
      msg.setText(tr("Taxonomy properties have changed. Do you want to save them"));
      msg.setStandardButtons(QMessageBox::Yes|QMessageBox::No);
      if (msg.exec() == QMessageBox::Yes)
        writeTaxonomyProperties();
    }
  }

  m_activeTaxonomy = elem;

  QVariant xSize = elem->property(TaxonomyElement::X_DIM);
  QVariant ySize = elem->property(TaxonomyElement::Y_DIM);
  QVariant zSize = elem->property(TaxonomyElement::Z_DIM);

  if (!xSize.isValid() || !ySize.isValid() || !zSize.isValid())
  {
    xSize = m_xSize->value();
    ySize = m_ySize->value();
    zSize = m_zSize->value();
  }

  m_xTaxSize->setValue(xSize.toInt());
  m_yTaxSize->setValue(ySize.toInt());
  m_zTaxSize->setValue(zSize.toInt());
}
