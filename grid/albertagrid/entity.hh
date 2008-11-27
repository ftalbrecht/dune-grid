// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef DUNE_ALBERTA_ENTITY_HH
#define DUNE_ALBERTA_ENTITY_HH

#include <dune/grid/common/entity.hh>

#include <dune/grid/albertagrid/geometry.hh>

namespace Dune
{

  // Forward Declarations
  // --------------------

  template< int codim, class GridImp >
  class AlbertaGridEntityPointer;

  template< int codim, PartitionIteratorType pitype, class GridImp >
  class AlbertaGridTreeIterator;

  template< class GridImp >
  class AlbertaGridHierarchicIterator;

  template< class GridImp >
  class AlbertaGridIntersectionIterator;



  // AlbertaGridEntity
  // -----------------

  /*!
     A Grid is a container of grid entities. An entity is parametrized by the codimension.
     An entity of codimension c in dimension d is a d-c dimensional object.

     Here: the general template
   */
  template< int cd, int dim, class GridImp >
  class AlbertaGridEntity
    : public EntityDefaultImplementation< cd, dim, GridImp, AlbertaGridEntity >
  {
    typedef AlbertaGridEntity< cd, dim, GridImp > This;

    enum { dimworld = GridImp::dimensionworld };
    friend class AlbertaGrid < dim , dimworld >;
    friend class AlbertaGridEntity < 0, dim, GridImp>;
    friend class AlbertaGridTreeIterator < cd, All_Partition,GridImp>;
    friend class AlbertaGridEntityPointer<cd,GridImp>;

    typedef AlbertaGridGeometry<dim-cd,dimworld,GridImp> GeometryImp;

  public:
    static const int dimension = dim;
    static const int codimension = cd;
    static const int mydimension = dimension - codimension;

    template <int cc>
    struct Codim
    {
      typedef typename GridImp::template Codim<cc>::EntityPointer EntityPointer;
    };

    typedef typename GridImp::template Codim<cd>::Entity Entity;
    typedef typename GridImp::template Codim<0>::EntityPointer EntityPointer;
    typedef typename GridImp::template Codim<cd>::Geometry Geometry;
    typedef typename GridImp::template Codim<cd>::LevelIterator LevelIterator;

    //! level of this element
    int level () const;

    //! index of the boundary which is associated with the entity, 0 for inner entities
    int boundaryId () const;

    //! contructor takeing traverse stack
    AlbertaGridEntity(const GridImp &grid, int level,
                      ALBERTA TRAVERSE_STACK * travStack);

    //! cosntructor
    AlbertaGridEntity(const GridImp &grid, int level, bool);

    //! copy constructor
    AlbertaGridEntity ( const This &other );

    //! return partition type of this entity ( see grid.hh )
    PartitionType partitionType() const;

    //! geometry of this entity
    const Geometry & geometry () const;

    //! type of geometry of this entity
    GeometryType type () const;

    //***********************************************
    //  End of Interface methods
    //***********************************************
    //! needed for the LevelIterator and LeafIterator
    ALBERTA EL_INFO *getElInfo () const;
    //! return element for equaltiy in EntityPointer
    ALBERTA EL *getElement () const;

    //! set elInfo and Element and builtgeometry to zero
    void removeElInfo();

    //! return the current face/edge or vertex number
    //! no interface method
    int getFEVnum () const;

    //! equality of entities
    bool equals ( const This &other ) const;

    // dummy function, only needed for codim 0
    bool leafIt () const { return false; }

    // methods for setting the infos from the albert mesh
    void setTraverseStack (ALBERTA TRAVERSE_STACK *travStack);
    void setElInfo ( ALBERTA EL_INFO *elInfo, int subEntity );

    // same as setElInfo just with a entity given
    void setEntity ( const This &other );

    // set level of entity
    void setLevel ( int newLevel );
    void setNewLevel ( int newLevel , bool ) { setLevel(level); }

    //! return reference to grid
    const GridImp& grid() const { return grid_; }

  private:
    const GeometryImp &geoImp () const
    {
      return GridImp :: getRealImplementation( geo_ );
    }

    GeometryImp &geoImp ()
    {
      return GridImp :: getRealImplementation( geo_ );
    }

  private:
    // the grid this entity belong to
    const GridImp &grid_;

    // Alberta element info
    ALBERTA EL_INFO *elInfo_;

    // Alberta element
    ALBERTA EL * element_;

    // current traverse stack this entity belongs too
    ALBERTA TRAVERSE_STACK * travStack_;

    //! level
    int level_;

    // type of createable object, just derived from Geometry class
    typedef MakeableInterfaceObject<Geometry> GeometryObject;

    //! the current geometry
    GeometryObject geo_;

    //! true if geometry has been constructed
    mutable bool builtgeometry_;

    //! local coord within father
    mutable FieldVector<albertCtype, dim> localFatherCoords_;
    mutable bool localFCoordCalced_;

    //! Number of the subentity within the element
    int subEntity_;
  };

  /*!
     A Grid is a container of grid entities. An entity is parametrized by the codimension.
     An entity of codimension c in dimension d is a d-c dimensional object.

     Entities of codimension 0 ("elements") are defined through template specialization. Note
     that this specialization has an extended interface compared to the general case

     Entities of codimension 0  allow to visit all neighbors, where
     a neighbor is an entity of codimension 0 which has a common entity of codimension 1 with the
     These neighbors are accessed via an iterator. This allows the implementation of
     non-matching meshes. The number of neigbors may be different from the number of faces/edges
     of an element!
   */
  //***********************
  //
  //  --AlbertaGridEntity
  //  --0Entity
  //
  //***********************
  template<int dim, class GridImp>
  class AlbertaGridEntity<0,dim,GridImp> :
    public EntityDefaultImplementation <0,dim,GridImp,AlbertaGridEntity>
  {
    enum { dimworld = GridImp::dimensionworld };
    friend class AlbertaGrid < dim , GridImp::dimensionworld >;
    friend class AlbertaMarkerVector;
    friend class AlbertaGridIntersectionIterator <GridImp>;
    friend class AlbertaGridHierarchicIterator <GridImp>;
    friend class AlbertaGridTreeIterator <0,All_Partition,GridImp>;
    friend class AlbertaGridEntityPointer<0,GridImp>;

  public:
    static const int dimension = dim;
    static const int codimension = 0;
    static const int mydimension = dimension - codimension;

    template< int codim >
    struct Codim
    {
      typedef typename GridImp :: template Codim< codim > :: EntityPointer
      EntityPointer;
    };

    typedef typename GridImp::template Codim<0>::Entity Entity;
    typedef typename GridImp::template Codim<0>::Geometry Geometry;
    typedef AlbertaGridGeometry<dim,dimworld,GridImp> GeometryImp;

    typedef typename GridImp::template Codim<0>::LevelIterator LevelIterator;
    typedef typename GridImp::template Codim<0>::HierarchicIterator HierarchicIterator;
    typedef typename GridImp::template Codim<0>::EntityPointer EntityPointer;

    typedef LeafIntersectionIteratorWrapper<GridImp> AlbertaGridLeafIntersectionIteratorType;
    typedef AlbertaGridLeafIntersectionIteratorType AlbertaGridIntersectionIteratorType;
    typedef AlbertaGridLeafIntersectionIteratorType AlbertaGridLevelIntersectionIteratorType;

    //! Destructor, needed perhaps needed for deleteing faceEntity_ and
    //! edgeEntity_ , see below
    //! there are only implementations for dim==dimworld 2,3
    ~AlbertaGridEntity() {};

    //! Constructor, real information is set via setElInfo method
    AlbertaGridEntity(const GridImp &grid, int level, bool leafIt );

    AlbertaGridEntity(const AlbertaGridEntity & org);

    //! level of this element
    int level () const;

    //! index of the boundary which is associated with the entity, 0 for inner entities
    int boundaryId () const;

    //! geometry of this entity
    const Geometry & geometry () const;

    //! type of geometry of this entity
    GeometryType type () const;

    /*! Intra-element access to entities of codimension cc > codim. Return number of entities
       with codimension cc.
     */
    //!< Default codim 1 Faces and codim == dim Vertices
    template<int cc> int count () const;

    //! Provide access to mesh entity i of given codimension. Entities
    //!  are numbered 0 ... count<cc>()-1
    template<int cc> typename Codim<cc>::EntityPointer entity (int i) const;

    //! Provide access to mesh entity i of given codimension. Entities
    //!  are numbered 0 ... count<cc>()-1
    //template<int cc> void entity (AlbertaGridTreeIterator<cc,dim,dimworld>& it, int i);

    /*! Intra-level access to intersection with neighboring elements.
       A neighbor is an entity of codimension 0
       which has an entity of codimension 1 in commen with this entity. Access to neighbors
       is provided using iterators. This allows meshes to be nonmatching. Returns iterator
       referencing the first neighbor. */
    AlbertaGridLeafIntersectionIteratorType ileafbegin () const;
    AlbertaGridIntersectionIteratorType ibegin () const
    {
      return ileafbegin();
    }

    AlbertaGridLevelIntersectionIteratorType ilevelbegin () const
    {
      DUNE_THROW(NotImplemented,"method ilevelbegin not implemented!");
      return ileafbegin();
    }

    //! Reference to one past the last intersection with neighbor
    AlbertaGridIntersectionIteratorType ileafend () const;
    AlbertaGridIntersectionIteratorType iend () const { return ileafend();}
    AlbertaGridLeafIntersectionIteratorType ilevelend () const
    {
      DUNE_THROW(NotImplemented,"method ilevelend not implemented!");
      return ibegin();
    }

    //! returns true if entity is leaf entity, i.e. has no children
    bool isLeaf () const ;

    //! Inter-level access to father element on coarser grid.
    //! Assumes that meshes are nested.
    EntityPointer father () const;

    /*! Location of this element relative to the reference element
       of the father. This is sufficient to interpolate all
       dofs in conforming case. Nonconforming may require access to
       neighbors of father and computations with local coordinates.
       On the fly case is somewhat inefficient since dofs  are visited
       several times. If we store interpolation matrices, this is tolerable.
       We assume that on-the-fly implementation of numerical algorithms
       is only done for simple discretizations. Assumes that meshes are nested.
       NOTE: the imeplementation at the moment is very inefficient.
     */
    const Geometry & geometryInFather () const;

    /*! Inter-level access to son elements on higher levels<=maxlevel.
       This is provided for sparsely stored nested unstructured meshes.
       Returns iterator to first son.
     */
    AlbertaGridHierarchicIterator<GridImp> hbegin (int maxlevel) const;

    //! Returns iterator to one past the last son
    AlbertaGridHierarchicIterator<GridImp> hend (int maxlevel) const;

    /**\brief Returns true, if entity was refined during last adaptation
        cycle */
    bool wasRefined () const DUNE_DEPRECATED {return isNew();}

    /**\brief Returns true, if entity might be coarsened during next
        adaption cycle */
    bool mightBeCoarsened () const DUNE_DEPRECATED { return mightVanish(); }

    /**\brief Returns true, if entity was refined during last adaptation
        cycle */
    bool isNew () const ;

    /**\brief Returns true, if entity might be coarsened during next
        adaption cycle */
    bool mightVanish () const ;

    /**\brief Returns true, if entity has intersections with boundary
     */
    bool hasBoundaryIntersections () const ;

    //! return partition type of this entity ( see grid.hh )
    PartitionType partitionType() const;

    //! equality of entities
    bool equals ( const AlbertaGridEntity<0,dim,GridImp> & i) const;

    //***************************************************************
    //  Interface for parallelisation
    //***************************************************************
    // set leaf data with processor number
    void setLeafData( int proc );

    // return true if this entity belong to master set of this grid
    bool master() const;

    // return 0 for elements
    int getFEVnum () const { return 0; }

    // needed for LevelIterator to compare
    ALBERTA EL_INFO *getElInfo () const;

    // return element for equaltiy in EntityPointer
    ALBERTA EL *getElement () const;

    // set elInfo and Element to nil
    void removeElInfo();

    // returns true if entity comes from LeafIterator
    bool leafIt () const { return leafIt_; }

    // face, edge and vertex only for codim > 0, in this
    // case just to supply the same interface
    void setTraverseStack (ALBERTA TRAVERSE_STACK *travStack);
    void setElInfo (ALBERTA EL_INFO *elInfo,
                    int face = 0,
                    int edge = 0,
                    int vertex = 0 );

    // same as setElInfo just with a entity given
    void setEntity (const AlbertaGridEntity<0,dim,GridImp> & org);

    //! return reference to grid
    const GridImp& grid() const { return grid_; }
  private:
    //! return which number of child we are, i.e. 0 or 1
    int nChild () const;

    //! make a new AlbertaGridEntity
    void makeDescription();

    //! the corresponding grid
    const GridImp & grid_;

    //! the level of the entity
    int level_;

    //! pointer to the Albert TRAVERSE_STACK data
    ALBERTA TRAVERSE_STACK * travStack_;

    //! pointer to the real Albert element data
    ALBERTA EL_INFO *elInfo_;

    //! pointer to the real Albert element
    ALBERTA EL *element_;

    // local coordinates within father
    typedef MakeableInterfaceObject<Geometry> GeometryObject;

    //! the cuurent geometry
    mutable GeometryObject geoObj_;
    mutable GeometryImp & geo_;
    mutable bool builtgeometry_;  //!< true if geometry has been constructed

    // is true if entity comes from leaf iterator
    bool leafIt_;

  }; // end of AlbertaGridEntity codim = 0

}

#endif
