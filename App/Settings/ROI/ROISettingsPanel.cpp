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

//------------------------------------------------------------------------
ROISettingsPanel::ROISettingsPanel(ModelAdapterSPtr model,
                                   ROISettings *settings,
                                   ViewManagerSPtr viewManager)
: m_model           {model}
, m_settings        {settings}
, m_activeCategory  {nullptr}
, m_viewManager     {viewManager}
{
  setupUi(this);

  m_xSize->setValue(m_settings->xSize());
  m_ySize->setValue(m_settings->ySize());
  m_zSize->setValue(m_settings->zSize());

  QSettings espinaSettings(CESVIMA, ESPINA);

  if (espinaSettings.value(FIT_TO_SLICES).toBool())
  {
    m_zSize->setSuffix(" slices");
    m_zCategorySize->setSuffix(" slices");
  }
  else
  {
    m_zSize->setSuffix(" nm");
    m_zCategorySize->setSuffix(" nm");
  }

  m_categorySelector->setModel(m_model.get());

  // disable category selector if there isn't a category to choose.
  if (m_model->classification() == nullptr)
  {
    m_categorySelectorGroup->setEnabled(false);
  }

  connect(m_categorySelector, SIGNAL(activated(QModelIndex)),
          this, SLOT(updateCategoryROI(QModelIndex)));

  m_categorySelector->setRootModelIndex(m_model->classificationRoot());
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
  m_settings->setZSize(m_zSize->value());

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
  returnValue |= (m_zSize->value() != m_settings->zSize());
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
    modified |= (m_activeCategory->property(Category::DIM_X()).toUInt() != m_xCategorySize->value());
    modified |= (m_activeCategory->property(Category::DIM_Y()).toUInt() != m_yCategorySize->value());
    modified |= (m_activeCategory->property(Category::DIM_Z()).toUInt() != m_zCategorySize->value());
  }

  return modified;
}

//------------------------------------------------------------------------
void ROISettingsPanel::writeCategoryProperties()
{
  if (m_activeCategory)
  {
    m_activeCategory->addProperty(Category::DIM_X(), m_xCategorySize->value());
    m_activeCategory->addProperty(Category::DIM_Y(), m_yCategorySize->value());
    m_activeCategory->addProperty(Category::DIM_Z(), m_zCategorySize->value());
  }
}

//------------------------------------------------------------------------
void ROISettingsPanel::updateCategoryROI(const QModelIndex& index)
{
  if (!index.isValid())
    return;

  ItemAdapterPtr itemPtr = itemAdapter(index);
  if (isCategory(itemPtr))
    return;

  CategoryAdapterPtr category = categoryPtr(itemPtr);
  if (m_activeCategory && m_activeCategory.get() != category)
  {
    // Check for changes
    if (categoryROIModified())
    {
      QMessageBox msg;
      msg.setWindowTitle(tr("ESPINA"));
      msg.setText(tr("The properties of the category \"%1\" have been modified.\nDo you want to save the changes?").arg(m_activeCategory->data().toString()));
      msg.setStandardButtons(QMessageBox::Yes|QMessageBox::No);
      if (msg.exec() == QMessageBox::Yes)
        writeCategoryProperties();
    }
  }

  m_activeCategory = m_model->smartPointer(category);

  // 2013-03-19 Fix missing category properties in some cases. By default revert to "default ROI" values.
  if (!m_activeCategory->properties().contains(Category::DIM_X()) ||
      !m_activeCategory->properties().contains(Category::DIM_Y()) ||
      !m_activeCategory->properties().contains(Category::DIM_Z()))
  {
    m_activeCategory->addProperty(Category::DIM_X(), QVariant(m_settings->xSize()));
    m_activeCategory->addProperty(Category::DIM_Y(), QVariant(m_settings->ySize()));
    m_activeCategory->addProperty(Category::DIM_Z(), QVariant(m_settings->zSize()));
  }

  QVariant xSize = category->property(Category::DIM_X());
  QVariant ySize = category->property(Category::DIM_Y());
  QVariant zSize = category->property(Category::DIM_Z());

  if (!xSize.isValid() || !ySize.isValid() || !zSize.isValid())
  {
    xSize = m_xSize->value();
    ySize = m_ySize->value();
    zSize = m_zSize->value();
  }

  m_xCategorySize->setValue(xSize.toUInt());
  m_yCategorySize->setValue(ySize.toUInt());
  m_zCategorySize->setValue(zSize.toUInt());
}
