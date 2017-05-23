
#include "Logger.hpp"

#include <ctime>
#include <iostream>
#include <fstream>
#include <mutex>

//==========================================================================================================================================
//
//  This file is part of simpleNewton. simpleNewton is free software: you can 
//  redistribute it and/or modify it under the terms of the GNU General Public
//  License as published by the Free Software Foundation, either version 3 of 
//  the License, or (at your option) any later version.
//  
//  simpleNewton is distributed in the hope that it will be useful, but WITHOUT 
//  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or 
//  FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License 
//  for more details.
//  
//  You should have received a copy of the GNU General Public License along
//  with simpleNewton (see LICENSE.txt). If not, see <http://www.gnu.org/licenses/>.
//
///   Contains the implementation of Logger class.
///   \file
///   \addtogroup core Core
///   \author Nitin Malapally (anxiousprogrammer) <nitin.malapally@gmail.com>
//
//==========================================================================================================================================

/** The space in which all global entities of the framework are accessible */
namespace simpleNewton {

/** The buffer must be flushed after streaming to ensure output.
*
*   \param write   A flag which decides whether or not the buffer will be written to process' log file.
*/
void Logger::flushBuffer( flag_t _write ) {

   if( _write && ! buffer_.str().empty() )
      writeLog();
   
   static std::mutex proc_cout_mutex;
   std::lock_guard< std::mutex > cout_lock( proc_cout_mutex );   // std::cout is also a shared, 'external' resource.
   std::cout << buffer_.str() << std::endl;
   buffer_.str( std::string() );
}



/** This function writes the buffer to process' log file with name, "<executable_name>_log_on_proc<process_rank>". */
void Logger::writeLog() {
   
   static std::mutex proc_file_mutex;   // Looks like lazy initialization; mutex is shared resource.
   static std::ofstream file;           // owned by process, not threads.
   
   std::lock_guard< std::mutex > file_lock( proc_file_mutex );   // Thread safety
   
   file.open( ProcSingleton::getExecName() + "_log_on_proc" + std::to_string( SN_MPI_RANK() ), std::ios_base::app );
   if( ! file.is_open() ) {
   
      real_t time_point = ProcSingleton::getDurationFromStart() * real_cast(1e+3);
      fixFP(2);
      std::cerr << "[" << time_point << " ms][LOGGER__>][P" << SN_MPI_RANK()
                << "][FILE ERROR ]:   Could not open the log file for logger. "
                << "The program will continue but the buffer was not written to process' log file in this instance.";
      unfixFP();
      
      return;
   }
   
   time_t _now = time(nullptr);
   try {
      file << std::endl << ctime( &_now) << buffer_.str() << std::endl << std::endl;
   } 
   catch( const std::exception & ex ) {

      real_t time_point = ProcSingleton::getDurationFromStart() * real_cast(1e+3);
      fixFP(2);
      std::cerr << "[" << time_point << " ms][LOGGER__>][P" << SN_MPI_RANK()
                << "][EXCEPTION CAUGHT ]: A standard exception was caught during the writing of the log file. "
                << "The program will continue but the buffer was not written to process' log file in this instance. "
                << ex.what() << std::endl;
      unfixFP();
      
      throw;
   }
   file.close();
}



/** The destructor flushes the buffer with an order to write to process' log file. */
Logger::~Logger() {

   flushBuffer( true );
}



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//   MACRO function implementation
///////////////////////////////////

#ifndef DOXYGEN_SHOULD_SKIP_THIS
namespace logger {
namespace impl {

bool eventWatchRegionSwitch_[] = { false, false };   // Here's a global!


void print_message( const std::string & msg ) {

   Logger lg;
   real_t time_point = ProcSingleton::getDurationFromStart() * real_cast(1e+3);
   lg.fixFP(2);
   lg << "[" << time_point << " ms][LOGGER__>][P" << SN_MPI_RANK() << "][MESSAGE ]:   " << msg << '\n';
   lg.unfixFP();
   lg.flushBuffer( false );
}



void report_error( const std::string & msg, const std::string & file, int line, const std::string & func ) {

   Logger lg;
   real_t time_point = ProcSingleton::getDurationFromStart() * real_cast(1e+3);
   lg.fixFP(2);
   lg << "[" << time_point << " ms][LOGGER__>][P" << SN_MPI_RANK() << "][ERROR ]:   " << msg << '\n' 
      << ">--- From function, " << func << " <" << file << " :" << line << " > ---<" << '\n';
   lg.unfixFP();
   lg.flushBuffer( true );
}



void catch_exception( const std::exception & exc, const std::string & file, int line, const std::string & func ) {

   Logger lg;
   real_t time_point = ProcSingleton::getDurationFromStart() * real_cast(1e+3);
   lg.fixFP(2);
   lg << "[" << time_point << " ms][LOGGER__>][P" << SN_MPI_RANK() << "][EXCEPTION CAUGHT ]:   " << exc.what() << '\n'
      << ">--- From function, " << func << " <" << file << " :" << line << " > ---<" << '\n';
   lg.unfixFP();
   lg.flushBuffer( true );
}



void report_warning( const std::string & msg, const std::string & file, int line, const std::string & func ) {

   Logger lg;
   real_t time_point = ProcSingleton::getDurationFromStart() * real_cast(1e+3);
   lg.fixFP(2);
   lg << "[" << time_point << " ms][LOGGER__>][P" << SN_MPI_RANK() << "][WARNING ]:    " << msg << '\n'
      << ">--- From function, " << func << " <" << file << " :" << line << " > ---<" << '\n';
   lg.unfixFP();
      
   #ifdef __SN_LOGLEVEL_WRITE_WARNINGS__
      lg.flushBuffer( true );
   #else
      lg.flushBuffer( false );
   #endif
}



void markEventHorizon( const uint_t eventLevel ) {
   eventWatchRegionSwitch_[ eventLevel ] = ! eventWatchRegionSwitch_[ eventLevel ];
}
void report_L1_event( LogEventType event, const std::string & file, int line, const std::string & func, const std::string & info ) {
   
   if( ! eventWatchRegionSwitch_[0] )
      return;
   
   Logger lg;
   
   std::string event_tag = "N/A";
   std::string descr = "N/A";

   switch( event ) {
      case LogEventType::ResAlloc: event_tag = "HEAP RESOURCE ALLOCATED"; descr = "Pointer, Type, Size (in that order): "; break;
         
      case LogEventType::ResDealloc: event_tag = "HEAP RESOURCE DEALLOCATED"; descr = "Pointer, Type, Size (in that order): "; break;
         
      case LogEventType::OMPFork: event_tag = "OMP PARALLEL REGION ENTERED"; descr = "OpenMP fork occurred. "; break;
         
      case LogEventType::OMPJoin: event_tag = "OMP PARALLEL REGION EXITED";  descr = "OpenMP operation synchronized. "; break;
         
      case LogEventType::MPISend: event_tag = "MPI COMMUNICATION (SEND)"; descr = "Package, source, target (in that order): "; break;
         
      case LogEventType::MPISsend: event_tag = "MPI COMMUNICATION (SYNC. SEND)"; 
                                   descr = "Package, source, target (in that order): "; break;
         
      case LogEventType::MPIIsend: event_tag = "MPI COMMUNICATION (ISEND)"; descr = "Package, source, target (in that order): "; break;
         
      case LogEventType::MPIRecv: event_tag = "MPI COMMUNICATION (RECV)"; descr = "Package, source, target (in that order): "; break;
        
      case LogEventType::MPIIrecv: event_tag = "MPI COMMUNICATION (IRECV)"; descr = "Package, source, target (in that order): "; break;
         
      case LogEventType::MPIBcast: event_tag = "MPI COMMUNICATION (BCAST)"; descr = "Package, source (in that order): "; break;
         
      case LogEventType::MPIWait: event_tag = "MPI COMMUNICATION (WAIT)"; 
                                  descr = "An immediate MPI operation has been completed."; break;
         
      case LogEventType::MPIWaitAll: event_tag = "MPI COMMUNICATION (WAITALL)"; 
                                     descr = "A set of immediate MPI operations has been completed."; break;
         
      case LogEventType::Other: event_tag = "SPECIAL EVENT"; descr = "A special event has occurred.";
         
      default: event_tag = "UNKNOWN EVENT"; descr = "An unspecified event has ocurred"; break;
   }
   
   real_t time_point = ProcSingleton::getDurationFromStart() * real_cast(1e+3);
   lg.fixFP(2);
   lg << "[" << time_point << " ms][LOGGER__>][P" << SN_MPI_RANK() << "][L1 EVENT - " << event_tag << " ]:   " 
      << descr << "   " << info << '\n' 
      << ">--- From function, " << func << " <" << file << " :" << line << " > ---<" << '\n';
   lg.unfixFP();
   
   #ifdef __SN_LOGLEVEL_WRITE_EVENTS__
      lg.flushBuffer( true );
   #else
      lg.flushBuffer( false );
   #endif
}
void report_L2_event( const std::string & file, int line, const std::string & func, const std::string & event_tag, 
                      const std::string & descr ) {
   
   if( ! eventWatchRegionSwitch_[1] )
      return;
   
   Logger lg;
   
   real_t time_point = ProcSingleton::getDurationFromStart() * real_cast(1e+3);
   lg.fixFP(2);
   lg << "[" << time_point << " ms][LOGGER__>][P" << SN_MPI_RANK() << "][L2 EVENT - " << event_tag << " ]:   " 
      << descr << '\n' 
      << ">--- From function, " << func << " <" << file << " :" << line << " > ---<" << '\n';
   lg.unfixFP();
   
   #ifdef __SN_LOGLEVEL_WRITE_EVENTS__
      lg.flushBuffer( true );
   #else
      lg.flushBuffer( false );
   #endif
}



void watch_impl( Logger & ) {}

}   // namespace impl
}   // namespace logger
#endif   //DOXYSKIP

}   // namespace simpleNewton
