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

#ifndef ESPINA_CACHED_REPRESENTATION_H_
#define ESPINA_CACHED_REPRESENTATION_H_

// EspINA
#include "Representation.h"
#include <Core/MultiTasking/Scheduler.h>

// Qt
#include <QList>
#include <QMutex>

// VTK
#include <vtkSmartPointer.h>
#include <vtkImageActor.h>

class vtkAssembly;
class vtkActor;

namespace EspINA
{
  class RenderView;

  class CachedRepresentationTask;
  using CachedRepresentationTaskPtr  = CachedRepresentationTask *;
  using CachedRepresentationTaskSPtr = std::shared_ptr<CachedRepresentationTask>;

  class CachedRepresentation
  : public Representation
  {
    Q_OBJECT
    public:
      /* \brief Default class constructor.
       *
       * Class constructor. The cache is created but not pupulated. this->m_actor is created and added
       * to the view in the subclases.
       */
      explicit CachedRepresentation(RenderView *view);

      /* \brief Class destructor.
       *
       * Class destructor. Removes m_actor from the view and deletes it, deletes all nodes and waits for
       * all pending tasks to end.
       */
      virtual ~CachedRepresentation();

      /* \brief Clears the cache.
       *
       * Does not modify the cache size, just deletes tasks and actors, and empties the global actor
       * (empties all nodes). Do not removes this->m_actor from the view.
       */
      virtual void clearCache();

      /* \brief Populates all the cache.
       *
       * Fills the cache nodes with tasks (if the virtual method createTask() returns one).
       */
      virtual void fillCache(int position);

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

      /* \brief Set position of the cache.
       *
       * Sets the position of the cache. If the position is not cached the cache is emptied and
       * filled again. If the position is cached the cache position shifts to that place and the
       * borders of the window deletes/creates task and actors conveniently.
       */
      virtual void setPosition(int position);

      /* \brief Returns true if the representation contains the actor.
       *
       */
      virtual bool hasActor(vtkProp *actor) const;

      /* \brief Sets the color of the channel.
       *
       * Sets the color of the channel, specified as a QColor.
       */
      virtual void setColor(const QColor &color);

      /* \brief Sets the brightness of the channel representation.
       *
       * Sets the brightness of the channel representation. Brightness value belongs to [-1,1].
       */
      virtual void setBrightness(double value);

      /* \brief Sets the channel representation constrat value.
       *
       * Sets the channel representation constrat value. Contrast value belongs to [0,2].
       */
      virtual void setContrast(double value);

      /* \brief Returns the actors that comprise this representation.
       *
       */
      virtual QList<vtkProp*> getActors();

      /* \brief Returns this representation settings widget.
       *
       */
      virtual RepresentationSettings *settingsWidget();

      /* \brief Updates the representation visibility.
       *
       */
      virtual void updateVisibility(bool visible);

      /* \brief Returns true if the representations needs to update at the moment of calling
       * this method.
       *
       */
      virtual bool needUpdate();

    protected:
      struct CacheNode;
    public slots:
      void renderFrame(CachedRepresentation::CacheNode *node);

    protected:

      /* \brief If a cache miss happens and the window is not in the maximum value, then the window size is
       *        incremented by WINDOW_INCREMENT size;
       */
      static int WINDOW_INCREMENT;

      /* \brief Circular buffer node.
       *
       * Struct to use as circular buffer node. If worker and actor are both nullptr that means the node is unused.
       */
      struct CacheNode
      {
          int                            position;
          CachedRepresentationTaskSPtr   worker;
          vtkSmartPointer<vtkImageActor> actor;
          unsigned long long             creationTime;
          CacheNode                     *next;
          CacheNode                     *previous;
          QMutex                         mutex;

          CacheNode(): position{0}, worker{nullptr}, actor{nullptr}, creationTime{0}, next{nullptr}, previous{nullptr} {};
      };

      /* \brief Virtual method to override by subclasses.
       *
       * Virtual method to override by subclasses. Returns a task with the input set, ready to be executed. The parameters
       * specify the position (slice) of the needed actor and the priority of the task (NORMAL priority if omitted).
       */
      virtual CachedRepresentationTaskSPtr createTask(CacheNode *node, Priority priority = Priority::NORMAL) = 0;

      /* \brief Prints the cache window info.
       *
       * Prints the occupation of the cache nodes along with the memory consumption and average task execution time.
       */
      void printBufferInfo();

      /* \brief Returns true if the representation in that node needs to update at the moment
       * of calling this method.
       *
       */
      virtual bool needUpdate(CacheNode *node) = 0;

      // Protected attributes.
      unsigned int   m_windowWidth;    // Actual cache window width
      unsigned int   m_maxWindowWidth; // Maximum allowed cache window width.
      CacheNode     *m_actualPos;      // Node interpreted as the "center" of the circular buffer and actual actor on the view
      CacheNode     *m_edgePos;        // Node interpreted as a edge of the circular buffer and the one inserting/deleting
                                       // actors from the ring as the "center" moves

      vtkSmartPointer<vtkActor> m_symbolicActor; // Actor that the user sees when the task hasn't finished yet (during a cache miss).

      friend class CachedRepresentationTask;
  };

  using CachedRepresentationPtr   = CachedRepresentation *;
  using CachedRepresentationList  = QList<CachedRepresentationPtr>;
  using CachedRepresentationSPtr  = std::shared_ptr<CachedRepresentation>;
  using CachedRepresentationSList = QList<CachedRepresentationSPtr>;

} /* namespace EspINA */

#endif /* CACHEDREPRESENTATION_H_ */
