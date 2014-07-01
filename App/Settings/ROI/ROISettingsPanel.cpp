/*
    
    Copyright (C) 2014  Jorge Peña Pastor <jpena@cesvima.upm.es>

    This file is part of ESPINA.

    ESPINA is free software: you can redistribute it and/or modify
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
#include "ROISettingsPanel.h"
#include "ROISettings.h"
#include <Support/Settings/EspinaSettings.h>
#include <GUI/Model/ModelAdapter.h>
#include <GUI/Model/CategoryAdapter.h>
#include <Support/ViewManager.h>

// VTK
#include <vtkMath.h>

// Qt
#include <QMessageBox>
#include <QSettings>
#include <QDebug>

using namespace EspINA;

const QString FIT_TO_SLICES ("ViewManager::FitToSlices");

// declared in Core library.
extern const QString Category::X_DIM;
extern const QString Category::Y_DIM;
extern const QString Category::Z_DIM;

//------------------------------------------------------------------------
ROISettingsPanel::ROISettingsPanel(ModelAdapterSPtr model,
                                   ROISettings *settings,
                                   ViewManagerSPtr viewManager)
: m_model           {model}
, m_settings        {settings}
, m_activeCategory  {nullptr}
, m_viewManager     {viewManager}
, m_zValueChanged   {false}
, m_zTaxValueChanged{false}
{
  setupUi(this);
  m_zSpacing = 1.0;

  m_xSize->setValue(m_settings->xSize());
  m_ySize->setValue(m_settings->ySize());

  QSettings espinaSettings(CESVIMA, ESPINA);

  if (espinaSettings.value(FIT_TO_SLICES).toBool())
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

  m_zSize->setValue(vtkMath::Round(m_settings->zSize()/m_zSpacing));

  m_categorySelector->setModel(m_model.get());

  // disable category selector if there isn't a category to choose.
  if (m_model->classification() == nullptr)
  {
    m_categorySelectorGroup->setEnabled(false);
  }

  connect(m_categorySelector, SIGNAL(activated(QModelIndex)),
          this, SLOT(updateCategoryROI(QModelIndex)));

  m_categorySelector->setRootModelIndex(m_model->classificationRoot());

  // the rounding of fit to slices on the z value was making the dialog ask the
  // user if he wanted to save the changes even when the user hasn't changed
  // anything. this fixes it.
  connect(m_zTaxSize, SIGNAL(valueChanged(int)), this, SLOT(zValueChanged(int)));
  connect(m_zSize, SIGNAL(valueChanged(int)), this, SLOT(zValueChanged(int)));
}

//------------------------------------------------------------------------
ROISettingsPanel::~ROISettingsPanel()
{
//   qDebug() << "Destroy Settings Panel";
}

//------------------------------------------------------------------------
void ROISettingsPanel::acceptChanges()
{
  m_settings->setXSize(m_xSize->value());
  m_settings->setYSize(m_ySize->value());
  m_settings->setZSize(m_zSize->value()*m_zSpacing);

  writeCategoryProperties();
}

//------------------------------------------------------------------------
void ROISettingsPanel::rejectChanges()
{
}

//------------------------------------------------------------------------
bool ROISettingsPanel::modified() const
{
  bool returnValue = false;
  returnValue |= (m_xSize->value() != m_settings->xSize());
  returnValue |= (m_ySize->value() != m_settings->ySize());
  returnValue |= m_zValueChanged;
  returnValue |= categoryROIModified();

  return returnValue;
}

//------------------------------------------------------------------------
SettingsPanelPtr ROISettingsPanel::clone()
{
  return SettingsPanelPtr(new ROISettingsPanel(m_model, m_settings, m_viewManager));
}

//------------------------------------------------------------------------
bool ROISettingsPanel::categoryROIModified() const
{
  bool modified = false;

  if (m_activeCategory)
  {
    modified |= (m_activeCategory->property(Category::X_DIM).toInt() != m_xTaxSize->value());
    modified |= (m_activeCategory->property(Category::Y_DIM).toInt() != m_yTaxSize->value());
    modified |= m_zTaxValueChanged;
  }

  return modified;
}

//------------------------------------------------------------------------
void ROISettingsPanel::writeCategoryProperties()
{
  if (m_activeCategory)
  {
    m_activeCategory->addProperty(Category::X_DIM, m_xTaxSize->value());
    m_activeCategory->addProperty(Category::Y_DIM, m_yTaxSize->value());
    m_activeCategory->addProperty(Category::Z_DIM, vtkMath::Round(m_zTaxSize->value()*m_zSpacing));
  }
}

//------------------------------------------------------------------------
void ROISettingsPanel::updateCategoryROI(const QModelIndex& index)
{
  if (!index.isValid())
    return;

  ItemAdapterPtr itemPtr = itemAdapter(index);
  if (ItemAdapter::Type::CATEGORY != itemPtr->type())
    return;

  CategoryAdapterPtr elem = categoryPtr(itemPtr);
  if (m_activeCategory && m_activeCategory.get() != elem)
  {
    // Check for changes
    if (categoryROIModified())
    {
      QMessageBox msg;
      msg.setWindowTitle(tr("EspINA"));
      msg.setText(tr("The properties of the category \"%1\" have been modified.\nDo you want to save the changes?").arg(m_activeCategory->data().toString()));
      msg.setStandardButtons(QMessageBox::Yes|QMessageBox::No);
      if (msg.exec() == QMessageBox::Yes)
        writeCategoryProperties();
    }
  }

  m_activeCategory = m_model->smartPointer(elem);
  m_zTaxValueChanged = false;

  // 2013-03-19 Fix missing taxonomy properties in some cases. By default revert to "default VOI" values.
  if (!m_activeCategory->properties().contains(Category::X_DIM) ||
      !m_activeCategory->properties().contains(Category::Y_DIM) ||
      !m_activeCategory->properties().contains(Category::Z_DIM))
  {
    m_activeCategory->addProperty(Category::X_DIM, QVariant(m_settings->xSize()));
    m_activeCategory->addProperty(Category::Y_DIM, QVariant(m_settings->ySize()));
    m_activeCategory->addProperty(Category::Z_DIM, QVariant(m_settings->zSize()));
  }

  QVariant xSize = elem->property(Category::X_DIM);
  QVariant ySize = elem->property(Category::Y_DIM);
  QVariant zSize = elem->property(Category::Z_DIM);

  if (!xSize.isValid() || !ySize.isValid() || !zSize.isValid())
  {
    xSize = m_xSize->value();
    ySize = m_ySize->value();
    zSize = m_zSize->value()*m_zSpacing;
  }

  m_xTaxSize->setValue(xSize.toInt());
  m_yTaxSize->setValue(ySize.toInt());
  m_zTaxSize->blockSignals(true);
  m_zTaxSize->setValue(vtkMath::Round(zSize.toInt()/m_zSpacing));
  m_zTaxSize->blockSignals(false);
}

//------------------------------------------------------------------------
void ROISettingsPanel::zValueChanged(int unused)
{
  if (sender() == m_zSize)
    m_zValueChanged = true;

  if (sender() == m_zTaxSize)
    m_zTaxValueChanged = true;
}
