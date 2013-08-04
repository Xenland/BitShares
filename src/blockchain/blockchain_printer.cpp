#include <bts/blockchain/blockchain_printer.hpp>
#include <bts/blockchain/trx_validation_state.hpp>
#include <fc/reflect/variant.hpp>
#include <fc/io/raw.hpp>
#include <fc/variant_object.hpp>
#include <fc/io/json.hpp>
#include <sstream>
#include <iomanip>

#include <fc/log/logger.hpp>

namespace bts { namespace blockchain {

  std::string print_output( const trx_output& o )
  {
      std::stringstream ss;
      
      switch( o.claim_func )
      {
          case claim_by_signature:
             ss << std::string(o.as<claim_by_signature_output>().owner);
             break;
      }
      return ss.str();
  }

  void pretty_print( std::ostream& out, blockchain_db& db, const trx_num& tn )
  {
     try {
        auto mtrx = db.fetch_trx(tn);
        trx_validation_state state( mtrx, &db, false, tn.block_num - 1 );
        state.validate();
        out << "<table border=1 width=\"100%\">\n";
        out << "<tr>\n";
        out << "<td width=\"33%\" valign=\"top\" padding=10>\n";
        out << "<ol>\n";
        for( uint32_t i = 0; i < state.inputs.size(); ++i )
        {
           out << "<li>\n";
           out << "<div>" << state.inputs[i].output.amount << " " << fc::variant( state.inputs[i].output.unit ).as_string();
           out << "   " << fc::variant(state.inputs[i].output.claim_func).as_string();
           out << "</br>\n   " << fc::variant(state.inputs[i].dividends).as_string() << " dividends";
           out << "</br>\n   " << fc::variant(state.inputs[i].dividend_fees).as_string() << " dividend fees";
           out << "</br>\n   Source: Block#  "<<state.inputs[i].source.block_num 
                                 << " Trx # " <<state.inputs[i].source.trx_idx <<"\n"
                                 << " Out # " << uint32_t(state.inputs[i].output_num) <<"\n";
           out << "</div>\n</li>\n";
        }
        out << "</ol>\n";
        out << "</td>\n";
        out << "<td width=\"33%\" align=\"right\" valign=\"top\" padding=10>\n";
        out << "<ol>\n";
        for( uint32_t i = 0; i < state.trx.outputs.size(); ++i )
        {
           out << "<li>\n";
           out << "<div>\n";
           out << state.trx.outputs[i].amount << " " << fc::variant( state.trx.outputs[i].unit ).as_string();
           out << "  <br/>" << fc::variant(state.trx.outputs[i].claim_func).as_string() <<"  ";
           out << "  <br/>\n" << print_output( state.trx.outputs[i] ) <<" \n";
           out << "  <br/>\n"; 
           if( mtrx.meta_outputs[i].is_spent() )
           {
              out << "SPENT Block #"<< mtrx.meta_outputs[i].trx_id.block_num;
              out << " Trx #"<< mtrx.meta_outputs[i].trx_id.trx_idx;
              out << " In  #"<< uint32_t(mtrx.meta_outputs[i].input_num);
           }
           out << "</div></li>\n";
        }

        out << "</ol>\n";
        out << "</td>\n";
        out << "<td valign=\"top\">\n";
        out << "<table width=\"100%\"><tr><th width=\"50%\" padding=10>In</th><th padding=10 width=\"50%\">Out</th></tr>\n";
        for( uint32_t i = 0; i < state.balance_sheet.size(); ++i )
        {
           if( state.balance_sheet[i].in.amount != 0 || state.balance_sheet[i].out.amount != 0  )
           {
              out <<"<tr>\n";
              out <<"<td>"<< std::string(state.balance_sheet[i].in)<<"</td>";
              out <<"<td>"<< std::string(state.balance_sheet[i].out)<<"</td>";
              out <<"</tr>";
           }
           if( state.balance_sheet[i].in.amount > state.balance_sheet[i].out.amount   )
           {
              out << "<tr><td colspan=2><hr/><br/> Fees: "<<std::string(state.balance_sheet[i].in - state.balance_sheet[i].out)<<"</td></tr>\n";
           }
        }
        out << "</table>\n";
        out << "</td>\n";
        out << "</tr>\n";
        out << "</table>\n";

     /*
      wlog( "==============================    TRX: ${t}", ("t",tn) );

        out << "INPUTS\n";
        out << "OUTPUTS\n";
        out << "SUMMARY\n";
      wlog( "==============================" );
      */

     } 
     catch ( const fc::exception& e )
     {
        out << e.to_detail_string();
        throw;
     }

#if 0
     auto mtrx = db.fetch_trx( tn );   
     out << "=====  INPUTS   ============================ trx: " 
         << std::right << std::setw(3) << tn.trx_idx << " - " << fc::variant(mtrx.id()).as_string().substr(0,6) 
         <<" ============================  OUTPUTS  ==== \n";

     asset total_in( 0, asset::bts );
     asset total_out( 0, asset::bts );
     asset total_div_fee( 0, asset::bts );
     std::vector<meta_trx_input> metain = db.fetch_inputs(mtrx.inputs);
     uint32_t mx = std::max( mtrx.inputs.size(), mtrx.outputs.size() );
     for( uint32_t i = 0; i < mx; ++i )
     {
          if( i < mtrx.inputs.size() )
          {
              // from
              out << std::left << std::setw( 20 ) << 
                  fc::variant( metain[i].output.amount ).as_string() + 
                      " " + fc::variant(metain[i].output.unit).as_string();
              out << std::right << std::setw(30) << 
                   fc::format_string( "from B${block} T${trx_idx} Out# ${out}  ",
                              fc::mutable_variant_object( "block", metain[i].source.block_num )
                                                        ( "trx_idx", metain[i].source.trx_idx )
                                                        ( "out", metain[i].output_num ) );
              total_in += asset( metain[i].output.amount, metain[i].output.unit );
          }
          else
          {
              out << std::setw( 50 ) <<" ";
          }
          out << "| ";

          if( i < mtrx.outputs.size() )
          {
              out << std::left << std::setw( 29 ) <<
                    fc::variant( mtrx.outputs[i].claim_func ).as_string();
              out << " ";
              out << std::right << std::setw( 20 ) << 
                  fc::variant( mtrx.outputs[i].amount ).as_string() + 
                      " " + fc::variant(mtrx.outputs[i].unit).as_string();
              total_out += asset( mtrx.outputs[i].amount, mtrx.outputs[i].unit );
          }
          out <<" \n";


          if( i < mtrx.inputs.size() )
          {
              // from
              out << std::left << std::setw( 50 ) << 
                     fc::variant( metain[i].dividends ).as_string() +  " dividends";
              total_in +=  metain[i].dividends;
              total_div_fee +=  metain[i].dividend_fees;
          }
          else
          {
              out << std::setw( 50 ) <<" ";
          }
          out << "| ";
          if( i < mtrx.outputs.size() )
          {
              out << std::left << std::setw( 29 );
              switch( mtrx.outputs[i].claim_func )
              {
                  case claim_by_signature:
                     out << std::string(mtrx.outputs[i].as<claim_by_signature_output>().owner);
                     break;
              }
          }
          out <<" \n";

          if( i < mtrx.inputs.size() )
          {
              // from
              out << std::left << std::setw( 50 ) << 
                     fc::variant( metain[i].dividend_fees ).as_string() +  " div fees";
          }
          else
          {
              out << std::setw( 50 ) <<" ";
          }
          out << "|\n";
     }
     out << "-----------------------------------------------------------------------------------------------------\n";
     out << std::setw( 10 ) << "Total In: " << std::setw(40) << std::string(total_in) << "|     Total Out: "<<std::string(total_out)<<"\n";
     if( total_in > total_out ) 
         out << std::setw( 10 ) << "Sum Trx Fees: " << std::setw(40) << std::string(total_in - total_out) << "\n";
     else
         out << std::setw( 10 ) << "Sum Trx Fees: " << std::setw(40) << "-" + std::string(total_out - total_in) << "\n";
     out << std::setw( 10 ) << "Trx Div Fees: " << std::setw(40) << std::string(total_div_fee) << "\n";
     if( total_in > total_out ) 
         out << std::setw( 10 ) << "Total Trx Fees: " << std::setw(40) << std::string(total_in - total_out + total_div_fee) << "\n";
     out << "=====================================================================================================\n";
     #endif
  }


  template<typename T> class ThousandsSeparator : public std::numpunct<T> {
      public:
      ThousandsSeparator(T Separator) : m_Separator(Separator) {}

      protected:
         T do_thousands_sep() const  {
            return m_Separator;
         }
         std::string do_grouping() const
         {
             return "\03";
         }


      private:
          T m_Separator;
   };


  std::string pretty_print( const trx_block& b, blockchain_db& db )
  {
     uint64_t reward = calculate_mining_reward( b.block_num);
     uint64_t fees = 2*(b.trxs[0].outputs[0].amount - reward/2);
     uint64_t dividends = (reward + fees) / 2; //2*(b.trxs[0].outputs[0].amount - reward/2);
     std::stringstream ss;
     ss.imbue( std::locale( std::locale::classic(), new ThousandsSeparator<char>(',')) );
     ss << std::fixed;
//     ss << "<table border=1 width=\"100%\">\n";
   //  ss << "<tr><td>\n";
     ss << "<table border=1 width=\"1360px\" padding=5px>\n";
     ss << "  <tr>\n";
     ss << "    <th width=\"40px\">Block # </th>\n";
     ss << "    <th width=\"200px\">Time    </th>\n";
     ss << "    <th width=\"80px\">Id      </th>\n";
     ss << "    <th width=\"80px\">Prev Id </th>\n";
     ss << "    <th width=\"200px\">Fees    </th>\n";
     ss << "    <th width=\"200px\">Reward  </th>\n";
     ss << "    <th width=\"200px\">Dividends (Fee+Reward)/2</th>\n";
     ss << "    <th width=\"80px\">POW     </th>\n";
     ss << "  </tr>\n";
     ss << "  <tr>\n";
     ss << "    <td>" << b.block_num                                            <<"</td>\n";
     ss << "    <td>" << std::string( fc::time_point(b.timestamp) )             <<"</td>\n";
     ss << "    <td>" << std::string( b.id() ).substr(0,8)                      <<"</td>\n";
     ss << "    <td>" << std::string( b.prev ).substr(0,8)                      <<"</td>\n";
     ss << "    <td align=right cellpadding=5>" << fees                                                   <<"</td>\n";
     ss << "    <td align=right cellpadding=5>" << reward                                                 <<"</td>\n";
     ss << "    <td align=right cellpadding=5>" << dividends                                              <<"</td>\n";
     ss << "    <td cellpadding=5>" << fc::variant(b.proof_of_work()).as_string().substr(0,8) <<"</td>\n";
     ss << "  </tr>\n";
     ss << "</table>\n";
     ss << "</td></tr>\n";
     ss << "<tr>\n";
     ss << "<td>\n";
     ss << "  <table border=1 width=\"1360px\">\n";
     ss << "  <tr><th>Trx #</th><th><table border=1 width=\"100%\"><tr><th width=\"33%\">INPUTS</th><th width=\"33%\">OUTPUTS</th><th> TRX SUMMARY </th></tr></table></th></tr>\n";

             for( uint32_t i = 0; i < b.trxs.size(); ++i )
             {
                ss << "<tr><td>"<<i<<" - "<< std::string(b.trxs[i].id()).substr(0,8) <<"</td><td>\n";
                pretty_print( ss, db, trx_num( b.block_num, i ) );
                ss << "</td></tr>\n";
             }
     ss << "  </table>\n";
     ss << "<p/>\n";
  //   ss << "</td>\n";
 //    ss << "</table><br/>";

     return ss.str();
  }

} }
