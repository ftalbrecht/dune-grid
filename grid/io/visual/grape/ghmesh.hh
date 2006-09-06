// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#ifndef __GRAPE_HMESH_H__
#define __GRAPE_HMESH_H__

enum { MAX_NAME_LENGTH = 32 };

typedef struct dune_elem DUNE_ELEM;
typedef struct dune_fdata DUNE_FDATA;
typedef struct dune_dat DUNE_DAT;

typedef void evalDof_t (DUNE_ELEM *, DUNE_FDATA *, int , double *);
typedef void evalCoord_t (DUNE_ELEM *, DUNE_FDATA *, const double *, double * );

/* interface element */
typedef struct dune_elem
{

  // default constructor
  dune_elem()
    : type(127)
      , eindex(-1)
      , level(-1)
      , level_of_interest(-1)
      , has_children(0)
      , liter(0)
      , enditer(0)
      , hiter(0)
      , actElement(0)
      , gridPart(0)
      , display(0)
      , mesh(0)
  {
    // default set all coordinates to zero
    for(int i=0; i<MAX_EL_DOF; ++i)
    {
      vindex [i] = -1;
      for(int j=0; j<3; ++j)
      {
        vpointer[i][j] = 0.0;
      }
    }
    for(int i=0; i<MAX_EL_FACE; ++i)
    {
      bnd [i] = -1;
    }
  }

  /*
   *  see g_eldesc.h for ElementType
   */
  int type;

  double vpointer [MAX_EL_DOF][3];
  int vindex [MAX_EL_DOF] ;
  int bnd [MAX_EL_FACE] ;
  int eindex;
  int level;
  int level_of_interest;
  int has_children;

  /* is the pointer to LevelIterator or to LeafIterator */
  void          * liter;
  void          * enditer;

  // pointer fo hierarchic iterator */
  void          * hiter;

  /* points to actual iterator to compare an get type */
  /* down cast to EntityPointer */
  void          * actElement;

  /* actual choosen gridPart */
  void          * gridPart;

  // pointer to my display class
  void          * display;

  // pointer to mesh
  void          * mesh;
};

typedef struct dune_fdata
{
  // default constructor
  dune_fdata()
    : mynum (-1)
      , name(0)
      , evalCoord(0)
      , evalDof(0)
      , discFunc(0)
      , indexSet(0)
      , allLevels(0)
      , dimVal(0)
      , dimRange(0)
      , comp(0)
      , polyOrd(0)
      , continuous(0)
      , compName(0)
      , gridPart(0)
      , setGridPartIterators(0) {}

  /* my number in the data vector */
  int mynum;

  const char * name;

  // functions to evaluate
  evalCoord_t * evalCoord;
  evalDof_t   * evalDof;

  /* pointer to object of discrete function or vector */
  const void *discFunc;

  /* pointer to index set of underlying datas */
  const void *indexSet;

  /* are all Levels occupied? */
  int allLevels;

  /* dimension of value, i.e. the length of the vector  */
  int dimVal;

  /* dimension of data, when vectorial data is interpreted as scalar data */
  int dimRange;

  /* index of current component */
  /* for scalar this vec has length 1 and contains the component number */
  /* for vector this contains the number of each component */
  int * comp;

  /* polynonial order of basis functions */
  int polyOrd;

  /* continuous or not */
  int continuous;

  /* max number of components */
  int compName;

  /* the corresponding gridPart */
  void * gridPart;

  void (*setGridPartIterators)(DUNE_DAT * , void * gridPart);
};

/**********************************************************************
 *  * storage of the dune data ( discrete functions )
 *   *
 *    * normaly stored as function_data in the F_DATA pointer
 *     **********************************************************************/
typedef struct dune_func
{
  /* name */
  const char * name;
  /* the function to evaluate */
  void (* func_real)(DUNE_ELEM *he, DUNE_FDATA *fe, int ind,
                     double G_CONST *coord, double *val);
  /* struct storing the pointer to the disrete function */
  DUNE_FDATA * all;
} DUNE_FUNC;




struct dune_dat
{
  // default constructor
  dune_dat()
    : first_macro(0)
      , next_macro(0)
      , delete_iter(0)
      , first_child(0)
      , next_child(0)
      , copy(0)
      , check_inside(0)
      , wtoc(0)
      , ctow(0)
      , setIterationModus(0)
      , partition(-1)
      , iteratorType(-1) // g_LeafIterator
      , partitionIteratorType(-1)
      , gridPart(0)
      , all (0) {}


  /* the actual first and next macro for Iteration  */
  int (* first_macro)(DUNE_ELEM *) ;
  int (* next_macro)(DUNE_ELEM *) ;

  /* method to delete iterators */
  void (* delete_iter)(DUNE_ELEM *) ;

  /* first and next child , if 0, then no child iteration */
  int (* first_child)(DUNE_ELEM *) ;
  int (* next_child)(DUNE_ELEM *) ;

  void * (* copy)(const void *) ;

  int (* check_inside)(DUNE_ELEM *, const double * ) ;
  int (* wtoc)(DUNE_ELEM *, const double *, double * ) ;
  void (* ctow)(DUNE_ELEM *, const double *, double * ) ;

  /* selects the iterators, like leaf iterator .. */
  void (* setIterationModus)(DUNE_DAT *, DUNE_FUNC *);

  /* to which processor partition the element belongs */
  int partition;

  /* type of choosen iterator */
  int iteratorType;

  /* type of partition to iterate */
  int partitionIteratorType;

  /* actual gridPart */
  void * gridPart;

  DUNE_ELEM * all;
};


/* setup hmesh with given data */
extern void *setupHmesh(
  void (* const func_real) (DUNE_ELEM *, DUNE_FDATA*, int ind, const double *coord,  double *),
  const int noe, const int nov, const int maxlev,
  DUNE_FDATA * fe, DUNE_DAT * dune);
/* delete given hmesh pointer */
extern void deleteHmesh( void * hmesh );

extern void displayTimeScene(INFO * info);
extern void handleMesh (void *hmesh, bool gridMode );

extern DUNE_FDATA * extractData (void *hmesh , int num );

/* setup TimeScene Tree  */
extern void timeSceneInit(INFO *info, int n_info, int procs , int time_bar);
extern void addDataToHmesh(void  *hmesh, DUNE_FDATA * fe,
                           void (* const func_real) (DUNE_ELEM *, DUNE_FDATA*, int ind, const double *, double *)  );
extern void addHmeshToTimeScene(void * timescene, double time, void  *hmesh , int proc);

extern void addHmeshToGlobalTimeScene(double time, void  *hmesh , int proc);
extern void tsc_timebar(void *timescene, double t_start, double t_end);
extern void colorBarMinMax(const double min, const double max);

#endif
