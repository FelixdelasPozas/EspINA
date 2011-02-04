#ifndef RENDERER_H_
#define RENDERER_H_

class pqRenderView;
class IRenderable;

enum RenderType
{ RENDERTYPE_FIRST = 0
, MESH_RENDERER    = 0
, VOLUME_RENDERER  = 1
, RENDERTYPE_LAST  = 1
};

//! Interface for renders
class IRenderer
{
public:
  IRenderer(){}
  virtual ~IRenderer() {};
  
  virtual RenderType type() = 0;
  
  //virtual void hide(Segmentation *seg, pqRenderView *view) = 0;
  virtual void render(IRenderable *actor, pqRenderView *view) = 0;
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
  void render(IRenderable *actor, pqRenderView *view);
  //void renderSelection(Segmentation *seg, pqRenderView *view);
  //void renderDiscarted(Segmentation *seg, pqRenderView *view);
  
private:
  static MeshRenderer *m_singleton;
  
};

//! A Volume Renderer Class
class VolumeRenderer : public IRenderer
{
public:
  VolumeRenderer() : IRenderer(){}
  ~VolumeRenderer(){}
  
  static IRenderer *renderer();
  
  RenderType type() {return VOLUME_RENDERER;}
  
  //void hide(Segmentation *seg, pqRenderView *view);
  void render(IRenderable *actor, pqRenderView *view);
  //void renderSelection(Segmentation *seg, pqRenderView *view);
  //void renderDiscarted(Segmentation *seg, pqRenderView *view);
  
private:
  static VolumeRenderer *m_singleton;
};


#endif// RENDERER_H_