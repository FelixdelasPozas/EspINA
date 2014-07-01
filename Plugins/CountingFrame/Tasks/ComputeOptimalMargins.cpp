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

// EspINA
namespace EspINA
{
  //------------------------------------------------------------------------
  template <typename C, typename S>
  ComputeOptimalMargins<C, S>::ComputeOptimalMargins(C channel,
                                                  S segmentations,
                                                  SchedulerSPtr            scheduler)
  : Task(scheduler)
  , m_channel(channel)
  , m_segmentations(segmentations)
  {
    setDescription("Computing Optimal Margins");

    memset(m_inclusion, 0, 3*sizeof(Nm));
    memset(m_exclusion, 0, 3*sizeof(Nm));
  }

  //------------------------------------------------------------------------
  template <typename C, typename S>
  ComputeOptimalMargins<C, S>::~ComputeOptimalMargins()
  {
  }

  //------------------------------------------------------------------------
  template <typename C, typename S>
  void ComputeOptimalMargins<C, S>::run()
  {
    auto spacing = m_channel->output()->spacing();

    memset(m_inclusion, 0, 3*sizeof(Nm));
    memset(m_exclusion, 0, 3*sizeof(Nm));

    const NmVector3 delta{ 0.5*spacing[0], 0.5*spacing[1], 0.5*spacing[2] };

    double taskProgress = 0;
    double inc = 100.0 / m_segmentations.size();

    for (auto segmentation : m_segmentations)
    {
      if (!canExecute()) break;

      auto extension = retrieveOrCreateExtension<EdgeDistance>(segmentation);

      Nm dist2Margin[6];
      extension->edgeDistance(dist2Margin);

      auto bounds  = segmentation->output()->bounds();
      auto spacing = segmentation->output()->spacing();

      for (int i=0; i < 3; i++)
      {
        Nm shift  = i < 2? 0.5:-0.5;
        Nm length = bounds.lenght(toAxis(i));

        if (dist2Margin[2*i] < delta[i])
          m_inclusion[i] = (vtkMath::Round(std::max(length, m_inclusion[i])/spacing[i]-shift)+shift)*spacing[i];
        //         if (dist2Margin[2*i+1] < delta[i])
        //           exclusion[i] = std::max(length, exclusion[i]);
      }
      taskProgress += inc;

      emit progress((int)taskProgress);
    }
  }
}