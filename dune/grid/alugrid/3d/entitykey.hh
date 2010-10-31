// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef ALU3DGRID_ENTITYKEY_HH
#define ALU3DGRID_ENTITYKEY_HH

namespace Dune
{

  template<int cd, class GridImp>
  class ALU3dGridEntityKey ;

  //**********************************************************************
  //
  // --ALU3dGridEntityKey
  // --EntityKey
  //**********************************************************************
  template< int codim, class GridImp >
  class ALU3dGridEntityKeyBase
  {
  protected:
    typedef ALU3dGridEntityKeyBase< codim, GridImp > ThisType;
    enum { dim       = GridImp::dimension };
    enum { dimworld  = GridImp::dimensionworld };


    typedef typename GridImp::MPICommunicatorType Comm;

    friend class ALU3dGridEntity<codim,dim,GridImp>;
    friend class ALU3dGridEntity< 0,dim,GridImp>;
    friend class ALU3dGrid < GridImp::elementType, Comm >;

    typedef ALU3dImplTraits<GridImp::elementType, Comm > ImplTraits;
    typedef typename ImplTraits::template Codim<codim>::ImplementationType ImplementationType;
    typedef typename ImplTraits::template Codim<codim>::InterfaceType HElementType;
    typedef typename ImplTraits::template Codim<codim>::EntityKeyType KeyType ;

    typedef typename ImplTraits::BNDFaceType BNDFaceType;
    typedef typename ImplTraits::HBndSegType HBndSegType;

    template <int cd, class Key>
    struct Bnd
    {
      static Key* toKey(const HBndSegType*)
      {
        return (Key*) 0;
      }
      static HElementType* getItem(KeyType* key)
      {
        return static_cast< HElementType* > ( key );
      }
      static bool isGhost(KeyType*) { return false; }
      static BNDFaceType* ghost( KeyType*  ) { return ( BNDFaceType* ) 0; }
    };
    template <class Key>
    struct Bnd<0, Key>
    {
      static Key* toKey(const HBndSegType* ghostFace)
      {
        return static_cast< KeyType* > (const_cast< BNDFaceType* >( static_cast<const BNDFaceType*> (ghostFace)));
      }
      static HElementType* getItem(KeyType* key)
      {
        if( key )
        {
          if( key->isboundary() )
          {
            return ((static_cast< BNDFaceType* > ( key ))->getGhost().first);
          }
          else
          {
            // we cannot cast to HElement here, since only the implementation is derived
            // from hasFace
            return static_cast< HElementType * > (static_cast< ImplementationType* > (key));
          }
        }
        else
          return static_cast< HElementType * > (0) ;
      }
      static bool isGhost(KeyType* key) { assert( key ); return key->isboundary(); }
      static BNDFaceType* ghost( KeyType* key ) { assert( key ); return (static_cast< BNDFaceType* > ( key )); }
    };
  public:
    static const int defaultValue = -665 ;

    enum { codimension = codim };

    //! type of Entity
    typedef typename GridImp::template Codim<codimension>::Entity Entity;
    //! underlying EntityImplementation
    typedef MakeableInterfaceObject<Entity> EntityObject;
    typedef typename EntityObject :: ImplementationType EntityImp;

    //! typedef of my type
    typedef ThisType ALU3dGridEntityKeyType;

    //! make type of entity pointer implementation available in derived classes
    typedef ALU3dGridEntityKey<codimension,GridImp> EntityKeyImp;

    //! Destructor
    ~ALU3dGridEntityKeyBase()
    {
#ifndef NDEBUG
      // clear pointer
      clear();
#endif
    }

    //! Constructor for EntityKey that points to an element
    ALU3dGridEntityKeyBase();

    //! Constructor for EntityKey that points to an element
    ALU3dGridEntityKeyBase(const HElementType& item);

    //! Constructor for EntityKey that points to an element
    ALU3dGridEntityKeyBase(const HElementType* item, const HBndSegType* ghostFace );

    //! Constructor for EntityKey that points to an ghost
    ALU3dGridEntityKeyBase(const HBndSegType& ghostFace );

    //! copy constructor
    ALU3dGridEntityKeyBase(const ALU3dGridEntityKeyType & org);

    //! equality
    bool equals (const ALU3dGridEntityKeyType& i) const;

    //! equality operator
    bool operator == (const ALU3dGridEntityKeyType& i) const
    {
      return equals( i );
    }

    //! inequality operator
    bool operator != (const ALU3dGridEntityKeyType& i) const
    {
      return ! equals( i );
    }

    //! get item from key
    HElementType* item() const { return Bnd<codim,KeyType>::getItem( item_ ); }

    //! return iterior item
    HElementType* interior() const
    {
      assert( ! isGhost() );
      return static_cast< HElementType * > (static_cast< ImplementationType* > (item_));
    }

    //! methods for ghosts
    bool isGhost() const { return Bnd<codim,KeyType>::isGhost( item_ ); }
    BNDFaceType* ghost() const
    {
      assert( isGhost() );
      return Bnd<codim,KeyType>::ghost( item_ );
    }

    void clear()
    {
      item_ = 0;
    }

    KeyType* toKey(const HElementType* item)
    {
      return static_cast< KeyType* > (const_cast< ImplementationType* > (static_cast<const ImplementationType* > (item)));
    }

    void set(const HElementType& item)
    {
      item_ = toKey( &item );
    }

    KeyType* toKey( const HBndSegType* ghostFace )
    {
      return Bnd<codim,KeyType>::toKey( ghostFace );
    }
    void set(const HBndSegType& ghostFace)
    {
      item_ = toKey( &ghostFace );
    }

    int level () const { return defaultValue; }
    int twist () const { return defaultValue; }
    int face  () const { return defaultValue; }

    //! assignment operator
    ThisType & operator = (const ThisType & org);

  protected:
    // pointer to item
    mutable KeyType* item_;
  };

  template<int cd, class GridImp>
  class ALU3dGridEntityKey :
    public ALU3dGridEntityKeyBase<cd,GridImp>
  {
    typedef ALU3dGridEntityKeyBase<cd,GridImp> BaseType;

    typedef ALU3dGridEntityKey <cd,GridImp> ThisType;
    enum { dim       = GridImp::dimension };
    enum { dimworld  = GridImp::dimensionworld };

    typedef typename GridImp::MPICommunicatorType Comm;

    friend class ALU3dGridEntity<cd,dim,GridImp>;
    friend class ALU3dGridEntity< 0,dim,GridImp>;
    friend class ALU3dGrid < GridImp::elementType, Comm >;

    typedef ALU3dImplTraits< GridImp::elementType, Comm > ImplTraits;
    typedef typename ImplTraits::template Codim<cd>::ImplementationType ImplementationType;
    typedef typename ImplTraits::template Codim<cd>::InterfaceType HElementType;

    typedef typename ImplTraits::BNDFaceType BNDFaceType;
    typedef ALU3dGridEntity<cd,dim,GridImp> ALU3dGridEntityType;

  public:
    using BaseType :: defaultValue ;

    //! type of Entity
    typedef typename GridImp::template Codim<cd>::Entity Entity;

    //! typedef of my type
    typedef ALU3dGridEntityKey<cd,GridImp> ALU3dGridEntityKeyType;

    //! Constructor for EntityKey that points to an element
    ALU3dGridEntityKey(const ImplementationType & item)
    {
      // this constructor should only be called by codim=0 entity keys
      assert( false );
      abort();
    }

    //! Constructor for EntityKey that points to an element
    ALU3dGridEntityKey(const HElementType & item,
                       const int level,
                       const int twist = defaultValue,
                       const int duneFace = defaultValue
                       );

    //! Constructor for EntityKey that points to an element
    ALU3dGridEntityKey() : BaseType(), level_(defaultValue), twist_(defaultValue), face_(defaultValue) {}

    //! Constructor for EntityKey that points to given entity
    ALU3dGridEntityKey(const ALU3dGridEntityType& entity)
      : ALU3dGridEntityKeyBase<cd,GridImp> (entity.getItem()),
        level_(entity.level()), twist_(defaultValue), face_(defaultValue)
    {}

    //! copy constructor
    ALU3dGridEntityKey(const ALU3dGridEntityKeyType & org);

    //! assignment operator
    ThisType & operator = (const ThisType & org);

    //! clear the key data structure
    void clear();

    //! set element and level
    void set(const HElementType & item, const int level )
    {
      BaseType :: set( item );
      level_ = level ;
    }

    //! return level
    int level () const { return level_ ; }
    //! return twist
    int twist () const { return twist_ ; }
    //! return face
    int face  () const { return face_ ; }

    using BaseType :: set ;

    bool operator == (const ALU3dGridEntityKeyType& i) const
    {
      return equals( i );
    }

    bool operator != (const ALU3dGridEntityKeyType& i) const
    {
      return ! equals( i );
    }

    //! equality, calls BaseType equals
    bool equals (const ALU3dGridEntityKeyType& key) const
    {
      // only compare the item pointer, this is the real key
      return BaseType :: equals( key ) && (level() == key.level());
    }

  protected:
    // level of entity
    int level_;
    // twist of face, for codim 1 only
    int twist_;
    // face number, for codim 1 only
    int face_;
  };

  //! ALUGridEntityKey points to an entity
  //! this class is the specialisation for codim 0,
  //! it has exactly the same functionality as the ALU3dGridEntityKeyBase
  template<class GridImp>
  class ALU3dGridEntityKey<0,GridImp> :
    public ALU3dGridEntityKeyBase<0,GridImp>
  {
  protected:
    typedef ALU3dGridEntityKeyBase<0,GridImp> BaseType;

    enum { cd = 0 };
    typedef ALU3dGridEntityKey <cd,GridImp> ThisType;
    enum { dim       = GridImp::dimension };
    enum { dimworld  = GridImp::dimensionworld };

    typedef typename GridImp::MPICommunicatorType Comm;

    friend class ALU3dGridEntity<cd,dim,GridImp>;
    friend class ALU3dGridEntity< 0,dim,GridImp>;
    friend class ALU3dGrid < GridImp::elementType, Comm >;

    typedef ALU3dImplTraits<GridImp::elementType, Comm > ImplTraits;
    typedef typename ImplTraits::template Codim<cd>::ImplementationType ImplementationType;
    typedef typename ImplTraits::template Codim<cd>::InterfaceType HElementType;

    typedef typename ImplTraits::BNDFaceType BNDFaceType;
    typedef typename ImplTraits::HBndSegType HBndSegType;

    typedef ALU3dGridEntity< 0,dim,GridImp> ALU3dGridEntityType ;

  public:
    using BaseType :: defaultValue ;

    //! type of Entity
    typedef typename GridImp::template Codim<cd>::Entity Entity;

    //! typedef of my type
    typedef ThisType ALU3dGridEntityKeyType;

    //! Constructor for EntityKey that points to an element
    ALU3dGridEntityKey() : BaseType() {}

    //! Constructor for EntityKey that points to an interior element
    ALU3dGridEntityKey(const HElementType& item)
      : ALU3dGridEntityKeyBase<cd,GridImp> (item) {}

    //! Constructor for EntityKey that points to an interior element
    ALU3dGridEntityKey(const HElementType& item, int , int , int )
      : ALU3dGridEntityKeyBase<cd,GridImp> (item) {}

    //! Constructor for EntityKey that points to an ghost
    ALU3dGridEntityKey(const HBndSegType& ghostFace )
      : ALU3dGridEntityKeyBase<cd,GridImp> ( ghostFace ) {}

    //! copy constructor
    ALU3dGridEntityKey(const ALU3dGridEntityKeyType & org)
      : ALU3dGridEntityKeyBase<cd,GridImp> (org)
    {}
  };


  //! print alugrid entity key to std::stream
  template <int cd, class GridImp>
  inline std :: ostream &operator<< ( std :: ostream &out,
                                      const ALU3dGridEntityKey<cd,GridImp>& key)
  {
    out << key.item() << " " << key.level() << " " << key.twist() << " " << key.face();
    return out;
  }


  //*******************************************************************
  //
  //  Implementation
  //
  //*******************************************************************
  template<int codim, class GridImp >
  inline ALU3dGridEntityKeyBase<codim,GridImp> ::
  ALU3dGridEntityKeyBase()
    : item_( 0 )
  {}

  template<int codim, class GridImp >
  inline ALU3dGridEntityKeyBase<codim,GridImp> ::
  ALU3dGridEntityKeyBase(const HElementType &item)
    : item_( toKey(&item) )
  {}

  template<int codim, class GridImp >
  inline ALU3dGridEntityKeyBase<codim,GridImp> ::
  ALU3dGridEntityKeyBase(const HBndSegType& ghostFace )
    : item_( toKey(&ghostFace) )
  {}

  template<int codim, class GridImp >
  inline ALU3dGridEntityKeyBase<codim,GridImp> ::
  ALU3dGridEntityKeyBase(const ALU3dGridEntityKeyType & org)
    : item_(org.item_)
  {}

  template<int codim, class GridImp >
  inline ALU3dGridEntityKeyBase<codim,GridImp> &
  ALU3dGridEntityKeyBase<codim,GridImp> ::
  operator = (const ALU3dGridEntityKeyType & org)
  {
    item_  = org.item_;
    return *this;
  }

  template<int codim, class GridImp >
  inline bool ALU3dGridEntityKeyBase<codim,GridImp>::
  equals (const ALU3dGridEntityKeyBase<codim,GridImp>& i) const
  {
    // check equality of underlying items
    return (item_ == i.item_);
  }

  ///////////////////////////////////////////////////////////////////
  //
  //  specialisation for higher codims
  //
  ///////////////////////////////////////////////////////////////////

  template<int codim, class GridImp >
  inline ALU3dGridEntityKey<codim,GridImp> ::
  ALU3dGridEntityKey(const HElementType &item,
                     const int level,
                     const int twist,
                     const int duneFace )
    : ALU3dGridEntityKeyBase<codim,GridImp> (item)
      , level_(level)
      , twist_ (twist)
      , face_(duneFace)
  {
    assert( (codim == 1) ? (face_ >= 0) : 1 );
  }

  template<int codim, class GridImp >
  inline ALU3dGridEntityKey<codim,GridImp> ::
  ALU3dGridEntityKey(const ALU3dGridEntityKeyType & org)
    : ALU3dGridEntityKeyBase<codim,GridImp>(org)
      , level_(org.level_)
      , twist_(org.twist_)
      , face_(org.face_)
  {}

  template<int codim, class GridImp >
  inline ALU3dGridEntityKey<codim,GridImp> &
  ALU3dGridEntityKey<codim,GridImp>::
  operator = (const ALU3dGridEntityKeyType & org)
  {
    // docu and cleanup
    BaseType :: operator = ( org );

    // clone other stuff
    level_ = org.level_;
    twist_ = org.twist_;
    face_  = org.face_;
    return *this;
  }

  template<int codim, class GridImp >
  inline void
  ALU3dGridEntityKey<codim,GridImp>::clear ()
  {
    BaseType :: clear();
    level_ = defaultValue ;
    twist_ = defaultValue ;
    face_  = defaultValue ;
  }

} // end namespace Dune
#endif