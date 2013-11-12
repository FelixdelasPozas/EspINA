/*
 * <one line to give the program's name and a brief idea of what it does.>
 * Copyright (C) 2013  Jorge Peña Pastor <jpena@cesvima.upm.es>
 *
 * This program is free software: you can redistribute it and/or modify
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

#ifndef ESPINA_SEED_GROW_SEGMENTATION_TOOL_H
#define ESPINA_SEED_GROW_SEGMENTATION_TOOL_H

#include <Support/Tool.h>

#include "SeedThreshold.h"
#include "ApplyROI.h"
#include <GUI/Widgets/ActionSelector.h>
#include <GUI/Selectors/Selector.h>

namespace EspINA {

  class SeedGrowSegmentationTool 
  : public Tool
  {
  public:
    explicit SeedGrowSegmentationTool();
    virtual ~SeedGrowSegmentationTool();

    virtual QList<QAction *> actions() const;

    virtual bool enabled() const;

    virtual void setEnabled(bool value);

    virtual void setInUse(bool value);

    virtual bool filterEvent(QEvent* e, RenderView *view);

    virtual QCursor cursor() const;

  private:
    void addVoxelSelector(QAction *action, SelectorSPtr selector);

  private:
    ActionSelector *m_selector;
    SeedThreshold  *m_seedThreshold;
    ApplyROI       *m_applyROI;

    QMap<QAction *, SelectorSPtr> m_voxelSelectors;
  };

  using SeedGrowSegmentationToolSPtr = std::shared_ptr<SeedGrowSegmentationTool>;

} // namespace EspINA

#endif // ESPINA_SEED_GROW_SEGMENTATION_TOOL_H
