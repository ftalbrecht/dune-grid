// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#include <dune/grid/geometrygrid/grid.hh>

/** \addtogroup GeoGrid
 *
 *  The GeometryGrid is an implementation of the DUNE grid interface that can
 *  wrap any other DUNE grid (called host grid) and replace its geometry.
 *  To this end, the grid also gets a coordinate function that maps the corners
 *  of the host grid into any larger Euklidian space.
 *  The generic geometries are then used to provide a geometry implementation
 *  for the grid, interpolating the corners in a linear (respectively n-linear)
 *  manner.
 *
 *  \image html helix.png
 *  The figure above displays a <tt>GeometryGrid< YaspGrid< 2 >, Helix ></tt>,
 *  where Helix models the following coordinate function:
 *  \f[
 *    \left(\begin{array}{c}r\\\varphi\end{array}\right)
 *    \mapsto
 *    \left(\begin{array}{c}
 *      (r + \frac{1}{5}) \cos( 2 \pi \varphi )\\
 *      (r + \frac{1}{5}) \sin( 2 \pi \varphi )\\
 *      \varphi
 *    \end{array}\right).
 *  \f]
 *  Though YaspGrid can only model plane, Carthesian grids, using GeometryGrid
 *  we have obtained a parallel surface grid with quadrilateral elements.
 *
 *  \section features Features
 *
 *  Features of the GeometryGrid include:
 *  - complete wrapper of the host grid
 *    (i.e., no non-geometric feature of the host grid is lost)
 *  - Only uses the coordinate of the corners of each entity -
 *    no other geometric information of the underlying grid is used.
 *  - provides entities for all codimensions, even if the host grid does not
 *    (though communication is not extended to these codimensions)
 *  .
 *
 *  \section usage Usage
 *
 *  There are three different construction mechanisms for a geometry grid.
 *  In each case a instance of the host grid must be provided and in
 *  addition:
 *  - a function mapping
 *    global coordinates from the host grid to some space
 *    with larger or equal dimension. For an entity \c e of the host grid
 *    with geometry \c e.g the resulting entity in the Dune::GeometryGrid has
 *    corners \c F(e.g.corner(i)) where \c F is the global coordinate
 *    mapping provided. For the geometry of the Dune::GeometryGrid the class
 *    Dune::GenericGeometry::CornerMapping is used.
 *  - a vector like container assinging each corner of a host entity a codimension.
 *  .
 *  Remark: in the second case no geometry class has to be implemented by the
 *          host grid.
 *          In the first case the host grid must provide an implementation of
 *          the method <tt>corner</tt> on the geometry class for codimension
 *          zero entity.
 *
 *  The approach taken is determined by the second template argument:
 *  \code
 *    GeometryGrid<HostGridType,CoordFunction> grid(hostGrid,coordFunction);
 *  \endcode
 *  The class \c CoordFunction must either be derived from
 *  Dune::AnalyticalCoordFunction or from
 *  Dune::DiscreteCoordFunction.
 *  An example of a analytical coordinate function is given by the following code:
 *  \code
 *  class ExampleFunction
 *  : public Dune :: AnalyticalCoordFunction< double, 2, 3, ExampleFunction >
 *  {
 *    typedef ExampleFunction This;
 *    typedef Dune :: AnalyticalCoordFunction< double, 2, 3, This > Base;
 *
 *  public:
 *    typedef Base :: DomainVector DomainVector;
 *    typedef Base :: RangeVector RangeVector;
 *
 *    void evaluate ( const DomainVector &x, RangeVector &y ) const
 *    {
 *      y[ 0 ] = x[ 0 ];
 *      y[ 1 ] = x[ 1 ];
 *      y[ 2 ] = x[ 0 ] + x[ 1 ];
 *    }
 *  };
 *  \endcode
 *  For a discrete coordinate function a method of the form
 *  \code
 *  template< class HostEntity >
 *  void evaluate ( const HostEntity &hostEntity, unsigned int corner,
 *                  RangeVector &y ) const
 *  \endcode
 *  must be implemented.
 *
 *  \note The <tt>GeometryGrid</tt> is able to forward a non-standard feature
 *        of some DUNE grids: the <tt>HierarchicIndexSet</tt>.
 *        Since only very few grids support this feature, <tt>GeometryGrid</tt>
 *        needs to know whether the host grid supports this feature.
 *        To this end, it expects the capability <tt>hasHierarchicIndexSet</tt>
 *        to be set, which is not part of the standard capabilities defined in
 *        <tt>dune/grid/common/capabilities.hh</tt>.
 *        If you intend to use the <tt>GeometryGrid</tt> with a host grid that
 *        potentially does not provide a <tt>HierarchicIndexSet</tt>, you have
 *        to provide a default implementation of this capability:
 *  \code
 *  namespace Dune
 *  {
 *
 *    namespace Capabilities
 *    {
 *
 *      template< class Grid >
 *      struct hasHierarchicIndexSet
 *      {
 *        static const bool v = false;
 *      };
 *
 *      template< class Grid >
 *      struct hasHierarchicIndexSet< const Grid >
 *      {
 *        static const bool v = hasHierarchicIndexSet< Grid >::v;
 *      };
 *
 *    }
 *
 *  }
 *  \endcode
 */