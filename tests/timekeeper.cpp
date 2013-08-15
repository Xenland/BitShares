#include <bts/blockchain/blockchain_time_keeper.hpp>
#include <iostream>
#include <fc/uint128.hpp>
#include <fc/time.hpp>
#include <fc/exception/exception.hpp>
#include <sstream>
#include <iomanip>

#include <stdlib.h>


int main( int argc, char** argv )
{
   try {
      int window = 1024;
      fc::time_point_sec origin   = fc::time_point::now();
      fc::microseconds   interval = fc::seconds( 60*5 );
      std::cout << "origin: "<< std::string(fc::time_point(origin)) <<" \n";
      std::cout << "interval: "<< interval.count() <<" us \n";
      std::cout << "initial difficulty: "<< std::string(fc::uint128(-1)) <<" \n";
      std::cout << "window: "<< window <<" \n";
      bts::blockchain::time_keeper tk;
      tk.configure( origin, interval, window );


      uint64_t base_diff = 100000;

      uint32_t block_num = 0;
      
      for( uint32_t i = 0; i < window; ++i )
      {
         tk.push_init( block_num, 
                  fc::time_point(origin) + fc::microseconds(interval.count() * i),
                  base_diff );
         ++block_num;
      }
      tk.init_stats();

      // randomly find blocks +/- 2 hours from expected time... 
      //   at the expected difficulty... this should result in
      //   an average difficulty that is about the same.
      for( uint32_t i = 0; i < 100000; ++i )
      {
         uint64_t range = 60*1000ll*60ll; // 1hr
         range /= 6; // 2*5 minute block interval
         int64_t error_usec = ((uint64_t(rand())) % (2*range)) -  range; // +/-1hr 
         error_usec *= 1000;
         auto next_diff = tk.next_difficulty();
//         next_diff++;
         next_diff += rand() % 3;

         auto next_time = tk.next_time();
         tk.push( block_num, 
                    next_time + fc::microseconds(error_usec), 
                    next_diff  );
         ++block_num;
         auto block_time = next_time + fc::microseconds(error_usec);

         if( block_num % 1000 == 0 )
         {
           std::cerr<<" "<<(tk.current_time() - origin).count()/1000000<<", ";
           std::cerr<<" "<<(block_time - origin).count()/1000000<<"\n";
           
           std::cout<< "block_num: "<<std::setw(10)<<block_num;
           std::cout<<"  cur_time:  "<<std::setw(11)<<std::string(tk.current_time());
           std::cout<<"  next_time: "<<std::setw(11)<<std::string(tk.next_time());
           std::cout<<"  delta_time: "<<std::right<<std::setw(14)<<(tk.next_time()-tk.current_time()-interval).count()/1000000<<" sec";
           std::cout<<"  cur_diff:  "<<std::right<<std::setw(14)<<tk.current_difficulty();
           std::cout<<"  next_diff: "<<std::right<<std::setw(14)<<tk.next_difficulty();
           std::cout<<"  delta: "<<std::right<<std::setw(14)<<int64_t(tk.next_difficulty()-tk.current_difficulty());
           std::cout<<"\n";
         }
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
