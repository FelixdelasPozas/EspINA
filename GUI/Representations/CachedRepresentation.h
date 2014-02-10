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

      /* \brief Returns the average nanoseconds that it takes to generate an actor.
       *
       * Avegare time needed to finish a task, that is, average time to generate an actor.
       */
      virtual double getAverageTaskTime();

      /* \brief Set position of the cache.
       *
       * Sets the position of the cache. If the position is not cached the cache is emptied and
       * filled again. If the position is cached the cache position shifts to that place and the
       * borders of the window deletes/creates task and actors conveniently.
       */
      virtual void setPosition(int position);

    protected slots:
      void addActor();

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
          CachedRepresentationTask      *worker;
          vtkSmartPointer<vtkImageActor> actor;
          unsigned long long             creationTime;
          CacheNode                     *next;
          CacheNode                     *previous;

          CacheNode(): position{0}, worker{nullptr}, actor{nullptr}, creationTime{0}, next{nullptr}, previous{nullptr} {};
      };

      /* \brief Struct to store task execution times.
       *
       * Struct to store task execution times. The struct accumulates the time of execution of the tasks.
       */
      struct CacheTime
      {
          unsigned long long usec;
          unsigned long long taskNum;

          CacheTime() : usec(0), taskNum(0) {};
      };

      /* \brief Deletes a task from a node.
       *
       * Deletes a task from a node. The parameter can be nullptr. If the task has not finished then it enters the list of
       * tasks to be deleted once they finish or are aborted.
       */
      void deleteTask(CacheNode *node);

      /* \brief Virtual method to override by subclasses.
       *
       * Virtual method to override by subclasses. Returns a task with the input set, ready to be executed. The parameters
       * specify the position (slice) of the needed actor and the priority of the task (NORMAL priority if omitted).
       */
      virtual CachedRepresentationTask *createTask(int position, Priority priority = Priority::NORMAL) = 0;

      /* \brief Prints the cache window info.
       *
       * Prints the occupation of the cache nodes along with the memory consumption and average task execution time.
       */
      void printBufferInfo();

      /* \brief Checks the status of every task in the deferred task list and removed the finished or aborted.
       *
       */
      void checkDeferredTaskList();


      // Protected attributes.
      unsigned int   m_windowWidth;    // Actual cache window width
      unsigned int   m_maxWindowWidth; // Maximum allowed cache window width.
      CacheNode     *m_actualPos;      // Node interpreted as the "center" of the circular buffer and actual actor on the view
      CacheNode     *m_edgePos;        // Node interpreted as a edge of the circular buffer and the one inserting/deleting
                                       // actors from the ring as the "center" moves
      CacheTime      m_time;           // Time storage.

      QList<CachedRepresentationTask *>  m_deferredDeletionList;  // List of tasks to be deleted once they finish or are aborted.

      vtkSmartPointer<vtkAssembly> m_actor; // Global actor. Generated actors are inserted into this one or removed from it.

      vtkSmartPointer<vtkActor> m_symbolicActor; // Actor that the user sees when the task hasn't finished yet (during a cache miss).
  };

  using CachedRepresentationPtr   = CachedRepresentation *;
  using CachedRepresentationList  = QList<CachedRepresentationPtr>;
  using CachedRepresentationSPtr  = std::shared_ptr<CachedRepresentation>;
  using CachedRepresentationSList = QList<CachedRepresentationSPtr>;

} /* namespace EspINA */

#endif /* CACHEDREPRESENTATION_H_ */
