// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#include <iostream>
#include <fstream>
#include <vector>
#include <config.h>


#include <dune/common/fvector.hh>
#include <dune/grid/common/gridview.hh>
#include <dune/grid/common/datahandleif.hh>

using namespace Dune;

/*  Communication Test for Parallel Grids
 *  -------------------------------------
 *
 *  For a fixed codimension c and a fixed upwind direction u, the test works
 *  as follows:
 *    1) In the center of all upwind codim c subentities of the interioir codim 0
 *       leaf entites a function is stored. Also a flag is set to 1.
 *       The computation is also performed on the subentities of the physical
 *       boundary.
 *    -> For all leaf subentities of codim c the flag should be set to 1
 *       with the exception of the border subentities on the inflow
 *       processor boundary and in the ghost elements - on these
 *       subentities the flag is zero.
 *    2) Exchange both the data and the flags.
 *    3) Test if the flag for all leaf subentities of codim c is set to 1.
 *
 *  Note: This test requires the normals on both sides of an intersection to
 *        sum up to zero, i.e., there is exactly one tangent plane to the grid
 *        in every point of the intersection (actually, the barycenter would be
 *        sufficient).
 */


/*****
   The exchange is done using the ExampleDataHandle given below.
   Together with the function value and the flag the coordinates
   of all corners of the subenties are transmitted, giving
   the possibility for additional testing in the scatter/set
   methods.
 ******/
/*******************************************************************/
namespace {
  template <class GridType,int c> struct NextCodim {
    static const bool next = Dune::Capabilities::hasEntity<GridType, c-1>::v;
    static const int calc = ((next) ? c-1 : NextCodim<GridType,c-1>::calc);
  };
  template <class GridType> struct NextCodim<GridType,0> {
    static const int calc = -1;
  };
}


template <class IndexSetImp,
    class GlobalIdSetImp,
    class DataVectorType >
class ExampleDataHandle
  : public CommDataHandleIF <
        ExampleDataHandle<IndexSetImp,GlobalIdSetImp,DataVectorType> ,
        typename DataVectorType :: value_type >
{
  const IndexSetImp & iset_;
  const GlobalIdSetImp & ids_;
  int cdim_;
  DataVectorType & data1_;
  DataVectorType & data2_;
public:
  typedef typename DataVectorType :: value_type DataType;
  ExampleDataHandle(const IndexSetImp & iset,
                    const GlobalIdSetImp & ids,
                    int cdim,
                    DataVectorType & d1, DataVectorType & d2) :
    iset_(iset), ids_(ids) , cdim_(cdim), data1_(d1) , data2_(d2)
  {}

  //! returns true if data for this codim should be communicated
  bool contains (int dim, int codim) const
  {
    return (codim==cdim_);
  }

  //! returns true if size per entity of given dim and codim is a constant
  bool fixedsize (int dim, int codim) const
  {
    // this problem is a fixed size problem,
    // but to simulate also non-fixed size problems
    // we set this to false, should work anyway
    return false;
  }

  /*! how many objects of type DataType have to be sent for a given entity
     Note: Only the sender side needs to know this size.
   */
  template<class EntityType>
  size_t size (EntityType& e) const
  {
    // flag+data+coordinates
    return 2+e.geometry().corners()*e.geometry().dimensionworld;
  }

  //! pack data from user to message buffer
  template<class MessageBuffer, class EntityType>
  void gather (MessageBuffer& buff, const EntityType& e) const
  {
    int idx = iset_.index(e);

    //typename GlobalIdSetImp :: IdType id = ids_.id( e );
    //buff.write( id );
    buff.write(data2_[idx]);   // flag
    buff.write(data1_[idx]);   // data

    // all corner coordinates
    typedef typename EntityType::Geometry Geometry;
    const Geometry &geometry = e.geometry();
    for( int i = 0; i < geometry.corners(); ++i )
    {
      typedef FieldVector< typename Geometry::ctype, Geometry::dimensionworld > Vector;
      const Vector corner = geometry.corner( i );
      for( int j = 0; j < Geometry::dimensionworld; ++j )
        buff.write( corner[ j ] );
    }
  }

  /*! unpack data from message buffer to user
     n is the number of objects sent by the sender
   */
  template<class MessageBuffer, class EntityType>
  void scatter (MessageBuffer& buff, const EntityType& e, size_t n)
  {
    // as this problem is a fixed size problem we can check the sizes
    assert( n == size(e) );

    // make sure that global id on all processors are the same
    // here compare the id of the entity that sended the data with my id
    typename GlobalIdSetImp :: IdType id;
    //buff.read( id );
    //typename GlobalIdSetImp :: IdType myId = ids_.id( e );
    //std::cout << id << " id | my Id = " << myId << "\n";
    //assert( id == myId );

    // else do normal scatter
    int idx = iset_.index(e);
    DataType x=0.0;
    buff.read(x); // flag

    // for ghost entities x > 0 must be true
    assert( ( e.partitionType() == GhostEntity ) ? (x>=0.0) : 1);

    if (x>=0)
    { // only overwrite existing data if flag = 1, i.e.,
      // the sending processor acctually computed the value
      data2_[idx] = x;
      x=0.;
      buff.read(x);  // correct function value
      data1_[idx] = x;
    }
    else
    {
      x=0.;
      buff.read(x);
    }

    // test if the sending/receving entities are geometrically the same
    typedef typename EntityType::Geometry Geometry;
    const Geometry &geometry = e.geometry();
    for( int i = 0; i < geometry.corners(); ++i )
    {
      typedef FieldVector< typename Geometry::ctype, Geometry::dimensionworld > Vector;
      const Vector corner = geometry.corner( i );
      for( int j = 0; j < Geometry::dimensionworld; ++j )
      {
        buff.read(x);
        if( fabs( corner[ j ] - x ) > 1e-8 )
        {
          std::cerr << "ERROR in scatter: Vertex <" << i << "," << j << ">: "
                    << " this : (" << corner[ j ] << ")"
                    << " other : (" << x << ")"
                    << std::endl;
        }
      }
    }
  }

};


/*******************************************************************/
/*******************************************************************/
template< class GridView, int cdim, class OutputStream >
class CheckCommunication
{
  typedef typename GridView :: Grid Grid;
  typedef typename GridView :: IndexSet IndexSet;
  typedef typename GridView :: IntersectionIterator IntersectionIterator;

  typedef typename IntersectionIterator :: Intersection Intersection;

  typedef typename GridView :: template Codim< 0 > :: EntityPointer EntityPointer;
  typedef typename GridView :: template Codim< 0 > :: Entity Entity;
  typedef typename GridView :: template Codim< 0 > :: Iterator Iterator;

  typedef typename GridView :: template Codim< cdim > :: EntityPointer SubEntityPointer;

  enum { dimworld = Grid :: dimensionworld };
  enum { dim = Grid :: dimension };

  typedef typename Grid :: ctype ctype;

  typedef FieldVector< ctype, dimworld > CoordinateVector;
  typedef std :: vector< double > ArrayType;

  CoordinateVector upwind_;
  OutputStream &sout_;

  const GridView &gridView_;
  const IndexSet &indexSet_;
  const int level_;

  // the function
  double f ( const CoordinateVector &x )
  {
    CoordinateVector a( 1.0 );
    a[0] = -0.5;
    return a*x+1.5; //+cos(x*x);
  }

  // compute the data on the upwind entities
  void project ( int dataSize, ArrayType &data, ArrayType &weight, int rank )
  {
    // set initial data
    for(int i=0 ; i<dataSize; ++i)
    {
      data[i] = 0.0;
      weight[i] = -1.0;
    }

    Iterator end = gridView_.template end< 0 >();
    for( Iterator it = gridView_.template begin< 0 >(); it != end ; ++it )
    {
      const Entity &entity = *it;

      if( cdim == 0 )
      {
        CoordinateVector mid( 0.0 );
        const int numVertices = entity.template count< dim >();
        for( int i = 0; i < numVertices; ++i )
          mid += entity.geometry().corner( i );
        mid /= double( numVertices );

        int index = indexSet_.index( entity );
        data[ index ]  = f( mid );
        weight[ index ] = 1.0;
      }
      else
      {
        const IntersectionIterator nend = gridView_.iend( entity );
        for( IntersectionIterator nit = gridView_.ibegin( entity ); nit != nend; ++nit )
        {
          const Intersection &intersection = *nit;

          const typename Intersection :: LocalGeometry &geoInSelf
            = intersection.intersectionSelfLocal();

          const ReferenceElement< ctype, dim-1 > &faceRefElement
            = ReferenceElements< ctype, dim-1 > :: general( geoInSelf.type() );
          const FieldVector< ctype, dim-1 > &bary = faceRefElement.position( 0, 0 );

          const CoordinateVector normal = intersection.integrationOuterNormal( bary );
          double calc = normal * upwind_;

          // if testing level, then on non-conform grid also set values on
          // intersections that are not boundary, but has no level
          // neighbor
          const bool proceedAnyway = (level_ < 0 ? false : !intersection.neighbor());
          if( (calc > -1e-8) || intersection.boundary() || proceedAnyway )
          {
            const ReferenceElement< ctype, dim > &insideRefElem
              =  ReferenceElements< ctype, dim > :: general( entity.type() );

            const int numberInSelf = intersection.numberInSelf();
            for( int i = 0; i < insideRefElem.size( numberInSelf, 1, cdim ); ++i )
            {
              int e = insideRefElem.subEntity( numberInSelf, 1, i, cdim );
              int idx = indexSet_.template subIndex< cdim >( entity, e );
              CoordinateVector cmid( 0.0 );
              SubEntityPointer subEp = entity.template entity< cdim >( e );
              int c = subEp->geometry().corners();
              for (int j=0; j<c; j++)
                cmid += subEp->geometry().corner( j );
              cmid /= double(c);

              data[idx] = f(cmid);
              weight[idx] = 1.0;
            }

            // on non-conforming grids the neighbor entities might not
            // be the same as those on *it, therefore set data on neighbor
            // as well
            if( intersection.neighbor() )
            {
              EntityPointer ep = intersection.outside();
              const Entity &neigh = *ep;

              assert( (level_ < 0) ? (neigh.isLeaf()) : 1);
              assert( (level_ < 0) ? 1 : (neigh.level() == level_) );

              const ReferenceElement< ctype, dim > &outsideRefElem
                = ReferenceElements< ctype, dim > :: general( neigh.type() );

              const int numberInNeighbor = intersection.numberInNeighbor();
              for( int i = 0; i < outsideRefElem.size(numberInNeighbor, 1, cdim); ++i )
              {
                int e = outsideRefElem.subEntity( numberInNeighbor, 1, i, cdim );
                int idx = indexSet_.template subIndex<cdim>(neigh, e);
                CoordinateVector cmid( 0.0 );
                SubEntityPointer subEp = neigh.template entity< cdim >( e );
                int c = subEp->geometry().corners();
                for (int j=0; j<c; j++)
                  cmid += subEp->geometry().corner( j );
                cmid /= double(c);

                data[idx] = f(cmid);
                weight[idx] = 1.0;
              }
            }
          }
        }
      }
    }
  }

  // test if all flags are 1 and return the
  // difference in the function values.
  // if testweight is true an error is printed for each
  // flag not equal to 1
  double test ( int dataSize, ArrayType &data, ArrayType &weight, bool testweight )
  {
    const int rank = gridView_.comm().rank();

    //Variante MIT Geisterzellen
    //typedef typename IndexSet :: template Codim<0> :: template Partition<All_Partition> :: Iterator IteratorType;

    double maxerr = 0.0;
    Iterator end = gridView_.template end< 0 >();
    for( Iterator it = gridView_.template begin< 0 >(); it != end ; ++it )
    {
      const Entity &entity = *it;

      CoordinateVector mid( 0.0 );
      const int numVertices = entity.template count< dim >();
      for( int i = 0; i < numVertices; ++i )
        mid += entity.geometry().corner( i );
      mid /= double(numVertices);

      if( cdim == 0 )
      {
        int index = indexSet_.index( entity );
        double lerr = fabs( f( mid ) - data[ index ] );
        maxerr = std :: max( maxerr, lerr );
        if( testweight && (weight[ index ] < 0) )
        {
          sout_ << "<" << rank << "/test> Error in communication test.";
          sout_ << " weight:" << weight[ index ] << " (should be 0)";
          sout_ << " value is : " << data[ index ];
          sout_ << " index is: " << index;
          sout_ << " level:" << entity.level();
          sout_ << std :: endl;
        }
      }
      else
      {
        const int numSubEntities = entity.template count< cdim >();
        for( int i=0; i < numSubEntities; ++i )
        {
          SubEntityPointer subEp = entity.template entity< cdim >( i );

          const int index = indexSet_.index( *subEp );
          CoordinateVector cmid( 0.0 );

          const int numVertices = subEp->geometry().corners();
          for( int j = 0; j< numVertices; ++j )
            cmid += subEp->geometry().corner( j );
          cmid /= double( numVertices );

          double lerr = fabs( f( cmid ) - data[ index ] );
          maxerr = std::max( maxerr, lerr );
          if( testweight && (weight[ index ] < 0) )
          {
            sout_ << "<" << rank << "/test> Error in communication test.";
            sout_ << " weight:" << weight[ index ] << " should be zero!";
            sout_ << " value is : " << data[ index ];
            sout_ << " index is:" << index;
            sout_ << " level: " << entity.level() ;
            sout_ << std :: endl;

            for( int j = 0; j < numVertices; )
            {
              const ReferenceElement< double, dim > &refElem
                = ReferenceElements< double, dim > :: general( entity.type() );
              const int vx = refElem.subEntity( i, cdim, j, dim );

              const int tid = Dune::GenericGeometry::topologyId( subEp->type() );
              const int gj = Dune::GenericGeometry::MapNumberingProvider< dim-cdim >::template dune2generic< dim-cdim >( tid, j );

              sout_ << "index: " << indexSet_.template subIndex< dim >( entity, vx )
                    << " " << subEp->geometry().corner( gj );
              (++j < numVertices ? sout_ <<  "/" : sout_ << std :: endl);
            }
          }
        }
      }
    }
    return maxerr;
  }

  // The main ''algorithn''
  bool checkCommunication ()
  {
    upwind_[ 0 ] = -0.1113;
    int myrank = gridView_.comm().rank();

    if( myrank == 0 )
    {
      std::cout << "TEST ";
      (level_ < 0 ? std :: cout << "Leaf" : std :: cout << "Level<" << level_ << ">");
      std :: cout << " communication for codim " << cdim << std :: endl;
    }

    const int dataSize = indexSet_.size( cdim );
    ArrayType data(dataSize, 0.0);
    ArrayType weight(dataSize, 0.0);
    project( dataSize, data, weight, myrank );

    double preresult = test( dataSize, data, weight, false );
    sout_ << "Test before Communication on <" << myrank << "> " << preresult << std :: endl;
    // Communicate
    typedef typename Grid :: Traits :: GlobalIdSet GlobalIdSet;
    ExampleDataHandle< IndexSet, GlobalIdSet, ArrayType >
    dh( indexSet_, gridView_.grid().globalIdSet(), cdim, data, weight );

    // call communication of grid
    try
    {
      gridView_.communicate(dh,InteriorBorder_All_Interface,ForwardCommunication);
      // make sure backward communication does the same, this should change nothing
      gridView_.communicate(dh,InteriorBorder_All_Interface,BackwardCommunication);
      //gridView_.communicate(dh,All_All_Interface,ForwardCommunication);
    }
    catch( const Dune :: NotImplemented &exception )
    {
      if( myrank == 0 )
      {
        sout_ << "Error: Communication for codimension " << cdim
              << " not implemented." << std :: endl;
        sout_ << "       (" << exception << ")" << std :: endl;
      }
      return false;
    }

    double result = test( dataSize, data, weight, true );
    sout_ << "Test after Communication on <" << myrank << "> " << result << std :: endl;
    return (fabs(result) < 1e-8);
  }

public:
  CheckCommunication ( const GridView &gridView, OutputStream &sout, int level )
    : upwind_( -1.0 ),
      sout_( sout ),
      gridView_( gridView ),
      indexSet_( gridView_.indexSet() ),
      level_( level )
  {
    if( !checkCommunication() )
    {
      std :: cerr << "Error in communication test for codim "
                  << cdim << "!" << std :: endl;
    }

    // for automatic testing of all codims
    CheckCommunication< GridView, NextCodim< Grid, cdim > :: calc, OutputStream >
    test( gridView_, sout_, level_ );
  }
};


template< class GridView, class OutputStream >
class CheckCommunication< GridView, -1, OutputStream >
{
public:
  CheckCommunication ( const GridView &gridView, OutputStream &sout, int level )
  {}
};


template< class Grid, class OutputStream >
void checkCommunication( const Grid &grid, int level, OutputStream &sout )
{
  if( level < 0 )
  {
    typedef typename Grid :: template Partition< All_Partition > :: LeafGridView GridView;
    GridView gridView = grid.leafView();
    CheckCommunication< GridView, GridView :: dimension, OutputStream >
    test( gridView, sout, level );
  }
  else
  {
    typedef typename Grid :: template Partition< All_Partition > :: LevelGridView GridView;
    GridView gridView = grid.levelView( level );
    CheckCommunication< GridView, GridView :: dimension, OutputStream >
    test( gridView, sout, level );
  }
}