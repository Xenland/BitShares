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
          {
             ss << std::string(o.as<claim_by_signature_output>().owner);
             break;
          }
          case claim_by_bid:
          {
             claim_by_bid_output bid = o.as<claim_by_bid_output>();
             ss << "pay to: "<<std::string(bid.pay_address)<<"<br/>\n";
             ss << "price:  "<<std::string(bid.ask_price)<<"<br/>\n";
             ss << "min:    "<<bid.min_trade<<"\n";
            break;
          }
          case claim_by_long:
          {
             claim_by_long_output bid = o.as<claim_by_long_output>();
             ss << "pay to: "<<std::string(bid.pay_address)<<"<br/>\n";
             ss << "price:  "<<std::string(bid.ask_price)<<"<br/>\n";
             ss << "min:    "<<bid.min_trade<<"\n";
            break;
          }
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
        out << "<ol start=\"0\">\n";
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
        out << "<ol start=\"0\">\n";
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
           if( state.balance_sheet[i].in.amount  != fc::uint128(0) || 
               state.balance_sheet[i].out.amount != fc::uint128(0)  )
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

     } 
     catch ( const fc::exception& e )
     {
        out << e.to_detail_string();
        throw;
     }

  }

  /** make the numbers more readable, they can get really big */
  template<typename T> 
  class thousands_separator : public std::numpunct<T> 
  {
      public:
      thousands_separator(T separator) : m_separator(separator) {}

      protected:
         T do_thousands_sep() const  {
            return m_separator;
         }
         std::string do_grouping() const
         {
             return "\03";
         }


      private:
          T m_separator;
   };


  std::string pretty_print( const trx_block& b, blockchain_db& db )
  {
     uint64_t reward = calculate_mining_reward( b.block_num);
     uint64_t fees = 2*(b.trxs[0].outputs[0].amount - reward/2);
     uint64_t dividends = (reward + fees) / 2;

     std::stringstream ss;
     ss.imbue( std::locale( std::locale::classic(), new thousands_separator<char>(',')) );
     ss << std::fixed;
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

     return ss.str();
  }

} }
