// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef DUNE_ALBERTAGRIDINDEXSETS_HH
#define DUNE_ALBERTAGRIDINDEXSETS_HH

#if HAVE_ALBERTA

#include <dune/common/stdstreams.hh>

#include <dune/grid/common/grid.hh>
#include <dune/grid/common/indexidset.hh>
#include <dune/grid/common/indexstack.hh>

#include <dune/grid/albertagrid/albertaheader.hh>
#include <dune/grid/albertagrid/misc.hh>
#include <dune/grid/albertagrid/referencetopo.hh>
#include <dune/grid/albertagrid/dofadmin.hh>
#include <dune/grid/albertagrid/dofvector.hh>
#include <dune/grid/albertagrid/elementinfo.hh>

namespace Dune
{

  // External Forward Declarations
  // -----------------------------

  template< int dim, int dimworld >
  class AlbertaGrid;

  template< int codim, int dim, class GridImp >
  class AlbertaGridEntity;



  namespace Alberta
  {
    typedef Dune::IndexStack< int, 100000 > IndexStack;

    static IndexStack *currentIndexStack = 0;
  }



  //! HierarchicIndexSet uses LeafIterator types for all codims and partition types
  template <class GridImp>
  struct AlbertaGridHierarchicIteratorTypes
  {
    //! The types of the iterator
    template<int cd>
    struct Codim
    {
      template<PartitionIteratorType pitype>
      struct Partition
      {
        /*
           We use the remove_const to extract the Type from the mutable class,
           because the const class is not instantiated yet.
         */
        typedef typename remove_const<GridImp>::type::Traits::template Codim<cd>::template Partition<pitype>::LeafIterator Iterator;
      };
    };
  };



  // AlbertaGridHierarchicIndexSet
  // -----------------------------

  template< int dim, int dimworld >
  class AlbertaGridHierarchicIndexSet
    : public IndexSetDefaultImplementation
      < AlbertaGrid< dim, dimworld >,
          AlbertaGridHierarchicIndexSet< dim,dimworld >,
          AlbertaGridHierarchicIteratorTypes< AlbertaGrid< dim, dimworld > > >
  {
    typedef AlbertaGridHierarchicIndexSet< dim, dimworld > This;

    friend class AlbertaGrid< dim, dimworld >;

    typedef AlbertaGrid< dim, dimworld > Grid;

    typedef typename Grid::Traits Traits;

    static const int dimension = Grid::dimension;

    typedef Alberta::DofVectorPointer< int > IndexVectorPointer;

    template< int codim >
    class DofAccess;

    class InitEntityNumber;

    template< int codim >
    struct CreateEntityNumbers;

    template< int codim >
    class RefineNumbering;

    template< int codim >
    class CoarsenNumbering;

    explicit AlbertaGridHierarchicIndexSet ( const Grid &grid );

  public:
    typedef Alberta::IndexStack IndexStack;

    //! return true if entity is contained in set
    template< class Entity >
    bool contains ( const Entity & ) const
    {
      return true;
    }

    //! return index of entity
    template< class Entity >
    int index ( const Entity &entity ) const
    {
      const int codim = Entity::codimension;
      return index< codim >( entity );
    }

    //! return hierarchic index of given entity
    template< int codim >
    int index ( const typename Grid::Traits::template Codim< codim >::Entity &entity ) const
    {
      const AlbertaGridEntity< codim, dim, const Grid > &entityImp
        = Grid::getRealImplementation( entity );
      return subIndex< codim >( entityImp.elementInfo().el(), entityImp.subEntity() );
    }

    //! return subIndex of given enitiy's sub entity
    template< int codim >
    int subIndex ( const typename Traits::template Codim< 0 >::Entity &entity, int i ) const
    {
      const AlbertaGridEntity< 0, dim, const Grid > &entityImp
        = Grid::getRealImplementation( entity );
      const int j = entityImp.grid().dune2alberta( codim, i );
      return subIndex< codim >( entityImp.elementInfo().el(), j );
    }

    //! return size of set for given GeometryType
    int size ( GeometryType type ) const
    {
      return (type.isSimplex() ? size( dimension - type.dim() ) : 0);
    }

    //! return size of set
    int size ( int codim ) const
    {
      assert( (codim >= 0) && (codim <= dimension) );
      return indexStack_[ codim ].size();
    }

    //! return geometry types this set has indices for
    const std::vector< GeometryType > &geomTypes( int codim ) const
    {
      assert( (codim >= 0) && (codim <= dimension) );
      return geomTypes_[ codim ];
    }

#ifdef INDEXSET_HAS_ITERATORS
    /** @brief Iterator to one past the last entity of given codim for partition type
     */
    template<int cd, PartitionIteratorType pitype>
    typename AlbertaGridHierarchicIteratorTypes<Grid>::template Codim<cd>::
    template Partition<pitype>::Iterator end () const
    {
      return grid_.template leafend<cd,pitype> ();
    }

    /** @brief Iterator to first entity of given codimension and partition type.
     */
    template<int cd, PartitionIteratorType pitype>
    typename AlbertaGridHierarchicIteratorTypes<Grid>::template Codim<cd>::
    template Partition<pitype>::Iterator begin () const
    {
      return grid_.template leafbegin<cd,pitype> ();
    }
#endif

    template< int codim >
    int subIndex ( const Alberta::ElementInfo< dimension > &elementInfo, int i ) const
    {
      assert( !elementInfo == false );
      return subIndex< codim >( elementInfo.el(), i );
    }

    /** \brief obtain hierarchic subindex
     *
     *  \param[in]  element  pointer to ALBERTA element
     *  \param[in]  i        number of the subelement (in ALBERTA numbering)
     */
    template< int codim >
    int subIndex ( const Alberta::Element *element, int i ) const
    {
      Int2Type< codim > codimVariable;

      int *array = (int *)entityNumbers_[ codim ];
      const int subIndex = array[ dofAccess_[ codimVariable ]( element, i ) ];
      assert( (subIndex >= 0) && (subIndex < size( codim )) );
      return subIndex;
    }

    void preAdapt ()
    {
      // set global pointer to index stack
      if( !IndexVectorPointer::supportsAdaptationData )
      {
        assert( Alberta::currentIndexStack == 0 );
        Alberta::currentIndexStack = indexStack_;
      }
    }

    void postAdapt ()
    {
      // remove global pointer to index stack
      if( !IndexVectorPointer::supportsAdaptationData )
        Alberta::currentIndexStack = 0;
    }

    void create ( const Alberta::HierarchyDofNumbering< dimension > &dofNumbering )
    {
      Alberta::ForLoop< CreateEntityNumbers, 0, dimension >::apply( dofNumbering, *this );
    }

    void read ( const std::string &filename,
                const Alberta::MeshPointer< dimension > &mesh )
    {
      Alberta::ForLoop< CreateEntityNumbers, 0, dimension >::apply( filename, mesh, *this );
    }

    bool write ( const std::string &filename ) const
    {
      bool success = true;
      for( int i = 0; i <= dimension; ++i )
      {
        std::ostringstream s;
        s << filename << ".cd" << i;
        success &= entityNumbers_[ i ].write( s.str() );
      }
      return success;
    }

    void release ()
    {
      for( int i = 0; i <= dimension; ++i )
        entityNumbers_[ i ].release();
    }

  private:
    template< int codim >
    static IndexStack &getIndexStack ( const IndexVectorPointer &dofVector )
    {
      IndexStack *indexStack;
      if( IndexVectorPointer::supportsAdaptationData )
        indexStack = dofVector.template getAdaptationData< IndexStack >();
      else
        indexStack = &Alberta::currentIndexStack[ codim ];
      assert( indexStack != 0 );
      return *indexStack;
    }

  private:
#ifdef INDEXSET_HAS_ITERATORS
    // the grid this index set belongs to
    const Grid &grid_;
#endif

    // index stacks providing new numbers during adaptation
    IndexStack indexStack_[ dimension+1 ];

    // dof vectors storing the (persistent) numbering
    IndexVectorPointer entityNumbers_[ dimension+1 ];

    // access to the dof vectors
    Alberta::CodimTable< DofAccess, dimension > dofAccess_;

    // all geometry types contained in the grid
    std::vector< GeometryType > geomTypes_[ dimension+1 ];
  };



  template< int dim, int dimworld >
  inline AlbertaGridHierarchicIndexSet< dim, dimworld >
  ::AlbertaGridHierarchicIndexSet ( const Grid &grid )
#ifdef INDEXSET_HAS_ITERATORS
    : grid_( grid )
#endif
  {
    for( int codim = 0; codim <= dimension; ++codim )
    {
      const GeometryType type( GeometryType::simplex, dimension - codim );
      geomTypes_[ codim ].push_back( type );
    }
  }



  // AlbertaGridHierarchicIndexSet::DofAccess
  // ----------------------------------------

  template< int dim, int dimworld >
  template< int codim >
  class AlbertaGridHierarchicIndexSet< dim, dimworld >::DofAccess
    : public Alberta::DofAccess< dim, codim >
  {
    typedef Alberta::DofAccess< dim, codim > Base;

  public:
    DofAccess ()
    {}

    explicit DofAccess ( const Alberta::DofSpace *dofSpace )
      : Base( dofSpace )
    {}
  };



  // AlbertaGridHierarchicIndexSet::InitEntityNumber
  // -----------------------------------------------

  template< int dim, int dimworld >
  class AlbertaGridHierarchicIndexSet< dim, dimworld >::InitEntityNumber
  {
    IndexStack &indexStack_;

  public:
    InitEntityNumber ( IndexStack &indexStack )
      : indexStack_( indexStack )
    {}

    void operator() ( int &dof )
    {
      dof = indexStack_.getIndex();
    }
  };



  // AlbertaGridHierarchicIndexSet::CreateEntityNumbers
  // --------------------------------------------------

  template< int dim, int dimworld >
  template< int codim >
  struct AlbertaGridHierarchicIndexSet< dim, dimworld >::CreateEntityNumbers
  {
    static void setup ( AlbertaGridHierarchicIndexSet< dim, dimworld > &indexSet )
    {
      IndexVectorPointer &entityNumbers = indexSet.entityNumbers_[ codim ];

      Int2Type< codim > codimVariable;
      indexSet.dofAccess_[ codimVariable ] = DofAccess< codim >( entityNumbers.dofSpace() );

      entityNumbers.template setupInterpolation< RefineNumbering< codim > >();
      entityNumbers.template setupRestriction< CoarsenNumbering< codim > >();
      entityNumbers.setAdaptationData( &(indexSet.indexStack_[ codim ]) );
    }

    static void apply ( const Alberta::HierarchyDofNumbering< dimension > &dofNumbering,
                        AlbertaGridHierarchicIndexSet< dim, dimworld > &indexSet )
    {
      const Alberta::DofSpace *dofSpace = dofNumbering.dofSpace( codim );

      std::ostringstream s;
      s << "Numbering for codimension " << codim;
      indexSet.entityNumbers_[ codim ].create( dofSpace, s.str() );

      InitEntityNumber init( indexSet.indexStack_[ codim ] );
      indexSet.entityNumbers_[ codim ].forEach( init );

      setup( indexSet );
    }

    static void apply ( const std::string &filename,
                        const Alberta::MeshPointer< dimension > &mesh,
                        AlbertaGridHierarchicIndexSet< dim, dimworld > &indexSet )
    {
      std::ostringstream s;
      s << filename << ".cd" << codim;
      indexSet.entityNumbers_[ codim ].read( s.str(), mesh );

      const int maxIndex = max( indexSet.entityNumbers_[ codim ] );
      indexSet.indexStack_[ codim ].setMaxIndex( maxIndex + 1 );

      setup( indexSet );
    }
  };



  // AlbertaGridHierarchicIndexSet::RefineNumbering
  // ----------------------------------------------

  template< int dim, int dimworld >
  template< int codim >
  struct AlbertaGridHierarchicIndexSet< dim, dimworld >::RefineNumbering
  {
    static const int dimension = dim;
    static const int codimension = codim;

  private:
    typedef Alberta::Patch< dimension > Patch;
    typedef Alberta::DofAccess< dimension, codimension > DofAccess;

    IndexStack &indexStack_;
    IndexVectorPointer dofVector_;
    DofAccess dofAccess_;

    explicit RefineNumbering ( const IndexVectorPointer &dofVector )
      : indexStack_( getIndexStack< codimension >( dofVector ) ),
        dofVector_( dofVector ),
        dofAccess_( dofVector.dofSpace() )
    {}

  public:
    void operator() ( const Alberta::Element *child, int subEntity )
    {
      int *const array = (int *)dofVector_;
      const int dof = dofAccess_( child, subEntity );
      array[ dof ] = indexStack_.getIndex();
    }

    static void interpolateVector ( const IndexVectorPointer &dofVector,
                                    const Patch &patch )
    {
      RefineNumbering refineNumbering( dofVector );
      patch.forEachInteriorSubChild( refineNumbering );
    }
  };



  // AlbertaGridHierarchicIndexSet::CoarsenNumbering
  // -----------------------------------------------

  template< int dim, int dimworld >
  template< int codim >
  struct AlbertaGridHierarchicIndexSet< dim, dimworld >::CoarsenNumbering
  {
    static const int dimension = dim;
    static const int codimension = codim;

  private:
    typedef Alberta::Patch< dimension > Patch;
    typedef Alberta::DofAccess< dimension, codimension > DofAccess;

    IndexStack &indexStack_;
    IndexVectorPointer dofVector_;
    DofAccess dofAccess_;

    explicit CoarsenNumbering ( const IndexVectorPointer &dofVector )
      : indexStack_( getIndexStack< codimension >( dofVector ) ),
        dofVector_( dofVector ),
        dofAccess_( dofVector.dofSpace() )
    {}

  public:
    void operator() ( const Alberta::Element *child, int subEntity )
    {
      int *const array = (int *)dofVector_;
      const int dof = dofAccess_( child, subEntity );
      indexStack_.freeIndex( array[ dof ] );
    }

    static void restrictVector ( const IndexVectorPointer &dofVector,
                                 const Patch &patch )
    {
      CoarsenNumbering coarsenNumbering( dofVector );
      patch.forEachInteriorSubChild( coarsenNumbering );
    }
  };



  // AlbertaGridIdSet
  // ----------------

  //! hierarchic index set of AlbertaGrid
  template< int dim, int dimworld >
  class AlbertaGridIdSet
    : public IdSetDefaultImplementation
      < AlbertaGrid< dim, dimworld >, AlbertaGridIdSet< dim, dimworld >, unsigned int >
  {
    typedef AlbertaGridIdSet< dim, dimworld > This;
    typedef AlbertaGrid< dim, dimworld > Grid;
    typedef IdSetDefaultImplementation< Grid, This, unsigned int > Base;

    friend class AlbertaGrid< dim, dimworld >;

    static const int codimShift = 30;
    static const int maxCodimSize = (1 << codimShift);

    typedef typename Grid::HierarchicIndexSet HierarchicIndexSet;

    const HierarchicIndexSet &hIndexSet_;

    //! create id set, only allowed for AlbertaGrid
    AlbertaGridIdSet ( const HierarchicIndexSet &hIndexSet )
      : hIndexSet_( hIndexSet )
    {}

  public:
    //! export type of id
    typedef typename Base::IdType IdType;

    /** \copydoc IdSet::id(const EntityType &e) const */
    template< class Entity >
    IdType id ( const Entity &e ) const
    {
      const int codim = Entity::codimension;
      return id< codim >( e );
    }

    /** \copydoc IdSet::id(const typename remove_const<GridImp>::type::Traits::template Codim<cc>::Entity &e) const */
    template< int codim >
    IdType id ( const typename Grid::template Codim< codim >::Entity &e ) const
    {
      assert( hIndexSet_.size( codim ) < maxCodimSize );
      const IdType index = hIndexSet_.index( e );
      return ((IdType)codim << codimShift) + index;
    }

    /** \copydoc IdSet::subId(const typename remove_const<GridImp>::type::Traits::template Codim<0>::Entity &e,int i) const */
    template< int codim >
    IdType subId ( const typename Grid::template Codim< 0 >::Entity &e, int i ) const
    {
      assert( hIndexSet_.size( codim ) < maxCodimSize );
      const IdType index = hIndexSet_.template subIndex< codim >( e, i );
      return ((IdType)codim << codimShift) + index;
    }
  };

} // namespace Dune

#endif // HAVE_ALBERTA

#endif