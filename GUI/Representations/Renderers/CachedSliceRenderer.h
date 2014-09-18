/*

 Copyright (C) 2014 Felix de las Pozas Alvarez <fpozas@cesvima.upm.es>

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

#ifndef ESPINA_CACHED_SLICE_RENDERER_H_
#define ESPINA_CACHED_SLICE_RENDERER_H_

#include "GUI/EspinaGUI_Export.h"

// ESPINA
#include <Core/EspinaTypes.h>
#include <Core/MultiTasking/Task.h>
#include <GUI/Representations/SliceCachedRepresentation.h>
#include "RepresentationRenderer.h"

// VTK
#include <vtkImageActor.h>
#include <QMutex>

class vtkPropPicker;

namespace ESPINA
{
  class CachedSliceRendererTask;
  using CachedSliceRendererTaskPtr  = CachedSliceRendererTask *;
  using CachedSliceRendererTaskSPtr = std::shared_ptr<CachedSliceRendererTask>;

  class EspinaGUI_EXPORT CachedSliceRenderer
  : public RepresentationRenderer
  {
    Q_OBJECT
    public:
      /* \brief CachedSliceRenderer class constructor.
       * \param[in] parent, raw pointer of the QObject parent of this one.
       *
       */
      explicit CachedSliceRenderer(SchedulerSPtr scheduler, QObject *parent = nullptr);

      /* \brief CachedSliceRenderer class destructor.
       *
       */
      virtual ~CachedSliceRenderer();

      /* \brief Implements Renderer::icon() const.
       *
       */
      virtual const QIcon icon() const
      { return QIcon(":/espina/slice.png"); }

      /* \brief Implements Renderer::name() const.
       *
       */
      virtual const QString name() const
      { return "Slice (Cached)"; }

      /* \brief Implements Renderer::tooltip() const.
       *
       */
      virtual const QString tooltip() const
      { return "Segmentation's Slices (Cached)"; }

      /* \brief RepresentationRenderer::addRepresentation().
       *
       */
      virtual void addRepresentation(ViewItemAdapterPtr item, RepresentationSPtr rep);

      /* \brief RepresentationRenderer::removeRepresentation().
       *
       */
      virtual void removeRepresentation(RepresentationSPtr rep);

      /* \brief RepresentationRenderer::hasRepresentation() const.
       *
       */
      virtual bool hasRepresentation(RepresentationSPtr rep) const;

      /* \brief RepresentationRenderer::managesRepresentation() const.
       *
       */
      virtual bool managesRepresentation(const QString &representationType) const;

      /* \brief Implements Renderer::clone() const.
       *
       */
      virtual RendererSPtr clone() const
      { return RendererSPtr(new CachedSliceRenderer(m_scheduler)); }

      /* \brief Implements Renderer::numberOfvtkActors() const.
       *
       */
      virtual unsigned int numberOfvtkActors() const;

      /* \brief Implements RepresentationRenderer::renderableItems() const.
       *
       */
      virtual RenderableItems renderableItems() const
      { return RenderableItems(RenderableType::CHANNEL|RenderableType::SEGMENTATION); }

      /* \brief Implements RepresentationRenderer::canRender() const.
       *
       */
      virtual bool canRender(ItemAdapterPtr item) const
      { return (item->type() == ItemAdapter::Type::SEGMENTATION || item->type() == ItemAdapter::Type::CHANNEL); }

      /* \brief Implements Renderer::renderType() const.
       *
       */
      virtual RendererTypes renderType() const
      { return RendererTypes(RENDERER_VIEW2D); }

      /* \brief Implements Renderer::numberOfRenderedItems() const.
       *
       */
      virtual int numberOfRenderedItems() const
      { return m_representations.size(); }

      /* \brief RepresentationRenderer::pick().
       *
       */
      virtual ViewItemAdapterList pick(int x, int y, Nm z,
                                       vtkSmartPointer<vtkRenderer> renderer,
                                       RenderableItems itemType = RenderableItems(),
                                       bool repeat = false);

      /* \brief Modifies the cache window width.
       * \param[in] width, new width.
       *
       * Range: 0, N. If the window is extended new nodes and tasks are created, if the window is
       * reduced nodes and tasks are deleted. It doesn't affect window actual position (global actor
       * is unaffected). To ensure a fully populated cache this method should be called after the cache
       * has been filled first with fillcached(int position) or you can end with a mostly empty cache
       * except for the nodes added with the method.
       */
      virtual void setWindowWidth(unsigned int width);

      /* \brief Returns the cache window width.
       *
       */
      unsigned int getWindowWidth()
      { return m_windowWidth; };

      /* \brief Set the maximum window width for the cache.
       * \param[in] width, new maximum window width.
       *
       * Sets the maximum window width for the cache. If the value us less
       * than the actual window size, the cache is resized.
       */
      virtual void setWindowMaximumWidth(unsigned int width);

      /* \brief Returns the maximum cache window width.
       *
       */
      unsigned int getMaximumWindowWidth()
      { return m_maxWindowWidth; };

      /* \brief Returns memory consumed by all the actors of the cache.
       *
       */
      virtual unsigned long long getEstimatedMemoryUsed();

      /* \brief Overrides Renderer::setView().
       *
       */
      virtual void setView(RenderView* rView) override;

      /* \brief Overrides Renderer::setEnable().
       *
       */
      virtual void setEnable(bool value) override;

    protected:
      /* \brief Implements Renderer:hide().
       *
       */
      virtual void hide();

      /* \brief Implements Renderer::show().
       *
       */
      virtual void show();

    private:
      static const int WINDOW_INCREMENT;

      /* \brief Circular buffer node.
       *
       * Struct to use as circular buffer node. If worker and actor are both nullptr that means the node is unused.
       */
      struct CacheNode
      {
          // written by renderer, read by tasks
          Nm                                         position;
          CachedSliceRendererTaskSPtr                worker;

          // written by both renderer and tasks
          QMap<CachedRepresentationSPtr, vtkSmartPointer<vtkImageActor>> representations;
          CachedRepresentationSList                  repsToAdd;
          CachedRepresentationSList                  repsToDelete;
          bool                                       restart;

          // tasks never touch those two
          CacheNode                                 *next;
          CacheNode                                 *previous;

          // used by both tasks and renderer
          QReadWriteLock                             mutex;

          CacheNode(): position{0}, worker{nullptr}, restart{false}, next{nullptr}, previous{nullptr} {};
          ~CacheNode() { worker = nullptr; };
      };

    public slots:
    	/* \brief Updates the representation forcing a task execution.
    	 *
    	 */
      void updateRepresentation();

    	/* \brief Updates the representation when it's visibility changes.
    	 *
    	 */
      void updateRepresentationVisibility();

    	/* \brief Updates the representation when it's color changes.
    	 *
    	 */
      void updateRepresentationColor();

    protected slots:
      /* \brief Set position of the cache.
       * \param[in] plane, unused.
       * \param[in] pos, new position.
       *
       * Sets the position of the cache. If the position is not cached (not within the bounds of the
       * cache window) the cache is emptied and filled again. If the position is cached the cache
       * position shifts to that place and the borders of the window deletes/creates task and actors conveniently.
       */
      void setPosition(Plane plane, Nm pos);

      /* \brief Modifies the cache when the spacing of the view changes.
       *
       */
      void resolutionChanged();

      /* \brief Manages representations after a task has finished execution.
       * \param[in] node, node of the finished task.
       *
       */
      void renderFrame(CachedSliceRenderer::CacheNode *node);

    private:
      /* \brief Create a task for the specified node.
       * \param[in] priority, Initial priority of the task.
       *
       * Returns a task with the input set, ready to be executed. The parameters specify the
       * position (slice) of the actors and the priority of the task (NORMAL priority if omitted).
       */
      virtual CachedSliceRendererTaskSPtr createTask(Priority priority = Priority::LOW);

      /* \brief Prints the cache window info.
       *
       * Prints the occupation of the cache nodes along with the memory consumption and average task execution time.
       */
      void printBufferInfo();

      /* \brief Initializes the cache.
       *
       */
      void initCache();

      /* \brief Populates all the cache.
       * \param[in] position, center position to start.
       *
       * Deletes all previous existing actors and fills the cache nodes with tasks.
       */
      void fillCache(Nm position);

      /* \brief Returns the memory used for the actors in this node.
       * \param[in] node, node pointer.
       *
       */
      unsigned long long getNodeExtimatedMemoryUsed(CacheNode *node);

      /* \brief Returns the list of valid representations for a given position.
       *
       */
      CachedRepresentationSList validRepresentationsForPosition(const Nm pos) const;

      // Protected attributes.
      unsigned int   m_windowWidth;    // Actual cache window width
      unsigned int   m_maxWindowWidth; // Maximum allowed cache window width.
      CacheNode     *m_actualPos;      // Node interpreted as the "center" of the circular buffer and actual actors on the view.
      CacheNode     *m_edgePos;        // Node interpreted as a edge of the circular buffer and the one inserting/deleting tasks.
      Nm             m_windowSpacing;

      QMap<CachedRepresentationSPtr, vtkSmartPointer<vtkImageActor>> m_representationsActors;

      vtkSmartPointer<vtkPropPicker> m_picker;
      SchedulerSPtr                  m_scheduler;
      int                            m_planeIndex;
      bool                           m_needCameraReset;

      friend class CachedSliceRendererTask;
  };

} // namespace ESPINA

#endif // ESPINA_CACHED_SLICE_RENDERER_H_
