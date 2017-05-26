#include "ProcSingleton.hpp"

#include <iostream>
#include <ctime>

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
///   Contains the implementation of class ProcSingleton.
///   \file
///   \addtogroup core Core
///   \author Nitin Malapally (anxiousprogrammer) <nitin.malapally@gmail.com>
//
//==========================================================================================================================================

/** The space in which all global entities of the framework are accessible */
namespace simpleNewton {

/** If this function is called without having firstly initialized the ProcSingleton, it results in an error being thrown. Exception safety: 
*   strong exception safety guarantee.
*
*   \return   A const qualified reference to the singleton instance.
*/
const ProcSingleton & ProcSingleton::getInstance() {
   
   if( ! getPrivateInstance().is_initialized_ && ! getPrivateInstance().is_initialized_with_multithreading_ ) {
   
      SN_MPI_ROOTPROC_REGION() {
   
         std::cerr << "[PROCMAN__>][ERROR ]:   MPI Manager was not initialised before being called to service. " 
                   << "The MPI Manager will remain inaccessible during this sitting. "
                   << "The program will continue without MPI functionality."
                   << std::endl;
      }
   }
   return static_cast< const ProcSingleton & >( getPrivateInstance() );
}



/** This function must be called at the beginning of any program making use of the simpleNewton framework.
*
*   \param argc   The number of command arguments.
*   \param argv   A pointer to the command arguments.
*/
void ProcSingleton::init( int argc, char ** argv ) {
   
   if( ! getPrivateInstance().is_initialized_ && ! getPrivateInstance().is_initialized_with_multithreading_ ) {
      
      if( argc < 1 || argv == nullptr ) {   // Killing that -Wunused-parameter warning when not using MPI
         SN_MPI_ROOTPROC_REGION() {
            std::cerr << "[PROCMAN__>][ERROR ]:   Improper initializing arguments provided to ProcSingleton::init." << std::endl;
         }
      }
      
      // Exec name
      getPrivateInstance().exec_name_ = argv[0];
      
      #ifdef __SN_USE_MPI__
      int info = -1;
      
      #ifdef __SN_USE_MULTITHREADING__
      int prov = 0;
      info = MPI_Init_thread( &argc, &argv, MPI_THREAD_SERIALIZED, &prov );
      
      // Was init with thread support successful?
      if( info == MPI_SUCCESS )
         getPrivateInstance().is_initialized_with_multithreading_  = true;
      
      // Full thread support acquired? Maybe a problem with init?
      if( prov != MPI_THREAD_SERIALIZED || info != MPI_SUCCESS ) {
       
         std::cerr << "[PROCMAN__>][ERROR ]:   The MPI Manager could not be initialized with thread support. "
                   << "The program will continue with a single thread."
                   << std::endl;

         info = MPI_Init( &argc, &argv );
      }
      
      #else   // Multithreading
      info = MPI_Init( &argc, &argv );
      #endif
      
      
      if( info != MPI_SUCCESS ) {
         
         std::cerr << "[PROCMAN__>][ERROR ]:   There was an error in initialising the MPI Manager (ProcSingleton). "
                   << "The program will continue without MPI functionality."
                   << std::endl;
      }
      else {
      
         // Fly the flag: all is well!
         getPrivateInstance().is_initialized_  = true;
      
         MPI_Comm_size( MPI_COMM_WORLD, & getPrivateInstance().comm_size_ );
         MPI_Comm_rank( MPI_COMM_WORLD, & getPrivateInstance().comm_rank_ );
      }
      #endif   // Using MPI at all?
      
      // Console initialization
      SN_MPI_ROOTPROC_REGION() {
      
         time_t _now( time(nullptr) );
         std::cout << "========================================================================================================>" 
                   << std::endl
                   << "********************************************************************************************************" 
                   << std::endl
                   << ">>>   Simple Newton framework running \'" << argv[0] 
                   << "\' with " <<  " start time: " << ctime( &_now )
                   << ">>>   Timer resolution : " << ProcTimer::getExactResolution() << " second" << std::endl;
      }
      SN_MPI_BARRIER();
      
      #ifdef __SN_USE_MPI__
      if( getPrivateInstance().is_initialized_with_multithreading_ ) {
         
         SN_MPI_ROOTPROC_REGION() {
            std::cout << "[MPIMAN__>][ROOTPROC][EVENT ]:   MPI has been initialized with thread support." << std::endl << std::endl;
         }
      } else if( getPrivateInstance().is_initialized_ ) {
         
         SN_MPI_ROOTPROC_REGION() {
            std::cout << "[MPIMAN__>][ROOTPROC][EVENT ]:   MPI has been initialized." << std::endl << std::endl;
         }
      }
      #endif
   }
}



/** Multiple modules may require the executable's name for purposes of file writing, among others.
*
*   \return   A string with the name of the executable in it.
*/
const std::string & ProcSingleton::getExecName() {
   return getPrivateInstance().exec_name_;
}



/** Performs cleanup and calls MPI_Finalize(). */
ProcSingleton::~ProcSingleton() {

   SN_MPI_BARRIER();
  
   #ifdef __SN_USE_MPI__
   if( getPrivateInstance().is_initialized_with_multithreading_ || getPrivateInstance().is_initialized_ ) {
      MPI_Finalize();
   }
   #endif
   
   SN_MPI_ROOTPROC_REGION() {

      time_t _now = time(nullptr);
      //Wrap up I/O
      std::cout << "[MPIMAN__>][ROOTPROC][EVENT ]:   MPI has been finalized." << std::endl;
      std::cout << ">>>   Program completed execution with end time " << ctime( &_now );
      std::cout << "********************************************************************************************************" << std::endl
                << "========================================================================================================>" << std::endl;
   }

   exec_name_ = "";
   is_initialized_ = false;
   comm_size_ = 0;
   comm_rank_ = 0;
}



/** Any program which makes use of the simpleNewton framework must initialize the ProcSingleton.
*
*   \return   true if the ProcSingleton has been initialized, false if not.
*/
bool ProcSingleton::isInitialized() {
   return getPrivateInstance().is_initialized_;
}



/** Any program which makes use of the simpleNewton framework with multithreading/OpenMP must initialize the ProcSingleton accordingly.
*
*   \return   true if the ProcSingleton has been initialized with thread support, false if not.
*/
bool ProcSingleton::isInitializedWithMultithreading() {
   return getPrivateInstance().is_initialized_with_multithreading_;
}


}   // namespace simpleNewton
