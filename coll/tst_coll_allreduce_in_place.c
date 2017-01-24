/*
 * File: tst_coll_allreduce_in_place.c
 *
 * Functionality:
 *  Simple collective Allreduce test-program with MPI_IN_PLACE argument.
 *  Works with intra- and inter- communicators, MPI_COMM_SELF and up to now with any C (standard and struct) type.
 *
 * Author: Rainer Keller, Jelena Pjesivac-Grbovic
 *
 * Date: Jan 16th 2007
 */
#include <mpi.h>
#include "mpi_test_suite.h"


/*
 * XXX
static char * recv_buffer = NULL;
 */


int tst_coll_allreduce_in_place_init (struct tst_env * env)
{
  tst_output_printf (DEBUG_LOG, TST_REPORT_MAX, "(Rank:%d) env->comm:%d env->type:%d env->values_num:%d\n",
                 tst_global_rank, env->comm, env->type, env->values_num);

/*   send_buffer = tst_type_allocvalues (env->type, env->values_num); */
  env->recv_buffer = tst_type_allocvalues (env->type, env->values_num);

  return 0;
}

int tst_coll_allreduce_in_place_run (struct tst_env * env)
{
  int comm_rank;
  int comm_size;
  MPI_Comm comm;
  MPI_Datatype type;

  comm = tst_comm_getcomm (env->comm);
  type = tst_type_getdatatype (env->type);

  MPI_CHECK (MPI_Comm_rank (comm, &comm_rank));
  MPI_CHECK (MPI_Comm_size (comm, &comm_size));

  tst_type_setstandardarray (env->type, env->values_num, env->recv_buffer, comm_rank);

  tst_output_printf (DEBUG_LOG, TST_REPORT_MAX, "(Rank:%d) comm_size:%d comm_rank:%d\n",
                 tst_global_rank, comm_size, comm_rank);

  tst_output_printf (DEBUG_LOG, TST_REPORT_MAX, "(Rank:%d) Going to Allreduce\n",
                 tst_global_rank);

  MPI_CHECK (MPI_Allreduce (MPI_IN_PLACE, env->recv_buffer, env->values_num, type, MPI_MAX, comm));

  tst_test_checkstandardarray (env, env->recv_buffer, (comm_size - 1));

  return 0;
}

int tst_coll_allreduce_in_place_cleanup (struct tst_env * env)
{
  tst_type_freevalues (env->type, env->recv_buffer, env->values_num);
  return 0;
}
