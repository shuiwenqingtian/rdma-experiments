#pragma once

#include <iostream>

#include <mpi.h>

///
/// macro to deal with MPI errors
///
#define MPI_CHECK( mpi_call )                                           \
  do {                                                                  \
    int retval;                                                         \
    if( (retval = (mpi_call)) != 0 ) {                                  \
      char error_string[MPI_MAX_ERROR_STRING];                          \
      int length;                                                       \
      MPI_Error_string( retval, error_string, &length);                 \
      std::cerr << "MPI call failed: " #mpi_call ": "                   \
                << error_string << "\n";                                \
      exit(1);                                                          \
    }                                                                   \
  } while(0)

///
/// This class sets up MPI for use on a cluster of multicore nodes
/// (aka "locales").
///
/// It assumes you will be running multiple processes per node, and
/// gives each process two IDs ("ranks") and synchronization domains:
///
///  - one that is valid across all processes on all nodes in the job,
///    where processes on the same node have contiguous ranks, and
///
///  - another that is local to each node/locale, to support
///  - node-local barriers that other nodes do not participate in.
///
class MPIConnection {

  // private, modifiable MPI parameters; exposed as const references later
  int rank_;        // global ID of this core/process
  int size_;        // total # cores/processes in job
  int locales_;     // total number of nodes in job
  int locale_;      // global ID of this node
  int locale_rank_; // node-local ID of this core/process
  int locale_size_; // # cores/processes on this node

public:

  // MPI communicators, made public to enable other classes to use them
  MPI_Comm main_communicator_;   // job-wide communicator
  MPI_Comm locale_communicator_; // node-local communicator

  MPIConnection()
    : rank_(-1)
    , size_(-1)
    , locales_(-1)
    , locale_(-1)
    , locale_rank_(-1)
    , locale_size_(-1)
    , rank(rank_)
    , size(size_)
    , ranks(size_)
    , locales(locales_)
    , locale(locale_)
    , locale_rank(locale_rank_)
    , locale_size(locale_size_)
    , locale_ranks(locale_size_)
  {
    ;
  }

  /// Set up MPI communication. This should be called in all processes
  /// before doing anything with this object, either directly or
  /// through the following constructor.
  void init( int * argc_p, char ** argv_p[] );

  // alternative constructor that calls init directly
  MPIConnection( int * argc_p, char ** argv_p[] )
    : rank_(-1)
    , size_(-1)
    , locales_(-1)
    , locale_(-1)
    , locale_rank_(-1)
    , locale_size_(-1)
    , rank(rank_)
    , size(size_)
    , ranks(size_)
    , locales(locales_)
    , locale(locale_)
    , locale_rank(locale_rank_)
    , locale_size(locale_size_)
    , locale_ranks(locale_size_)
  {
    init( argc_p, argv_p );
  }

  /// Tear down MPI communication. Either call this before exiting, or
  /// let the destructor do it for you.
  void finalize();

  /// Destructor warns you if finalize has not yet been called.
  ~MPIConnection() {
    int finalized = 0;
    MPI_CHECK( MPI_Finalized( &finalized ) );
    if( !finalized ) {
      std::cerr << "Warning: you should probably call finalize() before MPIConnection goes out of scope, or you may occassionally see deadlock." << std::endl;
      // try to finalize, but it probably won't work
      finalize();
    }
  }

  /// Synchronize across all processes
  void barrier();

  /// Synchronize across all processes on the local node
  void locale_barrier();

  /// Get hostname of this node
  const char * hostname();
  
  /// const references to MPI parameters
  const int & rank;         // global ID of this core/process
  const int & size;         // total # cores/processes in job
  const int & ranks;        // total # cores/processes in job (alias for size)
  const int & locales;      // total number of nodes in job
  const int & locale;       // global ID of this node
  const int & locale_rank;  // node-local ID of this core/process
  const int & locale_size;  // # cores/processes on this node
  const int & locale_ranks; // # cores/processes on this node (alias for locale_size)

};
