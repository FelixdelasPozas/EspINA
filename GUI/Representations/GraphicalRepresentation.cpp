/*
    Copyright (c) 2013, Jorge Peña Pastor <jpena@cesvima.upm.es>
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:
        * Redistributions of source code must retain the above copyright
        notice, this list of conditions and the following disclaimer.
        * Redistributions in binary form must reproduce the above copyright
        notice, this list of conditions and the following disclaimer in the
        documentation and/or other materials provided with the distribution.
        * Neither the name of the <organization> nor the
        names of its contributors may be used to endorse or promote products
        derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY Jorge Peña Pastor <jpena@cesvima.upm.es> ''AS IS'' AND ANY
    EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL Jorge Peña Pastor <jpena@cesvima.upm.es> BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
    ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/


#include "GraphicalRepresentation.h"
#include <GUI/QtWidget/SliceView.h>

using namespace EspINA;

//-----------------------------------------------------------------------------
GraphicalRepresentation::GraphicalRepresentation(EspinaRenderView *view) 
: m_color(Qt::white)
, m_highlight(false)
, m_visible(true)
, m_view(view)
, m_active(false)
{}

//-----------------------------------------------------------------------------
ChannelGraphicalRepresentation::ChannelGraphicalRepresentation(EspinaRenderView *view)
: GraphicalRepresentation(view)
, m_brightness(0)
, m_contrast(1)
, m_opacity(1)
{
}

//-----------------------------------------------------------------------------
void GraphicalRepresentation::setActive(bool value, EspinaRenderView *view)
{
  if (!view || view == m_view)
    m_active = value;

  foreach (GraphicalRepresentationSPtr clone, m_clones)
  {
    clone->setActive(value, view);
  }
}

//-----------------------------------------------------------------------------
vtkMatrix4x4 *GraphicalRepresentation::slicingMatrix(SliceView *view) const
{
  return view->m_slicingMatrix;
}
namespace EspINA
{
}
