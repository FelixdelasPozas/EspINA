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

#ifndef ESPINA_VOLUMETRIC_RENDERER_H
#define ESPINA_VOLUMETRIC_RENDERER_H

// ESPINA
#include "RepresentationRenderer.h"
#include <GUI/Representations/VolumetricRepresentation.hxx>
#include <GUI/View/RenderView.h>
#include <Core/EspinaTypes.h>

// VTK
#include <vtkVolumePicker.h>
#include <vtkVolume.h>

// Qt
#include <QApplication>

namespace ESPINA
{
  template<class T>
  class VolumetricRenderer
  : public RepresentationRenderer
  {
  public:
  	/** \brief VolumetricRenderer class constructor.
  	 * \param[in] parent, raw pointer of the QObject parent of this one.
  	 *
  	 */
    explicit VolumetricRenderer(QObject* parent = nullptr);

    /** \brief VolumetricRenderer class destructor.
     *
     */
    virtual ~VolumetricRenderer();

    /** \brief Implements Renderer::icon() const.
     *
     */
    const QIcon icon() const
    {return QIcon(":/espina/voxel.png");}

    /** \brief Implements Renderer::name() const.
     *
     */
    const QString name() const
    {return "Volumetric";}

    /** \brief Implements Renderer::tooltip() const.
     *
     */
    const QString tooltip() const
    {return "Segmentation's Volumes";}

    /** \brief Implements RepresentationRenderer::addRepresentation().
     *
     */
    virtual void addRepresentation(ViewItemAdapterPtr item, RepresentationSPtr rep);

    /** \brief Implements RepresentationRenderer::removeRepresentation().
     *
     */
    virtual void removeRepresentation(RepresentationSPtr rep);

    /** \brief Implements RepresentationRenderer::hasRepresentation().
     *
     */
    virtual bool hasRepresentation(RepresentationSPtr rep) const;

    /** \brief Implements RepresentationRenderer::managesRepresentation().
     *
     */
    virtual bool managesRepresentation(const QString &representationType) const;

    /** \brief Implements Renderer::clone() const.
     *
     */
    RendererSPtr clone() const
    {return RendererSPtr(new VolumetricRenderer());}

    /** \brief Implements Renderer::numberOfvtkActors() const.
     *
     */
    unsigned int numberOfvtkActors() const
    { return 0; }

    /** \brief Implements RepresentationRenderer::renderableItems().
     *
     */
    RenderableItems renderableItems() const
    { return RenderableItems(RenderableType::SEGMENTATION); }

    /** \brief Implements Renderer::renderType() const.
     *
     */
    RendererTypes renderType() const
    { return RendererTypes(RENDERER_VIEW3D); }

    /** \brief Implements RepresentationRenderer::canRender() const.
     *
     */
    bool canRender(ItemAdapterPtr item) const
    { return (item->type() == ItemAdapter::Type::SEGMENTATION); }

    /** \brief Implements Renderer::numberOfRenderedItems() const.
     *
     */
    int numberOfRenderedItems() const
    { return m_representations.size(); }

    /** \brief Implements RepresentationRenderer::pick().
     *
     */
    ViewItemAdapterList pick(int x, int y, Nm z,
                             vtkSmartPointer<vtkRenderer> renderer,
                             RenderableItems itemType = RenderableItems(),
                             bool repeat = false);

    /** \brief Overrides Renderer::setView().
     *
     */
    virtual void setView(RenderView *view) override;

  protected:
    /** \brief Implements Renderer::hide().
     *
     */
    void hide();

    /** \brief Implements Renderer::show().
     *
     */
    void show();

    vtkSmartPointer<vtkVolumePicker> m_picker;
  };

  //-----------------------------------------------------------------------------
  template<class T>
  VolumetricRenderer<T>::VolumetricRenderer(QObject* parent)
  : RepresentationRenderer{parent}
  , m_picker              {nullptr}
  {
  }

  //-----------------------------------------------------------------------------
  template<class T>
  VolumetricRenderer<T>::~VolumetricRenderer()
  {
    for(auto item: m_representations.keys())
    {
      if (m_enable)
        for(auto rep: m_representations[item])
          for(auto prop: rep->getActors())
            m_view->removeActor(prop);

      m_representations[item].clear();
    }

    m_representations.clear();

    if(m_picker != nullptr)
      m_picker->GetPickList()->RemoveAllItems();
  }

  //-----------------------------------------------------------------------------
  template<class T>
  void VolumetricRenderer<T>::addRepresentation(ViewItemAdapterPtr item, RepresentationSPtr rep)
  {
    VolumetricRepresentationSPtr<T> volume = std::dynamic_pointer_cast<VolumetricRepresentation<T>>(rep);
    if (volume.get() != nullptr)
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
        for(auto prop: rep->getActors())
        {
          m_view->addActor(prop);
          m_picker->AddPickList(prop);
        }
    }
  }

  //-----------------------------------------------------------------------------
  template<class T>
  void VolumetricRenderer<T>::setView(RenderView *view)
  {
      Renderer::setView(view);

      m_picker = vtkSmartPointer<vtkVolumePicker>::New();
      m_picker->InitializePickList();
      m_picker->PickFromListOn();
      m_picker->SetPickClippingPlanes(false);
      m_picker->SetPickCroppingPlanes(false);
      m_picker->SetPickTextureData(false);
      m_picker->SetTolerance(0);
  }

  //-----------------------------------------------------------------------------
  template<class T>
  void VolumetricRenderer<T>::removeRepresentation(RepresentationSPtr rep)
  {
    VolumetricRepresentationSPtr<T> volume = std::dynamic_pointer_cast<VolumetricRepresentation<T>>(rep);
    if (volume.get() != nullptr)
    {
      for(auto item: m_representations.keys())
        if (m_representations[item].contains(rep))
        {
          if (m_enable)
            for(auto prop: rep->getActors())
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
  template<class T>
  bool VolumetricRenderer<T>::hasRepresentation(RepresentationSPtr rep) const
  {
    for(auto item: m_representations.keys())
      if (m_representations[item].contains(rep))
        return true;

    return false;
  }

  //-----------------------------------------------------------------------------
  template<class T>
  bool VolumetricRenderer<T>::managesRepresentation(const QString &repType) const
  {
    return (repType == VolumetricRepresentation<T>::TYPE);
  }

  //-----------------------------------------------------------------------------
  template<class T>
  void VolumetricRenderer<T>::hide()
  {
      if (!m_enable)
        return;

      for(auto item: m_representations.keys())
        for(auto rep: m_representations[item])
          for(auto prop: rep->getActors())
          {
            m_view->removeActor(prop);
            m_picker->DeletePickList(prop);
          }

      emit renderRequested();
  }

  //-----------------------------------------------------------------------------
  template<class T>
  void VolumetricRenderer<T>::show()
  {
    if (m_enable)
      return;

    QApplication::setOverrideCursor(Qt::WaitCursor);
    for(auto item: m_representations.keys())
      for(auto rep: m_representations[item])
        for(auto prop: rep->getActors())
        {
          m_view->addActor(prop);
          m_picker->AddPickList(prop);
        }

    QApplication::restoreOverrideCursor();
    emit renderRequested();
  }

  //-----------------------------------------------------------------------------
  template<class T>
  ViewItemAdapterList VolumetricRenderer<T>::pick(int x, int y, Nm z, vtkSmartPointer<vtkRenderer> renderer, RenderableItems itemType, bool repeat)
  {
    ViewItemAdapterList selection;
    QList<vtkVolume *> removedProps;

    if (!renderer || !renderer.GetPointer() || !itemType.testFlag(ESPINA::SEGMENTATION))
      return selection;

    while (m_picker->Pick(x, y, 0, renderer))
    {
      double point[3];
      m_picker->GetPickPosition(point);
      m_lastValidPickPosition = NmVector3{ point[0], point[1], point[2] };

      vtkVolume *pickedProp = m_picker->GetVolume();
      Q_ASSERT(pickedProp);

      m_picker->DeletePickList(pickedProp);
      removedProps << pickedProp;

      for(auto item: m_representations.keys())
        for(auto rep: m_representations[item])
          if (rep->isVisible() && rep->hasActor(pickedProp) && !selection.contains(item))
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

    for(auto prop: removedProps)
      m_picker->AddPickList(prop);

    return selection;
  }

} // namespace ESPINA

#endif // ESPINA_VOLUMETRIC_RENDERER_H
