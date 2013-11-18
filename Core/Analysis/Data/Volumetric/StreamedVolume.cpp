/*
 * Copyright (c) 2013, Jorge Peña Pastor <jpena@cesvima.upm.es>
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *     * Neither the name of the <organization> nor the
 *     names of its contributors may be used to endorse or promote products
 *     derived from this software without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY Jorge Peña Pastor <jpena@cesvima.upm.es> ''AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL Jorge Peña Pastor <jpena@cesvima.upm.es> BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * 
 */

template<typename T> using StreamReaderType  = itk::ImageFileReader<T>;
template<typename T> using StreamExtractType = itk::ExtractImageFilter<T, T>;

//-----------------------------------------------------------------------------
template<typename T>
StreamedVolume<T>::StreamedVolume()
: m_origin {0, 0, 0}
, m_spacing{1, 1, 1}
{
  setBackgroundValue(0);
}

//-----------------------------------------------------------------------------
template<typename T>
StreamedVolume<T>::StreamedVolume(const QFileInfo &fileName)
: m_fileName{fileName.absoluteFilePath()}
, m_origin {0, 0, 0}
, m_spacing{1, 1, 1}
{
  setBackgroundValue(0);
}

//-----------------------------------------------------------------------------
template<typename T>
double StreamedVolume<T>::memoryUsage() const
{
  return 0;
}


//-----------------------------------------------------------------------------
template<typename T>
Bounds StreamedVolume<T>::bounds() const
{
  if (!isValid()) throw File_Not_Found_Exception();

  auto reader = StreamReaderType<T>::New();
  reader->ReleaseDataFlagOn();
  reader->SetFileName(m_fileName.toStdString());
  reader->UpdateOutputInformation();

  typename T::Pointer image = reader->GetOutput();

  return equivalentBounds<T>(image, image->GetLargestPossibleRegion());
}

//-----------------------------------------------------------------------------
template<typename T>
void StreamedVolume<T>::setOrigin(const NmVector3& origin)
{
  m_origin = origin;
}


//-----------------------------------------------------------------------------
template<typename T>
void StreamedVolume<T>::setSpacing(const NmVector3& spacing)
{
  m_spacing = spacing;
}

//-----------------------------------------------------------------------------
template<typename T>
NmVector3 StreamedVolume<T>::spacing() const
{
  if (!isValid()) throw File_Not_Found_Exception();

  auto reader = StreamReaderType<T>::New();
  reader->ReleaseDataFlagOn();
  reader->SetFileName(m_fileName.toStdString());
  reader->UpdateOutputInformation();

  auto image = reader->GetOutput();

  NmVector3 spacing;
  for(int i = 0; i < 3; ++i)
  {
    spacing[i] = image->GetSpacing()[i];
  }

  return spacing;
}


//-----------------------------------------------------------------------------
template<typename T>
const typename T::Pointer StreamedVolume<T>::itkImage() const
{
  if (!isValid()) throw File_Not_Found_Exception();

  auto reader = StreamReaderType<T>::New();
  reader->ReleaseDataFlagOn();
  reader->SetFileName(m_fileName.toStdString());
  reader->Update();

  typename T::Pointer image = reader->GetOutput();
  image->DisconnectPipeline();

  return image;
}

//-----------------------------------------------------------------------------
template<typename T>
const typename T::Pointer StreamedVolume<T>::itkImage(const Bounds& bounds) const
{
  if (!isValid()) throw File_Not_Found_Exception();

  auto reader = StreamReaderType<T>::New();
  reader->ReleaseDataFlagOn();
  reader->SetFileName(m_fileName.toStdString());
  reader->UpdateOutputInformation();

  auto requestedRegion = equivalentRegion(reader->GetOutput(), bounds);

  auto extractor = StreamExtractType<T>::New();
  extractor->SetExtractionRegion(requestedRegion);
  extractor->SetInput(reader->GetOutput());
  extractor->Update();

  typename T::Pointer image = extractor->GetOutput();
  image->DisconnectPipeline();

  return image;
}
