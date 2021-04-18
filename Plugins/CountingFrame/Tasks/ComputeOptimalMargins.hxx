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


#ifndef ESPINA_COMPUTE_OPTIMAL_MARGINS_H
#define ESPINA_COMPUTE_OPTIMAL_MARGINS_H

// ESPINA
#include <Core/MultiTasking/Task.h>
#include <Core/Utils/Bounds.h>
#include <Extensions/EdgeDistances/EdgeDistance.h>
#include <Extensions/ExtensionUtils.h>
#include <GUI/ModelFactory.h>
#include <GUI/Model/SegmentationAdapter.h>

// VTK
#include <vtkMath.h>

using ESPINA::Extensions::retrieveExtension;

namespace ESPINA
{
  /** \class ComputeOptimalMargins
   * \brief Task that modifies the counting frame margins making it optimal for the given segmentations.
   *
   */
  template <typename C, typename S>
  class ComputeOptimalMargins
  : public Task
  {
    public:
      /** \brief ComputeOptimalMargins class constructor.
       * \param[in] stack stack object.
       * \param[in] segmentations segmentations list.
       * \param[in] factory application factory.
       * \param[in] scheduler application task scheduler.
       */
      explicit ComputeOptimalMargins(C stack,
                                     S segmentations,
                                     ModelFactory *factory,
                                     SchedulerSPtr scheduler = SchedulerSPtr());

      /** \brief ComputeOptimalMargins class virtual destructor.
       *
       */
      virtual ~ComputeOptimalMargins()
      {};

      /** \brief Returns the stack of the counting frame.
       *
       */
      C stack() const
      { return m_stack;}

      /** \brief Returns the inclusion margins of the counting frame.
       *
       */
      void inclusion(Nm value[3]) const
      { memcpy(value, m_inclusion, 3*sizeof(Nm)); }

      /** \brief Returns the exclusion margins of the counting frame.
       *
       */
      void exclusion(Nm value[3]) const
      { memcpy(value, m_exclusion, 3*sizeof(Nm)); }

      /** \brief Returns the names of the segmentations guilty of inclusion margins, or empty if none.
       *         The returned values correspond to Axis::X, Axis::Y, Axis::Z.
       */
      const QStringList guiltySegmentations() const
      { return m_guilty; }

    protected:
      virtual void run();

    private:
      C             m_stack;         /** stack of the counting frame.      */
      S             m_segmentations; /** list of segmentations.            */
      ModelFactory* m_factory;       /** application object factory.       */
      Nm            m_inclusion[3];  /** counting frame inclusion margins. */
      Nm            m_exclusion[3];  /** counting frame exclusion margins. */
      QStringList   m_guilty;        /** guilty segmentations names.       */
  };

  //------------------------------------------------------------------------
  template<typename C, typename S>
  ComputeOptimalMargins<C, S>::ComputeOptimalMargins(C stack, S segmentations, ModelFactory *factory, SchedulerSPtr scheduler)
  : Task           {scheduler}
  , m_stack        {stack}
  , m_segmentations{segmentations}
  , m_factory      {factory}
  , m_guilty       {QString(), QString(), QString()}
  {
    setDescription("Computing Optimal Margins");
    Q_ASSERT(factory);

    memset(m_inclusion, 0, 3 * sizeof(Nm));
    memset(m_exclusion, 0, 3 * sizeof(Nm));
  }

  //------------------------------------------------------------------------
  template<typename C, typename S>
  void ComputeOptimalMargins<C, S>::run()
  {
    auto spacing = m_stack->output()->spacing();

    memset(m_inclusion, 0, 3 * sizeof(Nm));
    memset(m_exclusion, 0, 3 * sizeof(Nm));

    const NmVector3 DELTA{ 0.5 * spacing[0], 0.5 * spacing[1], 0.5 * spacing[2] };

    double taskProgress = 0;
    double inc = 100.0 / m_segmentations.size();

    for (auto segmentation : m_segmentations)
    {
      if (!canExecute()) break;

      Nm dist2Margin[6];

      auto edgesExtension = retrieveOrCreateSegmentationExtension<Extensions::EdgeDistance>(segmentation, m_factory);
      edgesExtension->edgeDistance(dist2Margin);

      auto segBounds = segmentation->output()->bounds();
      auto segSpacing = segmentation->output()->spacing();

      for (int i = 0; i < 3; i++)
      {
        Nm shift = i < 2 ? 0.5 : -0.5;
        Nm length = segBounds.lenght(toAxis(i));

        if (dist2Margin[2 * i] < DELTA[i])
        {
          auto value = (vtkMath::Round(std::max(length, m_inclusion[i]) / segSpacing[i] - shift) + shift) * segSpacing[i];

          if(value != m_inclusion[i])
          {
            m_inclusion[i] = value;
            m_guilty[i] = segmentation->alias().isEmpty() ? segmentation->name() : segmentation->alias();
          }
        }

//        if (dist2Margin[2*i+1] < DELTA[i])
//        {
//          exclusion[i] = std::max(length, m_exclusion[i]);
//        }
      }
      taskProgress += inc;

      reportProgress((int) taskProgress);
    }
  }

}// namespace ESPINA

#endif // ESPINA_COMPUTE_OPTIMAL_MARGINS_H
