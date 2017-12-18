/*

 Copyright (C) 2017 Felix de las Pozas Alvarez <fpozas@cesvima.upm.es>

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

// ESPINA
#include <GUI/Model/CategoryAdapter.h>
#include <App/Dialogs/SkeletonStrokeDefinition/StrokeDefinitionDialog.h>
#include <App/ToolGroups/Segment/Skeleton/SkeletonToolsUtils.h>

// Qt
#include <QBitmap>
#include <QCloseEvent>

using namespace ESPINA;
using namespace ESPINA::Core;
using namespace ESPINA::SkeletonToolsUtils;

//--------------------------------------------------------------------
StrokeDefinitionDialog::StrokeDefinitionDialog(SkeletonStrokes &strokes, const CategoryAdapterSPtr category, QWidget* parent, Qt::WindowFlags flags)
: QDialog   {parent, flags}
, m_strokes (strokes)
, m_category{category}
{
  setupUi(this);
  m_hueWidget->reserveInitialValue(false);
  m_typeCombo->addItem(QIcon(":/espina/line.svg"), tr("Solid line"));
  m_typeCombo->addItem(QIcon(":/espina/dashed-line.svg"), tr("Dashed line"));
  m_removeButton->setEnabled(!m_strokes.isEmpty());
  enableProperties(!m_strokes.isEmpty());

  setWindowTitle(tr("%1 Skeleton strokes definition").arg(category->name()));

  connectSignals();

  updateStrokeList();

  updateStrokeProperties();
}

//--------------------------------------------------------------------
void StrokeDefinitionDialog::onAddButtonPressed()
{
  auto categoryColor = m_category->color().hue();
  QPixmap original(":/espina/line.svg");
  QPixmap copy(original.size());
  copy.fill(QColor::fromHsv(categoryColor,255,255));
  copy.setMask(original.createMaskFromColor(Qt::transparent));

  QStringList names;
  for(auto stroke: m_strokes)
  {
    if(stroke.name.startsWith("Undefined"))
    {
      names << stroke.name;
    }
  }

  int number = 0;
  auto name = tr("Undefined%1").arg(number == 0 ? "" : " (" + QString::number(number + 1) + ")");
  while(names.contains(name))
  {
    ++number;
    name = tr("Undefined%1").arg(number == 0 ? "" : " (" + QString::number(number + 1) + ")");
  }

  m_strokes.push_back(SkeletonStroke{name, categoryColor, 0, true});

  auto item = new QListWidgetItem(copy, name);
  m_list->addItem(item);
  m_list->setCurrentItem(item);
  m_list->update();

  m_removeButton->setEnabled(true);
}

//--------------------------------------------------------------------
void StrokeDefinitionDialog::onRemoveButtonPressed()
{
  auto index = m_list->currentIndex();
  if(index.isValid())
  {
    auto stroke = m_strokes[index.row()];
    m_strokes.removeOne(stroke);

    m_list->blockSignals(true);
    auto item = m_list->takeItem(index.row());
    delete item;
    m_list->blockSignals(false);
  }

  m_list->update();
  if(!m_strokes.isEmpty())
  {
    m_list->setCurrentItem(m_list->item(0));
  }
  updateStrokeProperties();

  m_removeButton->setEnabled(!m_strokes.isEmpty());
}

//--------------------------------------------------------------------
void StrokeDefinitionDialog::onStrokeChanged(int row)
{
  updateStrokeProperties();

  // disable removing default strokes.
  auto defaultvalues = defaultStrokes(m_category);
  auto index  = std::min(row, m_strokes.size() - 1);
  auto stroke = m_strokes.at(index);
  for(auto defaultStroke: defaultvalues)
  {
    if(stroke.name == defaultStroke.name)
    {
      m_removeButton->setEnabled(false);
      return;
    }
  }

  m_removeButton->setEnabled(true);
}

//--------------------------------------------------------------------
void StrokeDefinitionDialog::closeEvent(QCloseEvent* event)
{
  int number = 0;
  for(auto stroke: m_strokes)
  {
    if(stroke.name.startsWith("Undefined")) ++number;
  }

  if(number != 0)
  {
    GUI::DefaultDialogs::ErrorMessage(tr("There are %1 stroke type%2 with the default name \"Undefined\".\nChange the name%2 to %3valid one%2.").arg(number).arg(number > 1 ? "s":"").arg(number > 1 ? "":"a "));
    event->ignore();
    return;
  }

  if(m_strokes.isEmpty())
  {
    GUI::DefaultDialogs::ErrorMessage(tr("There must be at least one stroke type definition."));
    event->ignore();
    return;
  }

  QStringList names;
  for(auto stroke: m_strokes)
  {
    if(!names.contains(stroke.name))
    {
      names << stroke.name;
    }
    else
    {
      GUI::DefaultDialogs::ErrorMessage(tr("Stroke names must be different.\nThere are at least two strokes with the same name: %1").arg(stroke.name));
      event->ignore();
      return;
    }
  }

  QDialog::closeEvent(event);
}

//--------------------------------------------------------------------
void StrokeDefinitionDialog::updateStrokeList()
{
  m_list->blockSignals(true);
  m_list->clear();

  for(int i = 0; i < m_strokes.size(); ++i)
  {
    auto stroke = m_strokes.at(i);
    QPixmap original(ICONS.at(stroke.type));
    QPixmap copy(original.size());
    copy.fill(QColor::fromHsv(stroke.colorHue,255,255));
    copy.setMask(original.createMaskFromColor(Qt::transparent));

    auto item = new QListWidgetItem(copy, stroke.name);

    m_list->insertItem(i, item);
  }
  m_list->blockSignals(false);
}

//--------------------------------------------------------------------
void StrokeDefinitionDialog::connectSignals()
{
  connect(m_addButton, SIGNAL(pressed()),
          this,        SLOT(onAddButtonPressed()));

  connect(m_removeButton, SIGNAL(pressed()),
          this,           SLOT(onRemoveButtonPressed()));

  connect(m_list, SIGNAL(currentRowChanged(int)),
          this,   SLOT(onStrokeChanged(int)));

  connect(m_hueWidget, SIGNAL(newHsv(int, int, int)),
          this,        SLOT(onHueChanged(int)));

  connect(m_name, SIGNAL(textChanged(const QString &)),
          this,   SLOT(onTextChanged(const QString &)));

  connect(m_typeCombo, SIGNAL(currentIndexChanged(int)),
          this,        SLOT(onTypeChanged(int)));

  connect(m_validMeasure, SIGNAL(stateChanged(int)),
          this,           SLOT(onValidCheckChanged()));

  connect(m_useCategoryColor, SIGNAL(stateChanged(int)),
          this,               SLOT(onCategoryColorChecked(int)));
}

//--------------------------------------------------------------------
void StrokeDefinitionDialog::enableProperties(bool value)
{
  m_hueWidget->setEnabled(m_useCategoryColor->isChecked());
  m_name->setEnabled(value);
  m_typeCombo->setEnabled(value);
  m_validMeasure->setEnabled(value);
  m_useCategoryColor->setEnabled(value);

  if(!value)
  {
    m_name->blockSignals(true);
    m_name->clear();
    m_name->blockSignals(false);
  }
}

//--------------------------------------------------------------------
void StrokeDefinitionDialog::updateStrokeProperties()
{
  enableProperties(!m_strokes.isEmpty());

  auto currentIndex = m_list->currentIndex();
  auto categoryColor = m_category->color().hue();

  if(currentIndex.isValid() && !m_strokes.isEmpty())
  {
    auto index  = std::min(currentIndex.row(), m_strokes.size() - 1);
    auto stroke = m_strokes.at(index);

    m_name->blockSignals(true);
    m_hueWidget->blockSignals(true);
    m_typeCombo->blockSignals(true);
    m_validMeasure->blockSignals(true);
    m_useCategoryColor->blockSignals(true);

    m_name->setText(stroke.name);
    m_hueWidget->setHueValue(stroke.colorHue);
    m_hueWidget->setEnabled(stroke.colorHue != categoryColor);
    m_typeCombo->setCurrentIndex(stroke.type);
    m_validMeasure->setChecked(stroke.useMeasure);
    m_useCategoryColor->setChecked(stroke.colorHue == categoryColor);

    m_name->blockSignals(false);
    m_hueWidget->blockSignals(false);
    m_typeCombo->blockSignals(false);
    m_validMeasure->blockSignals(false);
    m_useCategoryColor->blockSignals(false);
  }
}

//--------------------------------------------------------------------
void StrokeDefinitionDialog::onHueChanged(int hueValue)
{
  auto index = m_list->currentIndex();

  if(index.isValid())
  {
    auto &stroke = m_strokes[index.row()];
    stroke.colorHue = hueValue;

    QPixmap original(ICONS.at(stroke.type));
    QPixmap copy(original.size());
    copy.fill(QColor::fromHsv(hueValue,255,255));
    copy.setMask(original.createMaskFromColor(Qt::transparent));

    auto item = m_list->currentItem();
    item->setIcon(copy);
    m_list->update();
  }
}

//--------------------------------------------------------------------
void StrokeDefinitionDialog::onTextChanged(const QString& text)
{
  auto index = m_list->currentIndex();

  if(index.isValid())
  {
    auto &stroke = m_strokes[index.row()];
    stroke.name = text;

    auto item = m_list->currentItem();
    item->setText(text);
    m_list->update();
  }
}

//--------------------------------------------------------------------
void StrokeDefinitionDialog::onTypeChanged(int index)
{
  auto currentIndex = m_list->currentIndex();

  if(currentIndex.isValid())
  {
    auto &stroke = m_strokes[currentIndex.row()];
    stroke.type = index;

    QPixmap original(ICONS.at(stroke.type));
    QPixmap copy(original.size());
    copy.fill(QColor::fromHsv(stroke.colorHue,255,255));
    copy.setMask(original.createMaskFromColor(Qt::transparent));

    auto item = m_list->currentItem();
    item->setIcon(copy);
    m_list->update();
  }
}

//--------------------------------------------------------------------
void StrokeDefinitionDialog::onValidCheckChanged()
{
  auto currentIndex = m_list->currentIndex();

  if(currentIndex.isValid())
  {
    auto &stroke = m_strokes[currentIndex.row()];
    stroke.useMeasure = m_validMeasure->isChecked();
  }
}

//--------------------------------------------------------------------
void StrokeDefinitionDialog::onCategoryColorChecked(int unused)
{
  auto currentIndex = m_list->currentIndex();
  auto checked      = m_useCategoryColor->isChecked();

  if(currentIndex.isValid())
  {
    if(checked)
    {
      auto &stroke = m_strokes[currentIndex.row()];
      stroke.colorHue = m_category->color().hue();

      m_hueWidget->setHueValue(stroke.colorHue);
    }

    m_hueWidget->setEnabled(!checked);
  }
}
