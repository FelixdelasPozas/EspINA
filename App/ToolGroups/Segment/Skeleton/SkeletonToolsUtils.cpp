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
#include "SkeletonToolsUtils.h"

// Qt
#include <QString>
#include <QObject>
#include <QMenu>
#include <QLabel>
#include <QWidgetAction>
#include <QBitmap>
#include <QPalette>
#include <QApplication>
#include <QColor>

// C++
#include <functional>

using namespace ESPINA;
using namespace ESPINA::Core;

QMap<const QString, Core::SkeletonStrokes> SkeletonToolsUtils::STROKES;

//--------------------------------------------------------------------
Core::SkeletonStrokes ESPINA::SkeletonToolsUtils::defaultStrokes(const CategoryAdapterSPtr category)
{
  SkeletonStrokes result;

  if(category)
  {
    if(category->classificationName().startsWith("Axon"))
    {
      result << SkeletonStroke(QObject::tr("Shaft"), -1, 0, true);
      result << SkeletonStroke(QObject::tr("Synapse en passant"), -1, 1, false);

      return result;
    }

    if(category->classificationName().startsWith("Dendrite"))
    {
      result << SkeletonStroke(QObject::tr("Shaft"), -1, 0, true);
      result << SkeletonStroke(QObject::tr("Spine"), -1, 0, true);
      result << SkeletonStroke(QObject::tr("Subspine"), -1, 0, true);
      result << SkeletonStroke(QObject::tr("Synapse on shaft"), -1, 1, false);
      result << SkeletonStroke(QObject::tr("Synapse on spine head"), -1, 1, false);
      result << SkeletonStroke(QObject::tr("Synapse on spine neck"), -1, 1, false);

      return result;
    }

    result << SkeletonStroke(QObject::tr("Stroke"), -1, 0, true);
  }
  else
  {
    result << SkeletonStroke(QObject::tr("Stroke"), -1, 0, true);
  }

  return result;
}

//--------------------------------------------------------------------
void ESPINA::SkeletonToolsUtils::saveStrokes(std::shared_ptr<QSettings> settings)
{
  settings->beginGroup("SkeletonStrokes");

  for(auto category: STROKES.keys())
  {
    settings->beginGroup(category);

    for(auto stroke: STROKES[category])
    {
      settings->setValue(stroke.name, QVariant::fromValue(stroke));
    }

    settings->endGroup();
  }

  settings->endGroup();
}

//--------------------------------------------------------------------
void ESPINA::SkeletonToolsUtils::loadStrokes(std::shared_ptr<QSettings> settings)
{
  STROKES.clear();

  const QString toolKey{"SkeletonStrokes/"};

  std::function<void(void)> loadGroupStrokes = [settings, toolKey, &loadGroupStrokes]()
  {
    // NOTE: doing it this way we need to remove the tool settings group before adding the strokes to STROKES.
    auto groupKey = settings->group().remove(0, toolKey.length());

    for(auto subGroup: settings->childGroups())
    {
      settings->beginGroup(subGroup);

      loadGroupStrokes();

      settings->endGroup();
    }

    for(auto key: settings->childKeys())
    {
      QVariant value = settings->value(key);

      STROKES[groupKey] << value.value<ESPINA::Core::SkeletonStroke>();
    }
  };

  settings->beginGroup("SkeletonStrokes");

  for(auto subGroup: settings->childGroups())
  {
    settings->beginGroup(subGroup);

    loadGroupStrokes();

    settings->endGroup();
  }

  settings->endGroup();
}

//--------------------------------------------------------------------
QMenu* SkeletonToolsUtils::createStrokesContextMenu(const QString& title, const Core::SkeletonStrokes &strokes, const CategoryAdapterSPtr category)
{
  QMenu *menu = nullptr;
  const auto palette = QApplication::palette();

  const QString style = QObject::tr("QMenu { background-color: %1; border: 1px solid black; }"
                                    "QMenu::item { background-color: transparent; }"
                                    "QMenu::item:selected { background-color: blue; }").arg(palette.color(QPalette::Window).name());

  const auto categoryColor = category->color().hue();

  if(!strokes.isEmpty())
  {
    menu = new QMenu(title);
    menu->setHidden(true);
    menu->setStyleSheet(style);

    auto label = new QLabel(QObject::tr("<b>%1</b>").arg(title));
    label->setStyleSheet("QLabel { margin: 2px; background-color : darkblue; color : white; }");
    label->setAlignment(Qt::AlignCenter);
    label->setBaseSize(label->size().width(), label->size().height()+10);
    auto action = new QWidgetAction(menu);
    action->setDefaultWidget(label);
    action->setEnabled(false);

    menu->addAction(action);

    for(int i = 0; i < strokes.size(); ++i)
    {
      auto stroke = strokes.at(i);

      QPixmap original(ICONS.at(stroke.type));
      QPixmap copy(original.size());
      const auto color = stroke.colorHue == -1 ? categoryColor : stroke.colorHue;
      copy.fill(QColor::fromHsv(color,255,255));
      copy.setMask(original.createMaskFromColor(Qt::transparent));

      auto strokeAction = menu->addAction(stroke.name);
      strokeAction->setIcon(QIcon(copy));
      strokeAction->setIconVisibleInMenu(true);
    }

    menu->addSeparator();
    menu->addAction(QObject::tr("Cancel"));
    menu->updateGeometry();
  }

  return menu;
}
