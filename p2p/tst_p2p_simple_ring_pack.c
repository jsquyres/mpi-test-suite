/*
 * File: tst_p2p_simple_ring.c
 *
 * Functionality:
 *  Simple point-to-point ring-communication test using MPI_Send and MPI_Recv starting
 *  with process zero.
 *  Works with intra-communicators and up to now with any C (standard and struct) type.
 *
 * Author: Rainer Keller
 *
 * Date: Aug 8th 2003
 */
#include "mpi.h"
#include "mpi_test_suite.h"
#include "tst_output.h"

#undef DEBUG
#define DEBUG(x)

/*
 * XXX
static char * send_buffer = NULL;
static char * send_pack_buffer = NULL;
static char * recv_buffer = NULL;
static char * recv_pack_buffer = NULL;
static int position = 0;
 */

int tst_p2p_simple_ring_pack_init (struct tst_env * env)
{
  int comm_rank;
  MPI_Comm comm;
  MPI_Datatype type;
  int size;

  tst_output_printf (DEBUG_LOG, TST_REPORT_MAX, "(Rank:%d) env->comm:%d env->type:%d env->values_num:%d\n",
                 tst_global_rank, env->comm, env->type, env->values_num);
  /*
   * Now, initialize the send_buffer
   */
  env->send_buffer = tst_type_allocvalues (env->type, env->values_num);
  comm = tst_comm_getcomm (env->comm);
  MPI_CHECK (MPI_Comm_rank (comm, &comm_rank));
  tst_type_setstandardarray (env->type, env->values_num, env->send_buffer, comm_rank);
  type = tst_type_getdatatype (env->type);
  MPI_Pack_size(env->values_num, type, comm, &size);
  env->send_pack_buffer = (char *)malloc(size);
  /*
   * Now, pack the send_buffer in send_pack_buffer
   */
  env->position = 0;
  MPI_Pack(env->send_buffer, env->values_num, type, env->send_pack_buffer, size, &env->position, comm);

  env->recv_buffer = tst_type_allocvalues (env->type, env->values_num);
  env->recv_pack_buffer = (char *)malloc(size);

  return 0;
}

int tst_p2p_simple_ring_pack_run (struct tst_env * env)
{
  int comm_size;
  int comm_rank;
  int send_to;
  int recv_from;
  int recv_count;
  int size;
  MPI_Comm comm;
  MPI_Datatype type;
  MPI_Status status;

  comm = tst_comm_getcomm (env->comm);
  type = tst_type_getdatatype (env->type);

  if (tst_comm_getcommclass (env->comm) & TST_MPI_COMM_SELF)
    {
      comm_size = 1;
      comm_rank = 0;
      send_to = MPI_PROC_NULL;
      recv_from = MPI_PROC_NULL;
    }
  else if (tst_comm_getcommclass (env->comm) & TST_MPI_INTRA_COMM)
    {
      MPI_CHECK (MPI_Comm_rank (comm, &comm_rank));
      MPI_CHECK (MPI_Comm_size (comm, &comm_size));

      if (comm_size > 1)
        {
          send_to = (comm_rank + 1) % comm_size;
          recv_from = (comm_rank + comm_size - 1) % comm_size;
        }
      else
        {
          send_to = MPI_PROC_NULL;
          recv_from = MPI_PROC_NULL;
        }
    }
  else
    ERROR (EINVAL, "tst_p2p_simple_ring cannot run with this kind of communicator");

  tst_output_printf (DEBUG_LOG, TST_REPORT_MAX, "(Rank:%d) comm_rank:%d comm_size:%d "
                 "send_to:%d recv_from:%d env->tag:%d\n",
                 tst_global_rank, comm_rank, comm_size,
                 send_to, recv_from, env->tag);

  if (comm_rank == 0)
    {
      MPI_CHECK (MPI_Send (env->send_pack_buffer, env->position, MPI_PACKED, send_to, env->tag, comm));
      MPI_CHECK (MPI_Recv (env->recv_pack_buffer, env->position, MPI_PACKED, recv_from, env->tag, comm, &status));

    }
  else
    {
       MPI_CHECK (MPI_Recv (env->recv_pack_buffer, env->position, MPI_PACKED, recv_from, env->tag, comm, &status));
       MPI_CHECK (MPI_Send (env->send_pack_buffer, env->position, MPI_PACKED, send_to, env->tag, comm));
    }

  if (status.MPI_SOURCE != recv_from ||
      (recv_from != MPI_PROC_NULL && status.MPI_TAG != env->tag) ||
      (recv_from == MPI_PROC_NULL && status.MPI_TAG != MPI_ANY_TAG))
    ERROR (EINVAL, "Error in status");



  if (recv_from != MPI_PROC_NULL)
    {
      int unpack_position = 0;
      MPI_Pack_size(env->values_num, type, comm, &size);
      MPI_Unpack(env->recv_pack_buffer, size, &unpack_position, env->recv_buffer, env->values_num, type, comm );
      if (tst_mode == TST_MODE_STRICT)
        {
          MPI_CHECK(MPI_Get_count(&status, type, &recv_count));
          if(recv_count != env->values_num)
            ERROR(EINVAL, "Error in Count");
        }
      tst_test_checkstandardarray (env, env->recv_buffer, recv_from);
    }
  return 0;
}

int tst_p2p_simple_ring_pack_cleanup (struct tst_env * env)
{
  tst_type_freevalues (env->type, env->send_buffer, env->values_num);
  tst_type_freevalues (env->type, env->recv_buffer, env->values_num);
  free (env->send_pack_buffer);
  free (env->recv_pack_buffer);
  return 0;
}
