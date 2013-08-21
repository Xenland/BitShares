#include <bts/blockchain/blockchain_time_keeper.hpp>
#include <iostream>
#include <fc/uint128.hpp>
#include <fc/time.hpp>
#include <fc/exception/exception.hpp>
#include <fc/log/logger.hpp>
#include <sstream>
#include <iomanip>

#include <stdlib.h>


int main( int argc, char** argv )
{
   try {
      int window = 64;
      fc::time_point_sec origin   = fc::time_point::now();
      fc::microseconds   interval = fc::seconds( 60*5 );
      std::cout << "origin: "<< std::string(fc::time_point(origin)) <<" \n";
      std::cout << "interval: "<< interval.count() <<" us \n";
      std::cout << "initial difficulty: "<< std::string(fc::uint128(-1)) <<" \n";
      std::cout << "window: "<< window <<" \n";
      bts::blockchain::time_keeper tk;
      tk.configure( origin, interval, window );


      uint64_t base_diff = 1000000;

      uint32_t block_num = 0;
      
      for( uint32_t i = 0; i < 1; ++i )
      {
         tk.push_init( block_num, 
                  fc::time_point(origin) + fc::microseconds(interval.count() * i),
                  base_diff );
         ++block_num;
      }
      tk.init_stats();

      fc::time_point sim_time = tk.next_time();
      auto next_diff = tk.next_difficulty();
      if( next_diff < 5*60*1000000 ) next_diff = 5*60*1000000;

      int64_t maxerr = 0;
      int64_t minerr = 0;

      // randomly find blocks +/- 2 hours from expected time... 
      //   at the expected difficulty... this should result in
      //   an average difficulty that is about the same.
      for( uint64_t i = 0; true || i < 10000000; ++i )
      {

         tk.push( block_num, sim_time, next_diff  );
         next_diff = tk.next_difficulty();
    //     if( next_diff < 60*1000000 ) next_diff = 60*1000000;
         if( next_diff <= 0 ) next_diff = 1;
         ++block_num;

   //      if( block_num % 100 == 0 )
         {
      //     std::cerr<<" "<<(tk.current_time() - origin).count()/1000000<<", ";
       //    std::cerr<<" "<<(sim_time - origin).count()/1000000<<"\n";
           
           std::cout<< "block_num: "<<std::setw(10)<<block_num;
           std::cout<<"  cur_time:  "<<std::setw(11)<<std::string(tk.current_time());
           std::cout<<"  next_time: "<<std::setw(11)<<std::string(tk.next_time());
           std::cout<<"  delta_time: "<<std::right<<std::setw(14)<<(tk.next_time()-tk.current_time()-interval).count()/1000000<<" sec";
           std::cout<<"  cur_diff:  "<<std::right<<std::setw(14)<<tk.current_difficulty();
           std::cout<<"  next_diff: "<<std::right<<std::setw(14)<<next_diff;
           std::cout<<"  delta: "<<std::right<<std::setw(14)<<int64_t(next_diff-tk.current_difficulty());
           std::cout<<"  sim_time: "<<std::right<<std::setw(14)<<std::string(sim_time)<<" ";
           std::cout<<"  sim-cur:  "<<std::right<<std::setw(14)<<(sim_time-tk.current_time()).count()/1000000<<" ";
           std::cout<<"  timeerr:  "<<std::right<<std::setw(14)<<tk.current_time_error()<<"  ";
           std::cout<<"\n";
           if( tk.current_time_error() < minerr ) 
           {
             minerr = tk.current_time_error();
             ilog( "               MIN: ${m}", ("m",minerr) );
           }
           if( tk.current_time_error() > maxerr )
           {
             maxerr = tk.current_time_error();
             ilog( "MAX: ${m}", ("m",maxerr) );
           }
         }
         if( block_num == 10000 )
         {  
            std::cout<<"\n.... skip 4 hours ... \n";
            sim_time += fc::seconds(4*60*60); // off by 1 hour...
         }
         
         uint64_t a = rand();
         uint64_t b = rand();
         auto sample_1 = uint64_t(a*rand()) % next_diff;
         auto sample_2 = uint64_t(b*rand()) % next_diff;
         sim_time += fc::microseconds( sample_1 + sample_2 );
      }




      std::cout << "current_time:          "<< std::string( tk.current_time() ) <<"\n";
      std::cout << "next_time:             "<< std::string( tk.next_time() ) <<"\n";
      std::cout << "current_difficulty:    "<< tk.current_difficulty()  <<"\n";
      std::cout << "next_difficulty:       "<< tk.next_difficulty()  <<"\n";
      
      std::string line;
      std::getline( std::cin, line );
      while( std::cin )
      {
         std::stringstream ss(line);

          std::string time_str;
          uint64_t pow;
         ss >> time_str >> pow;

         tk.push( block_num, fc::time_point::from_iso_string(time_str), pow  );
         ++block_num;

         std::cout << "current_time:          "<< std::string( tk.current_time() ) <<"\n";
         std::cout << "next_time:             "<< std::string( tk.next_time() ) <<"\n";
         std::cout << "current_difficulty:    "<<  tk.current_difficulty()  <<"\n";
         std::cout << "next_difficulty:       "<<  tk.next_difficulty()  <<"\n";
         std::getline( std::cin, line );
      }


      return 0;
   } catch ( const fc::exception& e )
   {
     std::cout << e.to_detail_string() <<"\n";
   }
   return -1;
}
