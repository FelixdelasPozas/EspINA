#ifndef RENDERER_H_
#define RENDERER_H_

#include <QMap>

class pqRenderView;
class EspinaProduct;
class pqPipelineSource;


enum RenderType
{ RENDERTYPE_FIRST = 0
, MESH_RENDERER    = 0
, VOLUME_RENDERER  = 1
, RENDERTYPE_LAST  = 1
};

//! Interface to render EspinaProduct classes
class IRenderer
{
public:
  IRenderer(){}
  virtual ~IRenderer() {};
  
  virtual RenderType type() = 0;
  
  //virtual void hide(Segmentation *seg, pqRenderView *view) = 0;
  virtual void render(EspinaProduct *actor, pqRenderView *view) = 0;
  //virtual void renderSelection(Segmentation *seg, pqRenderView *view) = 0;
  //virtual void renderDiscarted(Segmentation *seg, pqRenderView *view) = 0;
};

//! A Mesh Renderer Class
class MeshRenderer : public IRenderer
{
public:
  MeshRenderer() : IRenderer(){}
  ~MeshRenderer(){}
  
  static IRenderer *renderer();
  
  RenderType type() {return MESH_RENDERER;}
  
  //void hide(Segmentation *seg, pqRenderView *view);
  void render(EspinaProduct *actor, pqRenderView *view);
  //void renderSelection(Segmentation *seg, pqRenderView *view);
  //void renderDiscarted(Segmentation *seg, pqRenderView *view);
  
private:
  static MeshRenderer *m_singleton;
  static QMap<EspinaProduct *,pqPipelineSource *> m_contours;
  
};

//! A Volume Renderer Class
class VolumetricRenderer : public IRenderer
{
public:
  VolumetricRenderer() : IRenderer(){}
  ~VolumetricRenderer(){}
  
  static IRenderer *renderer();
  
  RenderType type() {return VOLUME_RENDERER;}
  
  //void hide(Segmentation *seg, pqRenderView *view);
  void render(EspinaProduct *actor, pqRenderView *view);
  //void renderSelection(Segmentation *seg, pqRenderView *view);
  //void renderDiscarted(Segmentation *seg, pqRenderView *view);
  
private:
  static VolumetricRenderer *m_singleton;
};


#endif// RENDERER_H_