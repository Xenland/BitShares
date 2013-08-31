#pragma once

namespace bts {

  struct identity
  {
     std::string                  label;  // user presented label
     std::string                  bit_id; // underlying BitID label
                                  
     float                        mining_effort; // how much effort to apply to mining (0-1)
     std::vector<friend_status>   friends;
     fc::variant_object           properties; // custom properties attached to the identity.
  };

  /*
  struct meta_transaction 
  {
     bts::blockchain::meta_trx           trx;
     fc::time_point                      date;
     std::string                         memo;
     uint32_t                            confirmations;
     fc::optional<std::string>           error_message;
  };
  */

  /**
   *  The 
   *
   */
  class profile
  {
    public:
      profile();
      ~profile();
      void  open( const fc::path& profile_dir, const fc::uint512_t& key );

      std::vector<identity>         identities()const;
      void                          store_identity( const identity& id );
      
      /**
       *  Checks the transaction to see if any of the inp
       */
      //void                          cache( const bts::blockchain::meta_transaction& mtrx );
      void                          cache( const bts::bitchat::decrypted_message& msg    );

      std::vector<meta_transaction> get_transactions()const;
      message_db_ptr                get_inbox()const;
      addressbook_ptr               get_addressbook()const;
      keychain                      get_keychain()const;
    private:
      std::unique_ptr<detail::profile_impl> my;
  };

} // namespace bts
