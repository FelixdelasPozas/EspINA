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

// EspINA
#include "SettingsPanel.h"
#include <Toolbars/VOI/Settings.h>
#include <Core/EspinaSettings.h>
#include <Core/Model/EspinaModel.h>
#include <Core/Model/Taxonomy.h>
#include <GUI/ViewManager.h>

// VTK
#include <vtkMath.h>

// Qt
#include <QMessageBox>
#include <QSettings>

using namespace EspINA;

const QString FIT_TO_SLICES ("ViewManager::FitToSlices");

//------------------------------------------------------------------------
RectangularVOI::SettingsPanel::SettingsPanel(EspinaModelPtr model,
                                             RectangularVOI::Settings *settings,
                                             ViewManager *viewManager)
: m_model(model)
, m_settings(settings)
, m_activeTaxonomy(NULL)
, m_viewManager(viewManager)
{
  setupUi(this);
  m_zSpacing = 1.0;

  m_xSize->setValue(m_settings->xSize());
  m_ySize->setValue(m_settings->ySize());

  QSettings espinaSettings(CESVIMA, ESPINA);

  if (espinaSettings.value(FIT_TO_SLICES).toBool())
  {
    if (m_viewManager->viewResolution() != NULL)
    {
      m_zSpacing = m_viewManager->viewResolution()[2];
      m_zSize->setSuffix(" slices");
      m_zTaxSize->setSuffix(" slices");
    }
    else
    {
      m_zSize->setSuffix(" nm");
      m_zTaxSize->setSuffix(" nm");
    }
  }

  m_zSize->setValue(vtkMath::Round(m_settings->zSize()/m_zSpacing));

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
  m_settings->setZSize(m_zSize->value()*m_zSpacing);

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
      || m_zSize->value()*m_zSpacing != m_settings->zSize()
      || taxonomyVOIModified();
}

//------------------------------------------------------------------------
ISettingsPanelPtr RectangularVOI::SettingsPanel::clone()
{
  return ISettingsPanelPtr(new SettingsPanel(m_model, m_settings, m_viewManager));
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
    modified = modified || zOldSize.toInt() != m_zTaxSize->value()*m_zSpacing;
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
    m_activeTaxonomy->addProperty(TaxonomyElement::Z_DIM, m_zTaxSize->value()*m_zSpacing);
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
    zSize = m_zSize->value()*m_zSpacing;
  }

  m_xTaxSize->setValue(xSize.toInt());
  m_yTaxSize->setValue(ySize.toInt());
  m_zTaxSize->setValue(vtkMath::Round(zSize.toInt()/m_zSpacing));
}
