/*****************************************************************************
 * Zoltan Library for Parallel Applications                                  *
 * Copyright (c) 2000,2001,2002, Sandia National Laboratories.               *
 * For more info, see the README file in the top-level Zoltan directory.     *
 *****************************************************************************/
/*****************************************************************************
 * CVS File Information :
 *    $RCSfile$
 *    $Author$
 *    $Date$
 *    $Revision$
 ****************************************************************************/

#ifdef __cplusplus
/* if C++, define the rest of this header file as extern C */
extern "C" {
#endif

#include <math.h>
#include "zz_const.h"
#include "zz_util_const.h"
#include "matrix.h"
#include "graph.h"

#define CHECK_IERR do {   if (ierr != ZOLTAN_OK || ierr != ZOLTAN_WARN) \
    goto End;  } while (0)


  /* This function needs a distribution : rows then cols to work properly */

int
ZG_Build (ZZ* zz, ZG* graph, int bipartite, int fixObj)
{
  static char *yo = "ZG_Build";
  int ierr = ZOLTAN_OK;

  memset (graph, 0, sizeof(ZG));

  graph->mtx.comm = (PHGComm*)ZOLTAN_MALLOC (sizeof(PHGComm));
  if (graph->mtx.comm == NULL) MEMORY_ERROR;

  ierr = Zoltan_Matrix_Build(zz, &graph->mtx.mtx);
  CHECK_IERR;
  if (bipartite) {
    ierr = Zoltan_Matrix_Bipart(zz, &graph->mtx.mtx, zz->Num_Proc, zz->Proc);
    CHECK_IERR;
  }
  ierr = Zoltan_Distribute_LinearY(zz, graph->mtx.comm);
  CHECK_IERR;
  ierr = Zoltan_Matrix2d_Distribute (zz, graph->mtx.mtx, &graph->mtx);
  CHECK_IERR;

  if (bipartite) {
    int vertlno;
    int limit;
    int offset;

    graph->bipartite = 1;
    graph->fixed_vertices = (int*) ZOLTAN_MALLOC(graph->mtx.mtx.nY*sizeof(int));
    if (graph->mtx.mtx.nY && graph->fixed_vertices == NULL) MEMORY_ERROR;
    limit = graph->mtx.mtx.offsetY;
    graph->fixObj = fixObj;

    offset = graph->mtx.mtx.offsetY - graph->mtx.dist_y[graph->mtx.comm->myProc_y];
    if (fixObj)
      for (vertlno = 0 ; vertlno < graph->mtx.mtx.nY ; ++ vertlno)
	graph->fixed_vertices[vertlno] = (vertlno < offset);
    else
      for (vertlno = 0 ; vertlno < graph->mtx.mtx.nY ; ++ vertlno)
	graph->fixed_vertices[vertlno] = (vertlno >= offset);
  }

 End:
  return (ierr);
}


  /* This function may work on any distribution of the bipartite graph */
int
ZG_Register(ZZ* zz, ZG* graph, int* properties)
{
  static char *yo = "ZG_Register";
  int ierr = ZOLTAN_OK;
  int *props;
  struct Zoltan_DD_Struct *dd;
  int size;
  ZOLTAN_ID_PTR GID;

  size = graph->mtx.mtx.nY;
  dd = graph->mtx.mtx.ddY;

  if (graph->bipartite) { /* Need to construct another properties array with only the fixed elements ! */
    int vertlno;

    if (graph->fixObj) {
      dd = graph->mtx.mtx.ddX;
    }
    props = (int*)ZOLTAN_MALLOC(sizeof(int)*size);
    if (graph->mtx.mtx.nY  && props == NULL) MEMORY_ERROR;
    GID = ZOLTAN_MALLOC_GID_ARRAY(zz, size);
    if (size && GID == NULL) MEMORY_ERROR;
    for (size = 0, vertlno = 0 ; vertlno < graph->mtx.mtx.nY ; ++vertlno) {
      if (graph->fixed_vertices[vertlno]) {
	props[size] = properties[vertlno];
	ZOLTAN_SET_GID(zz, GID+ size*zz->Num_GID,
		       graph->mtx.mtx.yGID+vertlno*zz->Num_GID);
	size ++;
      }
    }
  }
  else {
    props = properties;
    GID = graph->mtx.mtx.yGID;
  }
  /* Make our new numbering public */
  ierr = Zoltan_DD_Update (dd, GID, NULL, NULL, props, size);

  End:
  if (graph->bipartite) {
    ZOLTAN_FREE(&props);
    ZOLTAN_FREE(&GID);
  }
  return (ierr);
}


/* This function may work with any distribution of the bipartite graph */
int
ZG_Query (ZZ* zz, ZG* graph,
		    ZOLTAN_ID_PTR GID, int GID_length, int* properties)
{
  struct Zoltan_DD_Struct *dd;

  dd = graph->mtx.mtx.ddY;
  if (graph->bipartite && graph->fixObj)
    dd = graph->mtx.mtx.ddX;

  return Zoltan_DD_Find(dd, GID, NULL, NULL, properties, GID_length, NULL);
}

void
ZG_Free(ZZ *zz, ZG *graph){
  /* TODO : free the communicators properly */

  if (graph->bipartite)
    ZOLTAN_FREE(&graph->fixed_vertices);

  Zoltan_Matrix_Free(zz, &graph->mtx.mtx);
  Zoltan_Matrix2d_Free(zz, &graph->mtx);



}





#ifdef __cplusplus
}
#endif
