/*
 <one line to give the program's name and a brief idea of what it does.>
 Copyright (C) 2014 Félix de las Pozas Álvarez <felixdelaspozas@gmail.com>

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

#ifndef ESPINA_CACHED_SLICE_RENDERER_H_
#define ESPINA_CACHED_SLICE_RENDERER_H_

// EspINA
#include <Core/EspinaTypes.h>
#include <GUI/Representations/Representation.h>
#include "RepresentationRenderer.h"

// VTK
#include <vtkImageActor.h>

class vtkPropPicker;

namespace EspINA
{
  class CachedSliceRendererTask;
  using CachedSliceRendererTaskPtr  = CachedSliceRendererTask *;
  using CachedSliceRendererTaskSPtr = std::shared_ptr<CachedSliceRendererTask>;
  
  class CachedSliceRenderer
  : public RepresentationRenderer
  {
    Q_OBJECT
    public:
      /* \brief CachedSliceRenderer class constructor.
       * \param[in] parent QtObject parent of this class.
       */
      explicit CachedSliceRenderer(SchedulerSPtr scheduler, QObject *parent = 0);

      /* \brief CachedSliceRenderer class destructor.
       *
       */
      virtual ~CachedSliceRenderer();

      virtual const QIcon icon()      const   { return QIcon(":/espina/slice.png"); }
      virtual const QString name()    const   { return "Slice (Cached)"; }
      virtual const QString tooltip() const   { return "Segmentation's Slices (Cached)"; }

      virtual void addRepresentation(ViewItemAdapterPtr item, RepresentationSPtr rep);
      virtual void removeRepresentation(RepresentationSPtr rep);
      virtual bool hasRepresentation(RepresentationSPtr rep) const;
      virtual bool managesRepresentation(const QString &representationName) const;

      virtual RendererSPtr clone() const        { return RendererSPtr(new CachedSliceRenderer(m_scheduler)); }

      virtual unsigned int numberOfvtkActors() const;

      virtual RenderableItems renderableItems() const { return RenderableItems(RenderableType::CHANNEL|RenderableType::SEGMENTATION); }

      virtual RendererTypes renderType() const        { return RendererTypes(RENDERER_VIEW2D); }

      virtual int numberOfRenderedItems() const       { return m_representations.size(); }

      virtual ViewItemAdapterList pick(int x, int y, Nm z,
                                       vtkSmartPointer<vtkRenderer> renderer,
                                       RenderableItems itemType = RenderableItems(),
                                       bool repeat = false);

      virtual NmVector3 pickCoordinates() const;

      /* \brief Modifies the cache window width.
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

      /* \brief Implements Renderer::setView.
       *
       */
      virtual void setView(RenderView* rView);

    protected:
      virtual void hide();
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
          QMap<RepresentationSPtr, vtkSmartPointer<vtkImageActor>> representations;
          RepresentationSList                        repsToAdd;
          RepresentationSList                        repsToDelete;
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
      void updateRepresentation();
      void updateRepresentationVisibility();
      void updateRepresentationColor();

    protected slots:
      void changePosition(Plane plane, Nm pos);
      void resolutionChanged();
      void renderFrame(CachedSliceRenderer::CacheNode *node);

    private:
      /* \brief Create a task for the specified node.
       * \param[in] node* Pointer to the node.
       * \param[in] priority Initial priority of the task.
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
       *
       * Deletes all previous existing actors and fills the cache nodes with tasks.
       */
      void fillCache(Nm position);

      /* \brief Returns the memory used for the actors in this node.
       *
       */
      unsigned long long getNodeExtimatedMemoryUsed(CacheNode *node);

      /* \brief Set position of the cache.
       *
       * Sets the position of the cache. If the position is not cached (not within the bounds of the
       * cache window) the cache is emptied and filled again. If the position is cached the cache
       * position shifts to that place and the borders of the window deletes/creates task and actors conveniently.
       */
      virtual void setPosition(Nm position);

      // Protected attributes.
      unsigned int   m_windowWidth;    // Actual cache window width
      unsigned int   m_maxWindowWidth; // Maximum allowed cache window width.
      CacheNode     *m_actualPos;      // Node interpreted as the "center" of the circular buffer and actual actors on the view.
      CacheNode     *m_edgePos;        // Node interpreted as a edge of the circular buffer and the one inserting/deleting tasks.
      Nm             m_windowSpacing;

      QMap<RepresentationSPtr, vtkSmartPointer<vtkImageActor>> m_representationsActors;

      vtkSmartPointer<vtkPropPicker> m_picker;
      SchedulerSPtr                  m_scheduler;
      int                            m_planeIndex;
      bool                           m_needCameraReset;

      friend class CachedSliceRendererTask;
  };

} // namespace EspINA

#endif // ESPINA_CACHED_SLICE_RENDERER_H_
