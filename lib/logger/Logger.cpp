#include "Logger.hpp"

namespace simpleNewton {

/**||***************************************************************************************************************************************
*
*   Description: Basic logger module for the SimpleNewton framework 
*
|***************************************************************************************************************************************///+

void Logger::flushBuffer( flag_t _write ) {
   std::cout << buffer_.str() << std::endl;
   if( _write )
      writeLog();
   buffer_.str( std::string() );
}

void Logger::writeLog() {
   if( ! Logger::getInstance().streamSet_ ) {
      auto timep = std::chrono::high_resolution_clock::now();
      real_t point = std::chrono::duration_cast< std::chrono::microseconds >(timep - Logger::getStartTime()).count() * 0.001f;
      std::cout << "["<< point 
                << "][LOGGER__>][ERROR ]:   Logger attempted to write to no destination. Please set write settings first." << std::endl;
      std::abort();
   }
   
   if( Logger::getInstance().buffer_.str().empty() ) {
      return;
   }
   
   Logger::getInstance().file_guard_.lock();
   static_cast< std::ofstream & >( (*Logger::getInstance().file_) ).open( "log_" + Logger::getInstance().exec_.substr(2), 
                                                                                                          std::ios_base::app );
   if( ! static_cast< std::ofstream & >( (*Logger::getInstance().file_) ).is_open() ) {
      auto timep = std::chrono::high_resolution_clock::now();
      real_t point = std::chrono::duration_cast< std::chrono::microseconds >(timep - Logger::getStartTime()).count() * 0.001f;
      std::cout << "[" << point << "][LOGGER__>][ERROR ]:   Could not open the log file for logger.";
      std::abort();
   }
   
   time_t _now = time(nullptr);
   try {
   (*Logger::getInstance().file_) << ctime( &_now) << Logger::getInstance().buffer_.str() << std::endl << std::endl;
   } 
   catch( std::exception & ex ) {
      auto timep = std::chrono::high_resolution_clock::now();
      real_t point = std::chrono::duration_cast< std::chrono::microseconds >(timep - Logger::getStartTime()).count() * 0.001f;
      std::cout << "[" << point << "][LOGGER__>][EXCEPTION ]: A standard exception was caught during the writing of the log file. " 
                << ex.what() << std::endl;
   }
   static_cast< std::ofstream & >( (*Logger::getInstance().file_) ).close();
   Logger::getInstance().file_guard_.unlock();
}

Logger::~Logger() {
   flushBuffer( true );
   writeLevel_ = 0;
   
   time_t _now = time(nullptr);
   //Wrap up I/O
   std::cout << ">>>   Program completed execution with end time " << ctime( &_now );
   std::cout << "********************************************************************************************************" << std::endl
             << "========================================================================================================>" << std::endl;
}



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///   Implementation of the macros
//////////////////////////////////

namespace logger {
namespace impl {

void print_message( const char * msg ) {
   auto timep = std::chrono::high_resolution_clock::now();
   real_t point = std::chrono::duration_cast< std::chrono::microseconds >(timep - Logger::getStartTime()).count() * 0.001f;
   Logger::getInstance() << "[" << point << "][LOGGER__>][MESSAGE ]:   " << msg << Logger::nl << Logger::b_end;
}

void report_error( const char * msg, const char * const file, int line ) {
   auto timep = std::chrono::high_resolution_clock::now();
   real_t point = std::chrono::duration_cast< std::chrono::microseconds >(timep - Logger::getStartTime()).count() * 0.001f;
   Logger::getInstance() << "[" << point << "][LOGGER__>][ERROR ]:   " << "<Description>   " << msg
                         << Logger::nl
                         << ">--- From <" << file << " :" << line << " > ---<" << Logger::nl;
   Logger::getInstance() << Logger::b_write;
}



void report_warning( const char * msg, const char * const file, int line ) {
   auto timep = std::chrono::high_resolution_clock::now();
   real_t point = std::chrono::duration_cast< std::chrono::microseconds >(timep - Logger::getStartTime()).count() * 0.001f;
   Logger::getInstance() << "[" << point << "][LOGGER__>][WARNING ]:    " << msg
                         << Logger::nl
                         << ">--- From <" << file << " :" << line << " > ---<" << Logger::nl;
   if( Logger::writeLevel() >= __SN_LOGLEVEL_WARNING__ )
      Logger::getInstance() << Logger::b_write;
   else
      Logger::getInstance() << Logger::b_end;
}



void report_event( LogEventType event, const char * const file, int line, const char * info ) {
   std::string event_tag;
   std::string descr;
   switch( event ) {
   case LogEventType::ResAlloc: event_tag = "RESOURCE ALLOCATED"; descr = "Pointer, Type, Size (in that order): "; break;
   case LogEventType::ResDealloc: event_tag = "RESOURCE DEALLOCATED"; descr = "Pointer, Type, Size (in that order): "; break;
   case LogEventType::OMPFork: event_tag = "OMP PARALLEL REGION ENTERED"; descr = "OpenMP fork occurred. "; break;
   case LogEventType::OMPJoin: event_tag = "OMP PARALLEL REGION EXITED";  descr = "OpenMP operation synchronized. "; break;
   case LogEventType::MPISend: event_tag = "MPI COMMUNICATION (SEND)"; descr = "An MPI_Send operation was completed successfully. "; break;
   case LogEventType::MPIRecv: event_tag = "MPI COMMUNICATION (RECV)"; descr = "An MPI_Recv operation was completed successfully. "; break;
   case LogEventType::MPIBCast: event_tag = "MPI COMMUNICATION (BCAST)"; descr = "An MPI_BCast operation was completed successfully. ";
   break;
   default: event_tag = "UNKNOWN EVENT"; descr = "An unspecified event has ocurred"; break;
   };
   
   auto timep = std::chrono::high_resolution_clock::now();
   real_t point = std::chrono::duration_cast< std::chrono::microseconds >(timep - Logger::getStartTime()).count() * 0.001f;
   Logger::getInstance() << "[" << point << "][LOGGER__>][EVENT - " << event_tag << " ]:   " << descr
                         << Logger::nl << "   " << info << Logger::nl
                         << ">--- From <" << file << " :" << line << " > ---<" << Logger::nl;
   if( Logger::writeLevel() >= __SN_LOGLEVEL_EVENT__ )
      Logger::getInstance() << Logger::b_write;
   else
      Logger::getInstance() << Logger::b_end;
}



void watch_impl() {}

}   // namespace impl
}   // namespace logger

}   // namespace simpleNewton