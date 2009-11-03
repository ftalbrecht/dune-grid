// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef DUNE_GRID_ENTITYPOINTER_HH
#define DUNE_GRID_ENTITYPOINTER_HH

#include <dune/common/iteratorfacades.hh>
#include <dune/grid/common/grid.hh>

/** \file
    \brief Wrapper and interface classe for a static iterator (EntityPointer)
 */

namespace Dune
{

  /**
     @brief Wrapper class for pointers to entities

     Template parameters are:

     - <tt>GridImp</tt> Type that is a model of Dune::Grid
     - <tt>IteratorImp</tt> Class template that is a model of Dune::EntityPointer

     <H3>Engine Concept</H3>

     The EntityPointer class template wraps an object of type IteratorImp and forwards all member
     function calls to corresponding members of this class. In that sense EntityPointer
     defines the interface and IteratorImp supplies the implementation.

     <H3>Relation of EntityPointer and Iterators</H3>

      The EntityPointer can be used like a static iterator. It points to a
      Dune::Entity and can be dereferenced, compared and it knows the
      Entity's level.

      You should be able to initialize and interpret every Dune::XxxIterator
      that iterates over entities
      as a Dune::EntityPointer. Therefore we need an inheritance hierarchy of
      the Iterator wrappers:
      \code
      class Dune::EntityPointer<...>;

      class Dune::LevelIterator<...> :
         public Dune::EntityPointer<...>;

      class Dune::HierarchicIterator<...> :
         public Dune::EntityPointer<...>;

      class Dune::LeafIterator<...> :
         public Dune::EntityPointer<...>;
      \endcode

      This hierarchy must be mimicked in the implementation (i.e. SGrid):
      \code
      class SEntityPointer<...> :
         public Dune::EntityPointerDefault<..., SEntityPointer>;

      class SLevelIterator<...> :
         public SEntityPointer <...>,
         public Dune::LevelIteratorDefault <..., SLevelIterator>;

      class SHierarchicIterator<...> :
         public SEntityPointer <...>,
         public Dune::HierarchicIteratorDefault <..., SHierarchicIterator>;

      ...
      \endcode
      Please note that dereference(...), equals(...) and level() are only
      implemented in SEntityPointer -- SLevelIterator inherits these methods.
      And it is not possible to specialize these, because EntityPointer always
      uses the base class.

      This leads to a hierarchy where
      Dune::LevelIterator<..., SLevelIterator> inherits
      Dune::EntityPointer<..., SLevelIterator> and
      Dune::HierarchicIterator<..., SHierarchicIterator> inherits
      Dune::EntityPointer<..., SHierarchicIterator>.
      And virtualy all Dune::EntityPointer<..., SXxxIterator> are descendents
      of Dune::EntityPointer<..., SEntityPointer>.

      Now you can compare Dune::LevelIterator with Dune::EntityPointer and
      Dune::LeafIterator with Dune::HierarchicIterator. And you can assign
      Dune::EntityPointer from any Dune::XxxIterator class. Even more, you can
      cast an Iterator refence to a reference pointing to Dune::EntityPointer.

      The compiler takes care that you only assign/compare Iterators from the same
      Grid.

      The downside (or advantage) of this inheritance is that you can
      not use different comparison operators and different dereference
      oprators for the different Iterators in one Grid. On the first
      sight it is a downside because one might consider it a good idea
      to have special treatment for different iterators. On the other
      hand it's very confusing for the user if different Iterators show
      different behavior in the same situation. So now they are forced to
      show the same behavior.


      \ingroup GIEntityPointer
   */
  template<class GridImp, class IteratorImp>
  class EntityPointer
  {
    // we must be able to initialize a GenericLeafIterator.realIterator from
    // EntityPointer.realIterator
    friend class Dune::GenericLeafIterator<GridImp>;

    // need to make copy constructor of EntityPointer work for any iterator
    friend class EntityPointer<GridImp,typename IteratorImp::EntityPointerImp>;

  protected:
    IteratorImp realIterator;

    /// autocheck wether imp is convertable into imp::base
    typedef typename
    Dune::EnableIfInterOperable<typename IteratorImp::EntityPointerImp,IteratorImp,
        typename IteratorImp::EntityPointerImp>::type base;

  public:
    //! codimension of entity pointer
    enum { codimension = IteratorImp::codimension };

    //! type of real implementation
    typedef IteratorImp ImplementationType;

    /** \brief The Entity that this EntityPointer can point to */
    typedef typename IteratorImp::Entity Entity;

    enum {
      /** \brief The codimension of this EntityPointer */
      codim = IteratorImp::codimension
    };

    //===========================================================
    /** @name Constructor & conversion
     */
    //@{
    //===========================================================

    /** \brief Templatized copy constructor from arbitrary IteratorImp.
            This enables that an EntityPointer can be copy-constructed from
            LevelIterator, LeafIterator and HierarchicIterator (because
            these are derived from EntityPointer<...> with their
            corresponding implementation.
     */
    template<class ItImp>
    EntityPointer(const EntityPointer<GridImp,ItImp> & ep) :
      realIterator(ep.realIterator) {}

    /** \brief Templatized constructor from type of entity that
        this entity pointer points to. This constructor can be used to
        create an entity pointer from an entity in order to store an
        entity. The implementation of EntityPointer has to have a
        constructor taking a Dune::Entity.
     */
    EntityPointer(const Entity& entity) :
      realIterator( entity.getRealImp() ) {}

    /** \brief Cast to EntityPointer with base class of implementation as engine.
            This conversion ensures assignablity of LevelIterator, LeafIterator and
            HierarchicIterator to EntityPointer.
     */
    operator EntityPointer<GridImp,base>&()
    {
      return reinterpret_cast<EntityPointer<GridImp,base>&>(*this);
    };

    /** \brief Cast to EntityPointer with const base class of implementation as engine.
            This conversion ensures assignablity of LevelIterator, LeafIterator and
            HierarchicIterator to EntityPointer.
     */
    operator const EntityPointer<GridImp,base>&() const
    {
      return reinterpret_cast<const EntityPointer<GridImp,base>&>(*this);
    };

    /** \brief Reduce the entity pointers used
        memory to a minimum necessary to store all needed information.
     */
    void compactify ()
    {
      realIterator.compactify();
    }

    //@}

    //===========================================================
    /** @name Dereferencing
     */
    //@{
    //===========================================================

    /** \brief Dereferencing operator. */
    Entity & operator*() const
    {
      return realIterator.dereference();
    }

    /** \brief Pointer operator. */
    Entity * operator->() const
    {
      return & realIterator.dereference();
    }
    //@}

    //===========================================================
    /** @name Compare methods
     */
    //@{
    //===========================================================

    /** \brief Checks for equality.
            Only works for EntityPointers and iterators on the same grid.
            Due to the conversion operators one can compare
            all kinds of iterators and EntityPointer.
     */
    bool operator==(const EntityPointer<GridImp,base>& rhs) const
    {
      return rhs.equals(*this);
    }

    /** \brief Checks for inequality.
            Only works for EntityPointers and iterators on the same grid.
            Due to the conversion operators one can compare
            all kinds of iterators and EntityPointer.
     */
    bool operator!=(const EntityPointer<GridImp,base>& rhs) const
    {
      return ! rhs.equals(*this);
    }
    //@}


    //===========================================================
    /** @name Query methods
     */
    //@{
    //===========================================================

    /** \brief Ask for level of entity.
            This method is redundant and is only there for efficiency reasons.
            It allows an implementation to return the level without actually
            constructing the entity.
     */
    int level () const
    {
      return realIterator.level();
    }

    //@}


    //===========================================================
    /** @name Implementor interface
     */
    //@{
    //===========================================================


    /** \brief Copy Constructor from an Iterator implementation.

       You can supply LeafIterator, LevelIterator, HierarchicIterator
       or EntityPointer.
     */
    EntityPointer(const IteratorImp & i) :
      realIterator(i) {};

    /** @brief Forward equality check to realIterator */
    bool equals(const EntityPointer& rhs) const
    {
      return this->realIterator.equals(rhs.realIterator);
    }
    //@}

  protected:

    // give the GridDefaultImplementation class access to the realImp
    friend class GridDefaultImplementation<
        GridImp::dimension, GridImp::dimensionworld,
        typename GridImp::ctype,
        typename GridImp::GridFamily> ;

    //! return reference to the real implementation
    ImplementationType & getRealImp() { return realIterator; }
    //! return reference to the real implementation
    const ImplementationType & getRealImp() const { return realIterator; }

  };

  //**********************************************************************
  //
  //  --EntityPointerDefault
  //
  //! Default implementation of EntityPointer.
  //**********************************************************************
  template<int codim, class GridImp, class IteratorImp>
  class EntityPointerDefaultImplementation
  {
  public:
    //! export the type of the EntityPointer Implementation.
    //! Necessary for the typeconversion between Iterators and EntityPointer
    typedef IteratorImp EntityPointerImp;

    //! codimension of entity pointer
    enum { codimension = codim };

    //! make the constructor deprecated
    EntityPointerDefaultImplementation() DUNE_DEPRECATED {}

  private:
    //!  Barton-Nackman trick
    IteratorImp& asImp () {
      return static_cast<IteratorImp&>(*this);
    }
    const IteratorImp& asImp () const {
      return static_cast<const IteratorImp&>(*this);
    }
  } DUNE_DEPRECATED; // end EntityPointerDefaultImplementation

}

#endif // DUNE_GRID_ENTITYPOINTER_HH