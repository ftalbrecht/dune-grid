// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef DUNE_ALU3DGRIDMAPPINGS_HH
#define DUNE_ALU3DGRIDMAPPINGS_HH

// System includes
#include <limits>

// Dune includes
#include <dune/common/fvector.hh>
#include <dune/common/fmatrix.hh>

// Local includes
#include "alu3dinclude.hh"

namespace Dune {

  static const alu3d_ctype ALUnumericEpsilon = 10.0 * std::numeric_limits< alu3d_ctype >::epsilon();

  template<int mydim, int coorddim, class GridImp>
  class ALU3dGridGeometry;

  template <int dim, int dimworld, ALU3dGridElementType elType>
  class ALU3dGrid;

  //! A trilinear mapping from the Dune reference hexahedron into the physical
  //! space (same as in mapp_cube_3d.h, but for a different reference hexahedron)
  class TrilinearMapping {
    typedef FieldVector<double, 3> coord_t;
    typedef FieldMatrix<double, 3, 3> mat_t;
    static const double _epsilon ;

    // the internal mapping
    double a [8][3] ;
    mat_t Df;
    mat_t Dfi;
    mat_t invTransposed_;
    double DetDf ;

    bool calcedDet_;
    bool calcedInv_;
    bool affine_;

    void linear (const double, const double, const double) ;
    void linear (const coord_t&) ;
    void inverse (const coord_t&) ;
  public:
    TrilinearMapping (const coord_t&, const coord_t&,
                      const coord_t&, const coord_t&,
                      const coord_t&, const coord_t&,
                      const coord_t&, const coord_t&);

    // only to call from geometry class
    TrilinearMapping () {}

    TrilinearMapping (const TrilinearMapping &) ;

    ~TrilinearMapping () {}
    double det (const coord_t&) ;
    const mat_t& jacobianInverseTransposed(const coord_t&);
    void map2world (const coord_t&, coord_t&) const ;
    void map2world (const double , const double , const double ,
                    coord_t&) const ;
    void world2map (const coord_t&, coord_t&) ;

    template <class vector_t>
    void buildMapping(const vector_t&, const vector_t&,
                      const vector_t&, const vector_t&,
                      const vector_t&, const vector_t&,
                      const vector_t&, const vector_t&);

    // returns true if mapping is affine
    inline bool affine () const;
  };

  //! A bilinear surface mapping
  // NOTE: this class is different to the BilinearSurfaceMapping in
  // ALUGrid, for example the reference elements differ
  // here we have [0,1]^2 and in ALUGrid its [-1,1]^2
  // also the point numbering is different
  class SurfaceNormalCalculator
  {
  protected:
    // our coordinate types
    typedef FieldVector<double, 3> coord3_t;
    typedef FieldVector<double, 2> coord2_t;

    // type of coordinate vectors from elements
    typedef double double3_t[3];

    double _n [3][3] ;

    static const double _epsilon ;

    bool _affine;

  public:
    //! Constructor creating empty mapping with double , i.e. zero
    SurfaceNormalCalculator();

    SurfaceNormalCalculator (const SurfaceNormalCalculator &) ;
    ~SurfaceNormalCalculator () {}

    // returns true if mapping is affine
    inline bool affine () const { return _affine ; }

    // calcuates normal
    void normal(const coord2_t&, coord3_t&) const ;
    void normal(const double, const double, coord3_t&) const;

    void negativeNormal(const coord2_t&, coord3_t&) const ;
    void negativeNormal(const double, const double, coord3_t&) const;

  public:
    // builds _b and _n, called from the constructors
    // public because also used in faceutility
    template <class vector_t>
    void buildMapping (const vector_t & , const vector_t & ,
                       const vector_t & , const vector_t & );
  protected:
    // builds _b and _n, called from the constructors
    // public because also used in faceutility
    template <class vector_t>
    void buildMapping (const vector_t & , const vector_t & ,
                       const vector_t & , const vector_t & ,
                       double (&_b)[4][3] );
  } ;

  //! A bilinear surface mapping
  // NOTE: this class is different to the BilinearSurfaceMapping in
  // ALUGrid, for example the reference elements differ
  // here we have [0,1]^2 and in ALUGrid its [-1,1]^2
  // also the point numbering is different
  class BilinearSurfaceMapping : public SurfaceNormalCalculator
  {
  protected:
    typedef SurfaceNormalCalculator BaseType;

    using BaseType :: _n;
    static const double _epsilon;

    // our coordinate types
    typedef FieldVector<double, 3> coord3_t;
    typedef FieldVector<double, 2> coord2_t;

    // type of coordinate vectors from elements
    typedef double double3_t[3];

    // type for helper matrices
    typedef FieldMatrix<double,3,3> mat3_t;

    // type for inverse matrices
    typedef FieldMatrix<double,3,2> inv_t;

    double _b [4][3] ;

    mutable mat3_t Df,Dfi;
    mutable inv_t invTransposed_;
    mutable double DetDf;

    mutable coord3_t normal_;
    mutable coord3_t tmp_;

    mutable bool _calcedDet;
    mutable bool _calcedInv;
    mutable bool _calcedTransposed;

  public:
    //! Constructor creating empty mapping with double , i.e. zero
    BilinearSurfaceMapping ();

    //! Constructor getting FieldVectors
    BilinearSurfaceMapping (const coord3_t&, const coord3_t&,
                            const coord3_t&, const coord3_t&) ;
    //! Constructor for double[3]
    BilinearSurfaceMapping (const double3_t &, const double3_t &,
                            const double3_t &, const double3_t &) ;
    BilinearSurfaceMapping (const BilinearSurfaceMapping &) ;
    ~BilinearSurfaceMapping () {}

    void inverse (const coord3_t&) const;
    const inv_t& jacobianInverseTransposed(const coord2_t&) const ;

    // calculates determinant of mapping
    double det(const coord3_t&) const;

    // maps from local coordinates to global coordinates
    void world2map(const coord3_t &, coord2_t & ) const;

    // maps form global coordinates to local (within reference element)
    // coordinates
    void map2world(const coord2_t&, coord3_t&) const ;
    void map2world(const double ,const double , coord3_t&) const ;

  private:
    void map2worldnormal(const double, const double, const double , coord3_t&) const;
    void map2worldlinear(const double, const double, const double ) const;

  public:
    // builds _b and _n, called from the constructors
    // public because also used in faceutility
    template <class vector_t>
    void buildMapping (const vector_t & , const vector_t & ,
                       const vector_t & , const vector_t & );
  } ;

  //! General form of non-conforming face mapping
  //! This class is empty and needs to be specialised
  template <ALU3dGridElementType type>
  class NonConformingFaceMapping {};

  //! Non-conforming face mappings for tetrahedra
  template <>
  class NonConformingFaceMapping<tetra> {
  private:
    //- private typedefs and enums
  public:
    //- public typedefs and enums
    typedef FieldVector<alu3d_ctype, 3> CoordinateType;
    typedef ALU3DSPACE Hface3RuleType RefinementRuleType;

  public:
    //- public interface methods
    NonConformingFaceMapping(RefinementRuleType rule,
                             int nChild);
    NonConformingFaceMapping(const NonConformingFaceMapping<tetra>& orig);
    ~NonConformingFaceMapping();

    void child2parent(const CoordinateType& childCoordinates,
                      CoordinateType& parentCoordinates) const ;

  private:
    void child2parentNosplit(const CoordinateType& childCoordinates,
                             CoordinateType& parentCoordinates) const;
    void child2parentE01(const CoordinateType& childCoordinates,
                         CoordinateType& parentCoordinates) const;
    void child2parentE12(const CoordinateType& childCoordinates,
                         CoordinateType& parentCoordinates) const;
    void child2parentE20(const CoordinateType& childCoordinates,
                         CoordinateType& parentCoordinates) const;
    void child2parentIso4(const CoordinateType& childCoordinates,
                          CoordinateType& parentCoordinates) const;
  private:
    //- private data
    RefinementRuleType rule_;
    int nChild_;
  };

  //! Non-conforming face mappings for hexahedra
  template <>
  class NonConformingFaceMapping<hexa> {
  private:
    //- private typedefs and enums
  public:
    //- public typedefs and enums
    typedef FieldVector<alu3d_ctype, 2> CoordinateType;
    typedef ALU3DSPACE Hface4RuleType RefinementRuleType;
  public:
    NonConformingFaceMapping(RefinementRuleType rule, int nChild);
    NonConformingFaceMapping(const NonConformingFaceMapping<hexa>& orig);
    ~NonConformingFaceMapping();

    void child2parent(const CoordinateType& childCoordinates,
                      CoordinateType& parentCoordinates) const;
  private:
    void child2parentNosplit(const CoordinateType& childCoordinates,
                             CoordinateType& parentCoordinates) const;
    void child2parentIso4(const CoordinateType& childCoordinates,
                          CoordinateType& parentCoordinates) const;

  private:
    RefinementRuleType rule_;
    int nChild_;
  };

} // end namespace Dune

#include "mappings_imp.cc"

#endif