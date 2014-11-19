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

// ESPINA
#include <GUI/Model/ItemAdapter.h>
#include <GUI/Model/ViewItemAdapter.h>
#include <GUI/Representations/MeshRepresentation.h>
#include <GUI/Representations/Renderers/MeshRenderer.h>
#include <GUI/Representations/Renderers/Renderer.h>
#include <GUI/View/RenderView.h>

// VTK
#include <vtkPropPicker.h>

// Qt
#include <QApplication>

namespace ESPINA
{
  //-----------------------------------------------------------------------------
  MeshRenderer::MeshRenderer(QObject *parent)
  : RepresentationRenderer{parent}
  , m_picker              {nullptr}
  {
  }

  //-----------------------------------------------------------------------------
  MeshRenderer::~MeshRenderer()
  {
    for (auto item: m_representations.keys())
    {
      if (m_enable)
        for (auto rep: m_representations[item])
          for (auto prop: rep->getActors())
            m_view->removeActor(prop);

      m_representations[item].clear();
    }
    m_representations.clear();

    if(m_picker != nullptr)
      m_picker->GetPickList()->RemoveAllItems();
  }

  //-----------------------------------------------------------------------------
  void MeshRenderer::setView(RenderView *view)
  {
    Renderer::setView(view);

    m_picker = vtkSmartPointer<vtkPropPicker>::New();
    m_picker->InitializePickList();
    m_picker->PickFromListOn();
  }

  //-----------------------------------------------------------------------------
  void MeshRenderer::addRepresentation(ViewItemAdapterPtr item, RepresentationSPtr rep)
  {
    MeshRepresentationSPtr mesh = std::dynamic_pointer_cast<MeshRepresentation>(rep);
    if (mesh.get() != nullptr)
    {
      if (m_representations.keys().contains(item))
        m_representations[item] << rep;
      else
      {
        RepresentationSList list;
        list << rep;
        m_representations.insert(item, list);
      }

      if (m_enable)
        for (auto prop: rep->getActors())
        {
          m_view->addActor(prop);
          m_picker->AddPickList(prop);
        }
    }
  }

  //-----------------------------------------------------------------------------
  void MeshRenderer::removeRepresentation(RepresentationSPtr rep)
  {
    MeshRepresentationSPtr mesh = std::dynamic_pointer_cast<MeshRepresentation>(rep);
    if (mesh.get() != nullptr)
    {
      for (auto item: m_representations.keys())
        if (m_representations[item].contains(rep))
        {
          if (m_enable)
            for (auto prop: rep->getActors())
            {
              m_view->removeActor(prop);
              m_picker->DeletePickList(prop);
            }

          m_representations[item].removeAll(rep);

          if (m_representations[item].isEmpty())
            m_representations.remove(item);
        }
    }
  }

  //-----------------------------------------------------------------------------
  bool MeshRenderer::canRender(ItemAdapterPtr item) const
  {
    if(item->type() == ItemAdapter::Type::SEGMENTATION)
    {
      auto viewItem = dynamic_cast<ViewItemAdapterPtr>(item);
      if(viewItem != nullptr)
      {
        return hasMeshData(viewItem->output());
      }
    }
    return false;
  }
  //-----------------------------------------------------------------------------
  bool MeshRenderer::managesRepresentation(const QString &repType) const
  {
    return (repType == MeshRepresentation::TYPE);
  }

  //-----------------------------------------------------------------------------
  bool MeshRenderer::hasRepresentation(RepresentationSPtr rep) const
  {
    for (auto item: m_representations.keys())
      if (m_representations[item].contains(rep))
        return true;

    return false;
  }

  //-----------------------------------------------------------------------------
  void MeshRenderer::hide()
  {
    if (!m_enable)
      return;

    for (auto item: m_representations.keys())
      for (auto rep: m_representations[item])
        for (auto prop: rep->getActors())
        {
          m_view->removeActor(prop);
          m_picker->DeletePickList(prop);
        }

    emit renderRequested();
  }

  //-----------------------------------------------------------------------------
  void MeshRenderer::show()
  {
     if (m_enable)
       return;

     QApplication::setOverrideCursor(Qt::WaitCursor);
     for (auto item: m_representations.keys())
       for (auto rep: m_representations[item])
         for (auto prop: rep->getActors())
         {
           m_view->addActor(prop);
           m_picker->AddPickList(prop);
         }

     QApplication::restoreOverrideCursor();
     emit renderRequested();
  }

  //-----------------------------------------------------------------------------
  unsigned int MeshRenderer::numberOfvtkActors() const
  {
    unsigned int returnVal = 0;
    for (auto item: m_representations.keys())
      for (auto rep: m_representations[item])
        if (rep->isVisible()) ++returnVal;

    return returnVal;
  }

  //-----------------------------------------------------------------------------
  ViewItemAdapterList MeshRenderer::pick(int x, int y, Nm zUnused, vtkSmartPointer<vtkRenderer> renderer, RenderableItems itemType,  bool repeat)
  {
    ViewItemAdapterList selection;
    QList<vtkProp *> removedProps;

    if (!renderer || !renderer.GetPointer() || !itemType.testFlag(ESPINA::SEGMENTATION))
      return selection;

    while (m_picker->Pick(x, y, 0, renderer))
    {
      double point[3];
      m_picker->GetPickPosition(point);
      m_lastValidPickPosition = NmVector3{ point[0], point[1], point[2] };

      vtkProp *pickedProp = m_picker->GetViewProp();
      Q_ASSERT(pickedProp);

      m_picker->DeletePickList(pickedProp);
      removedProps << pickedProp;

      for (auto item: m_representations.keys())
        for (auto rep: m_representations[item])
        if (!selection.contains(item) && rep->isVisible() && rep->hasActor(pickedProp))
        {
          selection << item;

          if (!repeat)
          {
            for(auto actor: removedProps)
              m_picker->AddPickList(actor);

            return selection;
          }

          break;
        }
    }

    for(auto actor: removedProps)
      m_picker->AddPickList(actor);

    return selection;
  }

} // namespace ESPINA

