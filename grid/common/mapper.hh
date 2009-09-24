// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
// $Id$

#ifndef DUNE_MAPPER2_HH
#define DUNE_MAPPER2_HH

#include <iostream>
#include <dune/common/exceptions.hh>
#include <dune/common/helpertemplates.hh>

#include <dune/common/bartonnackmanifcheck.hh>

/** @file
 * @author Peter Bastian
 * @brief Provides classes with basic mappers which are used to attach data to a grid
 *
 */

/*! @addtogroup Mapper Mapper
   \ingroup Grid


   @section Mapper1 What is a Mapper ?
   <!--============================-->

   A mapper class is used to attach user-defined data to a subset of the grid entities
   \f$E^\prime\subseteq E\f$.

   It is assumed that the data \f$D(E^\prime)\f$ associated with
   \f$E^\prime\f$ is stored in an array. The array can be viewed as a map
   \f[ a : I_{E^\prime} \to D(E^\prime) \f] from the consecutive, zero-starting index set
   \f$ I_{E^\prime} = \{0, \ldots, |E^\prime|-1\}\f$ of \f$E^\prime\f$ to the data set.

   The mapper class provides a mapping \f[ m : E^\prime \to I_{E^\prime} \f] from the entity
   set to the index set.

   Access from a grid entity \f$e\in E^\prime\f$ to its associated data element \f$d_e\f$ then is
   a two step process: \f[ a(m(e)) = d_e. \f]

   @section Mapper2 Different Kinds of Mappers
   <!--====================================-->

   There are different kinds of mappers depending on functionality and efficiency of
   their implementation. The user selects an appropriate mapper depending on her/his needs.
   All mappers conform to the same interface.

   @subsection para1 Index based Mappers

   An index-based mapper is allocated for a grid and can be used as long as the grid is not changed
   (i.e. refined, coarsened or load balanced). The implementation of static mappers
   is based on a Dune::IndexSet and is typically of \f$O(1)\f$ complexity with a very
   small constant. Index-based mappers are only available for restricted (but usually sufficient)
   entity sets.

   @subsection para2 Id based Mappers

   An id-based mapper can also be used while a grid changes. For that it
   has to be implemented on the basis of a Dune::IdSet. This may be relatively slow
   because the data type used for ids is usually not an int and the non-consecutive
   ids require more complicated search data structures (typically a map). Access is therefore
   at least \f$O(\log |E^\prime|)\f$. On the other hand, id-based mappers can treat arbitrary
   entity sets \f$E^\prime\f$.

   @section Mapper3 Mapper Interface
   <!--==========================-->

   This interface is implemented by the class template Dune::Mapper. For a full documentation see the
   description of this class.

   The function Dune::Mapper::map delivers the index for an entity. Note that that for
   performance reasons it is usually not checked whether the entity is really in the
   entity set.

   The functions Dune::Mapper::map delivers the index for a (sub-)entity

   The function Dune::Mapper::size returns the size of the entity set, i.e. \f$|E^\prime|\f$

   The different implementations of the mapper interface are listed below.

   @section Mapper4 Overview of Different Mapper Implementations
   <!--======================================================-->

   @section Mapper5 Mappers and Mesh Changes
   <!--==================================-->


 */


namespace Dune
{
  /**
   * @addtogroup Mapper
   *
   * @{
   */


  /** @brief Mapper interface.

     This class template is used as a base class for all mapper implementations.
     It uses the Barton-Nackman trick to ensure conformity to the interface.

     Template parameters are:

     - <tt>G</tt> Type that is a model of Dune::Grid.
     - <tt>MapperImp</tt> Type that is a model of Dune::Mapper.

   */
  template <typename G, typename MapperImp>
  class Mapper {
  public:

    /** @brief Map entity to array index.

            \param e Reference to codim cc entity. The codim is extracted from the entity.
            \return An index in the range 0 ... Max number of entities in set - 1.
     */
    template<class EntityType>
    int map (const EntityType& e) const
    {
      CHECK_INTERFACE_IMPLEMENTATION((asImp().map(e)));
      return asImp().map(e);
    }

#ifdef DUNE_ENABLE_OLD_NUMBERING
    /** @brief Map subentity i of codim cc of a codim 0 entity to array index.

       \param e Reference to codim 0 entity.
       \param i Number of codim cc subentity of e, where cc is the template parameter of the function.
       \return An index in the range 0 ... Max number of entities in set - 1.
     */
    template<int cc>     // this is now the subentity's codim
    int DUNE_DEPRECATED map (const typename G::Traits::template Codim<0>::Entity& e, int i) const
    {
      CHECK_INTERFACE_IMPLEMENTATION((asImp().template map<cc>(e,i)));
      return asImp().template map<cc>(e,i);
    }
#endif

    /** @brief Map subentity i of codim cc of a codim 0 entity to array index.

       \param e Reference to codim 0 entity.
       \param i Number of codim cc subentity of e, where cc is the template parameter of the function.
       \return An index in the range 0 ... Max number of entities in set - 1.
     */
    int map (const typename G::Traits::template Codim<0>::Entity& e,
             int i,
             unsigned int codim) const
    {
      CHECK_INTERFACE_IMPLEMENTATION((asImp().map(e,i,codim)));
      return asImp().map(e,i,codim);
    }

    /** @brief Return total number of entities in the entity set managed by the mapper.

       This number can be used to allocate a vector of data elements associated with the
       entities of the set. In the parallel case this number is per process (i.e. it
       may be different in different processes).

       \return Size of the entity set.
     */
    int size () const
    {
      CHECK_INTERFACE_IMPLEMENTATION((asImp().size()));
      return asImp().size();
    }


    /** @brief Returns true if the entity is contained in the index set and at the same
            time the array index is returned.

       \param[in] e Reference to entity
       \param[out] result Filled with array index if entity is contained
       \return true if entity is in entity set of the mapper
     */
    template<class EntityType>
    bool contains (const EntityType& e, int& result) const
    {
      CHECK_INTERFACE_IMPLEMENTATION((asImp().contains(e,result )));
      return asImp().contains(e,result );
    }

    /** @brief Returns true if the subentity is contained in the index set and at the same time
            the array index is returned.

       \param[in] e Reference to codim 0 entity
       \param[in] i subentity number
       \param[out] result Filled with array index if entity is contained
       \return true if entity is in entity set of the mapper
     */
    template<int cc>     // this is now the subentity's codim
    bool contains (const typename G::Traits::template Codim<0>::Entity& e, int i, int& result) const
    {
      CHECK_INTERFACE_IMPLEMENTATION((asImp().template contains<cc>(e,i,result)))
      return asImp().template contains<cc>(e,i,result);
    }

    /** @brief Reinitialize mapper after grid has been modified.
     */
    void update ()
    {
      CHECK_AND_CALL_INTERFACE_IMPLEMENTATION((asImp().update()));
    }

  private:
    //!  Barton-Nackman trick
    MapperImp& asImp () {return static_cast<MapperImp &> (*this);}
    //!  Barton-Nackman trick
    const MapperImp& asImp () const {return static_cast<const MapperImp &>(*this);}
  };

  /** @} */

#undef CHECK_INTERFACE_IMPLEMENTATION
#undef CHECK_AND_CALL_INTERFACE_IMPLEMENTATION

}
#endif
