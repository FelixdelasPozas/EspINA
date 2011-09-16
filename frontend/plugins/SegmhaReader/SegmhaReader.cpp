/*=========================================================================

   Program: ParaView
   Module:    SegmentationToolbarActions.cxx

   Copyright (c) 2005-2008 Sandia Corporation, Kitware Inc.
   All rights reserved.

   ParaView is a free software; you can redistribute it and/or modify it
   under the terms of the ParaView license version 1.2. 

   See License_v1.2.txt for the full ParaView license.
   A copy of this license can be obtained by contacting
   Kitware Inc.
   28 Corporate Drive
   Clifton Park, NY 12065
   USA

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHORS OR
CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

=========================================================================*/
#include "SegmhaReader.h"

// Debug
#include "espina_debug.h"

// EspINA
#include "espina.h"
#include "sample.h"
#include "SegmhaImporterFilter.h"

#include <espINAFactory.h>


#include <QBitmap>
#include <EspinaPluginManager.h>

#define SFR "SegmhaReader"
#define SFRF "SegmhaReader::SegmhaReaderFilter"

//-----------------------------------------------------------------------------
SegmhaReader::SegmhaReader(QObject* parent)
{
  m_factoryName = SFR;
}

void SegmhaReader::onStartup()
{
  // Register Factory's filters
  QString filter("%1::%2");
  filter.arg(m_factoryName);
  filter.arg(SegmhaImporterFilter::ID);
//   ProcessingTrace::instance()->registerPlugin(filter, this);
  EspinaPluginManager::instance()->registerFilter(filter,this);
}



//-----------------------------------------------------------------------------
EspinaFilter *SegmhaReader::createFilter(QString filter, ITraceNode::Arguments & args)
{
  if (filter == SFRF)
  {
//     SegmhaReaderFilter *sgs_sgsf = new SegmhaReaderFilter(args);
//     return sgs_sgsf;
    assert(false);
    return NULL;
  }
  qWarning("::createFilter: Error no such a Filter");
  return NULL;
}