Transaction:
  100 BTC from ACNTA to ACNTB expires Block N signed ACNTA on Block Initial
  
  Result:  Balance of A and B are updated provided this transaction is included between blocks Initial and N.
  Transction only needs to be stored for N blocks and can be discarded 


  Because it is based on 'accounts' instead of linked-lists... the entire transaction history is 'lost' and
  need not be kept.  This will reduce the per-account overhead significantly.   Fruthermore, it makes paying
  of dividends easier?   

  Anyone who 'logs' all transactions will be able to identify users... perhaps... but we can still
  use the bitcoin technique of creating a transaction with multiple inputs/outputs and by using addresses
  to identify 'from' / 'to' amounts we are good.

  The downside is that a transaction can only be evaluated in the context ...


  Database (organized into a mtree)
   account  unit  balance  collateral
    

  In order to send a blance into the negative, there must be sufficient collateral posted.. 








