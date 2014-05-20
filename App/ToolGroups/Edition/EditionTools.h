/*
 <one line to give the program's name and a brief idea of what it does.>
 Copyright (C) 2013 Felix de las Pozas Alvarez <fpozas@cesvima.upm.es>

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

#ifndef ESPINA_EDITION_TOOLS_H_
#define ESPINA_EDITION_TOOLS_H_

#include <Support/ToolGroup.h>
#include <GUI/Model/ModelAdapter.h>
#include <GUI/View/Selection.h>
#include <Core/Factory/FilterFactory.h>
#include <GUI/ModelFactory.h>

#include "ManualEditionTool.h"
#include "SplitTool.h"
#include "MorphologicalEditionTool.h"

class QUndoStack;

namespace EspINA
{
  class EditionTools
  : public ToolGroup
  {
    class ManualFilterFactory
    : public FilterFactory
    {
      virtual FilterSPtr createFilter(InputSList inputs, const Filter::Type& filter, SchedulerSPtr scheduler) const throw (Unknown_Filter_Exception);

      virtual FilterTypeList providedFilters() const;

    private:
      mutable FetchBehaviourSPtr m_fetchBehaviour;
    };

    Q_OBJECT
    public:
      EditionTools(ModelAdapterSPtr model,
                   ModelFactorySPtr factory,
                   ViewManagerSPtr  viewManager,
                   QUndoStack      *undoStack,
                   QWidget         *parent = nullptr);
      virtual ~EditionTools();

      virtual void setEnabled(bool value);

      virtual bool enabled() const;

      virtual ToolSList tools();

    public slots:
      void selectionChanged(SelectionSPtr);
      void abortOperation();
      void drawStroke(CategoryAdapterSPtr, BinaryMaskSPtr<unsigned char> mask);

    private:
      ManualEditionToolSPtr        m_manualEdition;
      SplitToolSPtr                m_split;
      MorphologicalEditionToolSPtr m_morphological;
      ModelFactorySPtr             m_factory;
      QUndoStack                  *m_undoStack;
      ModelAdapterSPtr             m_model;
      FilterFactorySPtr            m_filterFactory;

      bool                         m_enabled;
  };

} // namespace EspINA

#endif // ESPINA_EDITION_TOOLS_H_