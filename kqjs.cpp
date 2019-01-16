#include "kqjs.hpp"
#include <eosiolib/asset.hpp>
using namespace eosio;


ACTION kqjs::charge( name account,
                     string IDType,              
                     uint64_t IDValue,           
                     string BizType,             
                     string OprNumb,             
                     HASHID OprNumb_hash,        
                     string BusiTransID,         
                     HASHID BusiTransID_hash, 
                     string PayTransID,          
                     HASHID PayTransID_hash,  
                     uint64_t ActionDate,        
                     uint64_t OprTime,           
                     uint64_t ChargeMoney,       
                     string CnlTyp,              
                     string PayedType,           
                     uint64_t SettleDate,        
                     string ActivityNo,          
                     uint64_t ProdDiscount,      
                     uint64_t DiscountRate,      
                     string PayOrganID,          
                     HASHID PayOrganID_hash,
                     uint64_t CommissionRate,
                     string PayeeOrganID,         
                     HASHID PayeeOrganID_hash,
                     string BizVersion ) {
  
   require_auth( account );
   print_f("code: %\n", get_code ());
   print_f("self: %\n", get_self ());

   uint64_t realpay = (ChargeMoney-ProdDiscount)*DiscountRate/10000;
   uint64_t in = realpay * CommissionRate / 10000;
   uint64_t out = realpay - in;

   {
      charges_t charges(_self, account.value);
      auto index = charges.get_index<name("oprnumb")>();
      auto itr = index.find(OprNumb_hash);
      eosio_assert(itr == index.end(), "oprnumb id already exists");
      charges.emplace(_self, [&]( auto& row ) {
                           row.id=                        charges.available_primary_key();
                           row.IDType=                    IDType;
                           row.IDValue=                   IDValue;
                           row.BizType=                   BizType;
                           row.OprNumb=                   OprNumb;
                           row.OprNumb_hash=              OprNumb_hash;
                           row.BusiTransID=               BusiTransID;
                           row.BusiTransID_hash=          BusiTransID_hash;
                           row.PayTransID=                PayTransID;
                           row.PayTransID_hash=           PayTransID_hash;
                           row.ActionDate=                ActionDate;
                           row.OprTime=                   OprTime;
                           row.ChargeMoney=               ChargeMoney;
                           row.CnlTyp=                    CnlTyp;
                           row.PayedType=                 PayedType;
                           row.SettleDate=                SettleDate;
                           row.ActivityNo=                ActivityNo;
                           row.ProdDiscount=              ProdDiscount;
                           row.DiscountRate=              DiscountRate;
                           row.PayOrganID=                PayOrganID;
                           row.PayOrganID_hash=           PayOrganID_hash;
                           row.CommissionRate=            CommissionRate;
                           row.PayeeOrganID=              PayeeOrganID;
                           row.PayeeOrganID_hash=         PayeeOrganID_hash;
                           row.BizVersion=                BizVersion;
                           row.quantity_in=               in;
                           row.quantity_out=              out;
                     });
   }

   {
      settlements_t settlements(_self, account.value);
      auto index = settlements.get_index<name("payeeorganid")>();
      auto itr = index.find(PayeeOrganID_hash);  

      for(; itr!=index.end(); itr++){
         if( itr->SettleDate == SettleDate) {
            index.modify( itr, 
                           account, 
                           [&]( auto& settlement ) { 
                              settlement.quantity_in=   itr->quantity_in + in;   
                              settlement.quantity_out=  itr->quantity_out + out;  
                           }
                        );   
            return;          
         }
      }

      settlements.emplace( _self, 
                  [&]( auto& settlement ) {
                     settlement.id=            settlements.available_primary_key();
                     settlement.SettleDate=    SettleDate;
                     settlement.PayeeOrganID=  PayeeOrganID;  
                     settlement.PayeeOrganID_hash=  PayeeOrganID_hash;  
                     settlement.quantity_in=   in;   
                     settlement.quantity_out=  out;  
                  }
               );

   }

}

// #define GET_ROW_BY_TWO_INDEX(index, value1, indexname2, value2) \
// { \
//    auto itr = index.find(value1);  \
//    eosio_assert(itr != index.end(), " don't exists");\
//    int finded=0;\
//    for(; itr<index.end(); itr++) {\
//       if(itr->indexname2 = value2) { finded = 1; break; }\
//    }\
//    if(finded==0) eosio_assert(itr != index.end(), indexname2" don't exists");\
//    (itr);\
// }


ACTION kqjs::uncharge(uncharge_t uncharge, name account) {
    
   require_auth( account );
   uncharges_t uncharges(_self, account.value);
   auto index = uncharges.get_index<name("oprnumb")>();
   auto itr = uncharges.find(uncharge.OprNumb_hash);
   eosio_assert(itr == uncharges.end(), "charge id already exists");

   uncharges.emplace(   _self, 
                        [&]( auto& row ){
                           row.IDValue                      = uncharge.IDValue;              
                           row.OprNumb                      = uncharge.OprNumb;              
                           row.OprNumb_hash                 = uncharge.OprNumb_hash;         
                           row.OprTime                      = uncharge.OprTime;              
                           row.OriActionDate                = uncharge.OriActionDate;        
                           row.OriTransactionID             = uncharge.OriTransactionID;     
                           row.OriTransactionID_hash        = uncharge.OriTransactionID_hash;
                           row.RevokeReason                 = uncharge.RevokeReason;         
                           row.SettleDate                   = uncharge.SettleDate;           
                           row.BizVersion                   = uncharge.BizVersion; 
                           row.id                           = uncharges.available_primary_key();
                        }
                     ); 
   
   charges_t charges(_self, account.value);
   auto charge_index = charges.get_index<name("oprnumb")>();
   auto charge_itr = charge_index.find(uncharge.OriTransactionID_hash);
   eosio_assert(charge_itr != charge_index.end(), "OriTransactionID don't exists");

    settlements_t settlements(_self, account.value);
    auto settlement_index = settlements.get_index<name("payeeorganid")>();
    auto settlement_itr = settlement_index.find(charge_itr->PayeeOrganID_hash);
    eosio_assert(settlement_itr != settlement_index.end(), "settlement don't exists");
    bool found = false;
    for(; settlement_itr!=settlement_index.end(); settlement_itr++ ) {
      if(settlement_itr->SettleDate == uncharge.SettleDate) {
         int64_t in = settlement_itr->quantity_in - charge_itr->quantity_in;
         int64_t out = settlement_itr->quantity_out - charge_itr->quantity_out;
         eosio_assert(in>=0, "in must >= 0!");
         eosio_assert(out>=0, "out must >= 0!");
         settlement_index.modify(   settlement_itr, 
                                    account, 
                                    [&]( auto& settlement ) { 
                                       settlement.quantity_in=   in;   
                                       settlement.quantity_out=  out;  
                                    }
                                 );
         return;
      }
    }
    eosio_assert(found==true, "settledate don't exists");
 

}

ACTION kqjs::rmcharge(uint64_t id, name account) {
   require_auth(account);

   charges_t charges(_self, account.value);  
   auto itr = charges.find(id);
   charges.erase(itr);
}


#define REMOVE_ROW(object, lower, upper)  \
{                                \
   while(lower != upper) {   \
      lower = object.erase(lower);    \
   }                              \
} 

ACTION kqjs::rmsettledate(name account, uint64_t settledate) {
   require_auth(account);

   charges_t charges(_self, account.value);
   uncharges_t uncharges(_self, account.value);
   settlements_t settlements(_self, account.value);
   {
      auto index = charges.get_index<name("settledate")>();
      REMOVE_ROW(index, index.lower_bound(settledate), index.upper_bound(settledate));   
   }
   {
      auto index = uncharges.get_index<name("settledate")>();
      REMOVE_ROW(index, index.lower_bound(settledate), index.upper_bound(settledate));   
   }
   {
      auto index = settlements.get_index<name("settledate")>();
      REMOVE_ROW(index, index.lower_bound(settledate), index.upper_bound(settledate));
   }
}


// ACTION kqjs::getchargeby(std::string keyanme, uint64_t value) {
//    require_auth( _self );

//    charges_t charges(_self, _self.value);  
//    if(keyanme == "id") {
//       auto itr = charges.find(value);
//       auto end = charges.end();
//       eosio_assert(itr != end, "charge id don't exists");
//       print_charges(itr, end, 1);      
//    }
//    else if(keyanme == "account"){
//       auto index = charges.get_index<name("account")>();
//       auto itr = index.find(value);
//       auto end = index.end();
//       eosio_assert(itr != end, "charge id don't exists");
//       print_charges(itr, end, 0);
//    }
// }

EOSIO_DISPATCH( kqjs, (charge)(uncharge)(rmcharge)(rmsettledate) ) 

