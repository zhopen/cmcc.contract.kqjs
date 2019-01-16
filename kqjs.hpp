#include <eosiolib/eosio.hpp>
//#include <libcxx/limits>

using namespace eosio;
using std::string;

typedef uint128_t HASHID;

CONTRACT kqjs : public eosio::contract
{
public:
  //uint64_t SCOPE = _self.value;
  using contract::contract;

private:
  TABLE charge_t
  {
    //来自原始请求的字段：
    string IDType;                    //用户标识类型 01-手机号码
    uint64_t IDValue;                 //@用户标识 手机号码
    string BizType;                   //业务类型代码

    string OprNumb;                   //发起方操作流水号
    HASHID OprNumb_hash;           //@发起方操作流水号的hash值

    string BusiTransID;               //业务流水号
    HASHID BusiTransID_hash;       //@业务流水号的hash值

    string PayTransID;                //支付流水号
    HASHID PayTransID_hash;        //@支付流水号的hash值

    uint64_t ActionDate;              //utc 
    uint64_t OprTime;                 //@utc 
    uint64_t ChargeMoney;             //单位：厘
    string CnlTyp;                    //充值渠道标识
    string PayedType;                 //缴费类型缴费类型 01：缴预存；02缴话费，默认为缴预存
    uint64_t SettleDate;              //@交易账期 utc 
    string ActivityNo;                //营销活动号
    uint64_t ProdDiscount;            //产品折减金额，单位为“厘”，为正整数，无小数点。 跨区交费折减金额为0，不填时取值默认为0
    uint64_t  DiscountRate;             //折扣率 整数 10000：100%无折扣， 9000：90%， 9987: 99.87%

    string PayOrganID;                //支付机构编码 ？？？？是否可以整数 是否是充值漫游地
    HASHID PayOrganID_hash;

    string BizVersion;                //业务版本号

    //其他新增的字段
    uint64_t id;
    uint64_t CommissionRate;          //佣金比例  1: 0.01%
    string PayeeOrganID;              //收款机构编码 ?????
    HASHID PayeeOrganID_hash;      
    uint64_t quantity_in;                      //分账留存的
    uint64_t quantity_out;                     //分账支付的

     //问题：！！！！！！！
     //没有落地方参数
     //支付机构编码可以是整数吗？
     //实时结算？？ 金额汇总？
     //冲正的执行

    uint64_t primary_key() const {return id;}
    uint64_t by_IDValue() const { return IDValue; }
    HASHID by_OprNumb_hash() const { return OprNumb_hash; } 
    HASHID by_BusiTransID_hash() const { return BusiTransID_hash; }     
    HASHID by_PayTransID_hash() const { return PayTransID_hash; }
    uint64_t by_OprTime() const { return OprTime; }
    uint64_t by_SettleDate() const { return SettleDate; }
    //EOSLIB_SERIALIZE(charge_t, (id)(account_name)(quantity)(from)(to)(ratio)(date))
  };

  TABLE uncharge_t {
    uint64_t IDValue;                 //@用户标识 手机号码

    string OprNumb;                   //发起方操作流水号
    HASHID OprNumb_hash;           //@发起方操作流水号的hash值

    uint64_t OprTime;                 //@utc 
    uint64_t OriActionDate;

    string OriTransactionID;          //同充值交易的OprNumb
    HASHID OriTransactionID_hash;  //同充值交易的OprNumb
    
    string RevokeReason;              //冲正原因
    uint64_t SettleDate;              //原交易账期
    string BizVersion;                //业务版本号

    uint64_t id;

    uint64_t primary_key() const {return id;}
    HASHID by_OprNumb_hash() const { return OprNumb_hash; } 
    HASHID by_OriTransactionID_hash() const { return OriTransactionID_hash; } 
    uint64_t by_SettleDate() const { return SettleDate; }
  };



  TABLE settlement_t  {
    uint64_t id;
    uint64_t SettleDate;        //交易账期
    string PayeeOrganID;      //收款机构编码 ?????   
    HASHID PayeeOrganID_hash;      //收款机构编码 ?????    
    uint64_t quantity_in;       //留存金额
    uint64_t quantity_out;      //结出金额
    uint64_t primary_key() const { return id; }
    uint64_t by_SettleDate() const { return SettleDate; }
    HASHID by_PayeeOrganID_hash() const { return PayeeOrganID_hash; }
  };

public:
  typedef eosio::multi_index<name("charges"), charge_t,
                             indexed_by<name("idvalue"), const_mem_fun<charge_t, uint64_t, &charge_t::by_IDValue>>,
                             indexed_by<name("oprnumb"), const_mem_fun<charge_t, HASHID, &charge_t::by_OprNumb_hash>>,
                             indexed_by<name("busitransid"), const_mem_fun<charge_t, HASHID, &charge_t::by_BusiTransID_hash>>,
                             indexed_by<name("paytransid"), const_mem_fun<charge_t, HASHID, &charge_t::by_PayTransID_hash>>,
                             indexed_by<name("oprtime"), const_mem_fun<charge_t, uint64_t, &charge_t::by_OprTime>>,
                             indexed_by<name("settledate"), const_mem_fun<charge_t, uint64_t, &charge_t::by_SettleDate>>
                            >charges_t;
  typedef eosio::multi_index<name("settlements"), settlement_t,
                             indexed_by<name("payeeorganid"), const_mem_fun<settlement_t, HASHID, &settlement_t::by_PayeeOrganID_hash>>,
                             indexed_by<name("settledate"), const_mem_fun<settlement_t, uint64_t, &settlement_t::by_SettleDate>>
                            >settlements_t;
  typedef eosio::multi_index<name("uncharges"), uncharge_t,
                             indexed_by<name("settledate"), const_mem_fun<uncharge_t, uint64_t, &uncharge_t::by_SettleDate>>,
                             indexed_by<name("oprnumb"), const_mem_fun<uncharge_t, HASHID, &uncharge_t::by_OprNumb_hash>>,
                             indexed_by<name("oritransid"), const_mem_fun<uncharge_t, HASHID, &uncharge_t::by_OriTransactionID_hash>>
                            >uncharges_t;

  ACTION charge(  name account,
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
                  string BizVersion );
  ACTION uncharge(uncharge_t uncharge, name account);                  
  ACTION rmcharge(uint64_t id, name account);          
  ACTION rmsettledate(name account, uint64_t settledate);        
    // ACTION getchargeby(std::string key, uint64_t value);



// public:
//   template <typename T>
//   void print_charges(T & itr, T & itr_end, int32_t limit)
//   {
//     int32_t i = 0;
//     if (limit <= 0)
//       limit = (std::numeric_limits<int32_t>::max)();
//     for (; itr != itr_end && i < limit; itr++, i++)
//     {
//       print("________________________________________");
//       print("\nid: ", itr->id,
//             "\naccount: ", itr->account_name,
//             "\nquantity: ", itr->quantity,
//             "\nfrom: ", itr->from,
//             "\nto: ", itr->to,
//             "\nratio: ", itr->ratio,
//             "\nserial: ", itr->serial_value,
//             "\ndate: ", itr->date);
//       print("\n");
//     }
//   };
};
