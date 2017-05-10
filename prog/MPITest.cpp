#include <iostream>

#include <core/ProcSingleton.hpp>
#include <core/Communicator.hpp>
#include <logger/Logger.hpp>

using namespace simpleNewton;

int main( int argc, char ** argv ) {
   
   ProcSingleton::init( argc, argv );
   
   SN_LOG_EVENT_WATCH_REGION_LIMIT();
   
   SN_LOG_REPORT_WARNING( "Hi! Testing the MPI module." );
   
   OpenStringBuffer j     = "Hallo, MPI World!";
   OpenStringBuffer rec_j = "Not arrived";
   MPI_Request r1, r2;
   
   SN_MPI_PROC_REGION( 1 ) {
      Communicator::send( j, SN_ROOT_PROC, MPISendMode::Immediate, &r1 );
      Communicator::waitOnSend( &r1 );
   }
   SN_MPI_PROC_REGION( SN_ROOT_PROC ) {
   
      Communicator::receive( rec_j, 17, 1, MPIRecvMode::Immediate, &r2 );
      Communicator::waitOnRecv( &r2 );
      SN_LOG_WATCH_VARIABLES( "The received value: ", std::string(rec_j), rec_j.getSize() );
   }
   
   return 0;
}
