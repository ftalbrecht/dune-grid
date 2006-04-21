// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef DUNE_ALU3DUTILITY_HH
#define DUNE_ALU3DUTILITY_HH

//- Dune includes
#include <dune/common/array.hh>

//- local includes
#include "topology.hh"
#include "alu3dinclude.hh"

namespace Dune {

  template <class GeometryImp, int nChild>
  class LocalGeometryStorage
  {
    // array with pointers to the geometries
    Array < GeometryImp * > geoms_;
    // count local geometry creation
    int count_;
  public:
    // create empty storage
    LocalGeometryStorage () : geoms_ (nChild) , count_ (0) {
      for(int i=0 ; i<geoms_.size(); ++i) geoms_[i] = 0;
    }

    // desctructor deleteing geometries
    ~LocalGeometryStorage () {
      for(int i=0 ; i<geoms_.size(); ++i)
        if(geoms_[i]) delete geoms_[i];
    }

    // check if geometry has been created
    bool geomCreated(int child) const { return geoms_[child] != 0; }

    // create local geometry
    template <class GridImp>
    void create (const GridImp & grid, const GeometryImp & father,
                 const GeometryImp & son, const int child)
    {
      assert( !geomCreated(child) );
      assert( child >=0 && child < nChild );

      assert( count_ < nChild );
      ++count_;

      typedef typename GeometryImp :: ImplementationType ImplType;
      GeometryImp * g = new GeometryImp(ImplType());
      geoms_[child] = g;
      GeometryImp & geo = *g;
      grid.getRealImplementation(geo).buildGeomInFather( father, son );
    }

    // return reference to local geometry
    const GeometryImp & operator [] (int child) const
    {
      assert( geomCreated(child) );
      return *(geoms_[child]);
    }
  };

  inline const ALU3dImplTraits<tetra>::GEOFaceType*
  getFace(const ALU3DSPACE GEOTetraElementType& elem, int index) {
    assert(index >= 0 && index < 4);
    return elem.myhface3(ElementTopologyMapping<tetra>::dune2aluFace(index));
  }

  inline const ALU3dImplTraits<hexa>::GEOFaceType*
  getFace(const ALU3DSPACE GEOHexaElementType& elem, int index) {
    assert(index >= 0 && index < 6);
    return elem.myhface4(ElementTopologyMapping<hexa>::dune2aluFace(index));
  }

} // end namespace Dune
#endif
