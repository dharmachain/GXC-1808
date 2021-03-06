#include <graphenelib/graphene.hpp>

#include <graphenelib/system.h>
#include <graphenelib/contract.hpp>
#include <graphenelib/dispatcher.hpp>
#include <graphenelib/multi_index.hpp>
#include <graphenelib/print.hpp>
#include <graphenelib/types.h>
#include <graphenelib/global.h>
#include <graphenelib/asset.h>
#include <graphenelib/contract_asset.hpp>
#include <graphenelib/crypto.h>
#include <vector>
#include <algorithm>

using namespace graphene;
using namespace std;

#include <string>
#include <vector>
#include <map>
#include <utility>
typedef uint128_t uuid;
typedef uint64_t id_type;
typedef string uri_type;

int64_t FEE = 1;  //0.00001 GXC

size_t sub2sep( const std::string& input, std::string& output, const char& separator,
    const size_t& first_pos = 0, const bool& required = false ) {

    graphene_assert(first_pos != std::string::npos, "invalid first pos");
    auto pos = input.find(separator, first_pos);
    if (pos == std::string::npos) {
        graphene_assert(!required, "parse memo error");
        return std::string::npos;
    }
    output = input.substr(first_pos, pos - first_pos);
    return pos;
}

class nft : public contract
{
  public:
    nft(uint64_t id)
        : contract(id), 
            nft_tables(_self, _self),
            accauth_tables(_self, _self),
            nftchain_tables(_self, _self),
            compose_tables(_self, _self),
            game_tables(_self, _self),
            assetmap_tables(_self, _self),
            admin_tables(_self, _self),
            composeattr_tables(_self, _self),
            index_tables(_self, _self),
            nftnumber_tables(_self, _self),
            order_tables(_self, _self),
            accounts(_self, _self)
        {
            std::string my_owner_name = "damo-test";//todo 根据更改默认合约所有者
            if (is_account(my_owner_name)) {
                int64_t account_id = get_account_id(
                    my_owner_name.c_str(),
                    my_owner_name.size());
                contract_owner_id = account_id;
            }
        }
    
    /// @abi action
    void addadmin(std::string admin);
    /// @abi action
    void deladmin(std::string admin);

    /// @abi action
    void create(std::string creator, std::string owner, std::string explain, std::string worldview);
    /// @abi action
    void createother(std::string creator, std::string owner, std::string explain, std::string worldview, id_type chainid, id_type targetid);
    /// @abi action
    void addnftattr(std::string owner, id_type nftid, std::string key, std::string value);
    /// @abi action
    void editnftattr(std::string owner, id_type nftid, std::string key, std::string value);
    /// @abi action
    void delnftattr(std::string owner, id_type nftid, std::string key);

    /// @abi action
    void addaccauth(std::string owner, std::string auth);
    /// @abi action
    void delaccauth(std::string owner);
    
    /// @abi action
    void addnftauth(std::string owner, std::string auth, id_type id);
    /// @abi action
    void delnftauth(std::string owner, id_type id);
    /// @abi action
    void transfernft(std::string from, std::string to, id_type id, std::string memo);
    /// @abi action
    void burn(std::string owner, id_type nftid);
	/// @abi action
    void addchain(std::string owner, std::string chain);
    /// @abi action
    void setchain(std::string owner, id_type chainid, id_type status);
    /// @abi action
    void addcompattr(std::string owner, id_type id);
    /// @abi action
    void delcompattr(std::string owner, id_type id);
    /// @abi action
    void setcompose(std::string owner, id_type firid, id_type secid);
    /// @abi action
    void delcompose(std::string owner, id_type firid, id_type secid);

    /// @abi action
    void addmapping(std::string owner, id_type fromid, id_type targetid, id_type chainid);
    /// @abi action
    void delmapping(std::string owner, id_type fromid, id_type chainid);
   
    /// @abi action
    void addgame(std::string owner, std::string name, std::string introduces);
    /// @abi action
    void editgame(std::string owner, id_type gameid, std::string name, std::string introduces);
    /// @abi action
    void setgame(std::string owner, id_type gameid, id_type status);
    /// @abi action
    void delgame(std::string owner, id_type gameid);
    /// @abi action
    void addgameattr(std::string owner, id_type gameid, std::string key, std::string value);
    /// @abi action
    void editgameattr(std::string owner, id_type gameid, std::string key, std::string value);
    /// @abi action
    void delgameattr(std::string owner, id_type gameid, std::string key);
 

	/// @abi action
    void transfer(std::string from, std::string to, const contract_asset& quantity, const std::string& memo);
 
    // @abi action
    // @abi payable
    void deposit();
    // @abi action
    void withdraw(std::string to_account, contract_asset amount);


    //@abi table admins i64
    struct admins
    {
        int64_t admin;
        uint64_t primary_key() const { return admin; }
    };

    //@abi table nftindexs i64
    struct nftindexs
    {
        id_type id;
        id_type status;
        uint64_t primary_key() const { return id; }
        uint64_t get_status() const { return status; }
    };

    //@abi table nftnumber i64
    struct nftnumber
    {
        int64_t owner;
        id_type number;
        uint64_t primary_key() const { return owner; }
    };

    struct attrpair{
        std::string key;
        std::string value;
    };

    //@abi table nftts i64
    struct nftts
    {
        id_type id;
        int64_t creator;
        int64_t owner;
        int64_t auth;
        std::string explain;
        uint64_t createtime;
        std::string worldview;
        std::vector<attrpair> attr;

        uint64_t primary_key() const { return id; }
        uint64_t get_owner() const { return owner; }
        uint64_t get_creator() const { return creator; }
        
    };

    //@abi table composeattr i64
    struct composeattr
    {
        id_type nftid;
        uint64_t primary_key() const { return nftid; }
    };
    
    //@abi table accauth i64
    struct accauth 
    {
        int64_t owner;
        int64_t auth;
        uint64_t primary_key() const { return owner; }
        uint64_t get_auth() const { return auth; }
    };

    //@abi table nftchains i64
    struct nftchains{
        id_type chainid;
        std::string chain;
        id_type status;
        uint64_t primary_key() const { return chainid; }
        uint64_t get_status() const { return status; }
    };

    //@abi table composes i64
    struct composes
    {
        id_type id;
        id_type firid;
        id_type secid;
        id_type status;
        uint64_t primary_key() const { return id; }
        uint64_t get_fir() const { return firid; }
        uint64_t get_sec() const { return secid; }
        uint64_t get_status() const { return status; }
    };
	
	//@abi table assetmapes i64
    struct assetmaps 
    {
        id_type mappingid;
        id_type fromid;
        id_type targetid;
        id_type chainid;
        uint64_t primary_key() const { return mappingid; }
        uint64_t get_fromid() const { return fromid; }
        uint64_t get_targetid() const { return targetid; }
        uint64_t get_chainid() const { return chainid; }
    };
    
    //@abi table nftgame i64
    struct nftgame
    {
        id_type gameid;
        std::string gamename;
        std::string introduces;
        id_type status;
        id_type index;
        uint64_t createtime;
        //std::map<string, string> gameattr;//todo(liyh) 不支持map，可用新建table查看
        std::vector<attrpair> gameattr;
        
        uint64_t primary_key() const { return gameid; }
        uint64_t get_status() const { return status; }
        uint64_t get_index() const { return index; }
    };

   //@abi table order i64
   struct order 
   {
        id_type         id;
        int64_t         owner;
        id_type         nftid;
        
        contract_asset  price;
        std::string     side;
        std::string     memo;
        uint64_t        createtime;

        uint64_t primary_key() const { return id; }
        uint64_t get_owner() const { return nftid; }
    };

    //@abi table account i64
    struct account {
        uint64_t owner;
        std::vector<contract_asset> balances;

        uint64_t primary_key() const { return owner; }

        GRAPHENE_SERIALIZE(account, (owner)(balances))
    };

    using admins_index = multi_index<N(admins), admins>;

    using nftindex_index = multi_index<N(nftindexs), nftindexs,
        indexed_by< N(bystatus), const_mem_fun< nftindexs, uint64_t, &nftindexs::get_status> > >;

    using nftnumber_index = multi_index<N(nftnumber), nftnumber>;

    using composeattr_index = multi_index<N(composeattr), composeattr>;

    using nfts_index = multi_index<N(nftts), nftts,
        indexed_by< N(byowner), const_mem_fun< nftts, uint64_t, &nftts::get_owner> >,
        indexed_by< N(bycreator), const_mem_fun< nftts, uint64_t, &nftts::get_creator> >>;

    using accauth_index = multi_index<N(accauth), accauth,
        indexed_by< N(byauth), const_mem_fun< accauth, uint64_t, &accauth::get_auth> > >;

    using nftchain_index = multi_index<N(nftchains), nftchains,
        indexed_by< N(bystatus), const_mem_fun< nftchains, uint64_t, &nftchains::get_status> > >;

    using compose_index = multi_index<N(composes), composes,
        indexed_by< N(byfir), const_mem_fun< composes, uint64_t, &composes::get_fir> >,
        indexed_by< N(bysec), const_mem_fun< composes, uint64_t, &composes::get_sec> >,
        indexed_by< N(bystatus), const_mem_fun< composes, uint64_t, &composes::get_status> > >;

    using nftgame_index = multi_index<N(nftgame), nftgame,
        indexed_by< N(byindex), const_mem_fun< nftgame, uint64_t, &nftgame::get_index> >,
        indexed_by< N(bystatus), const_mem_fun< nftgame, uint64_t, &nftgame::get_status> > >;
        
    using assetmaps_index = multi_index<N(assetmaps), assetmaps,
        indexed_by< N(byfromid), const_mem_fun< assetmaps, uint64_t, &assetmaps::get_fromid> >,
        indexed_by< N(bytargetid), const_mem_fun< assetmaps, uint64_t, &assetmaps::get_targetid> >,
        indexed_by< N(bychainid), const_mem_fun< assetmaps, uint64_t, &assetmaps::get_chainid> > >;
    using order_index = multi_index<N(orders), order,
            indexed_by<N(byowner), const_mem_fun<order, uint64_t, &order::get_owner> > >;

    using account_index = graphene::multi_index<N(account), account>;

    private:
        admins_index        admin_tables;
        nftnumber_index     nftnumber_tables;
        nftindex_index      index_tables;
        composeattr_index   composeattr_tables;
        nfts_index          nft_tables;
        accauth_index       accauth_tables;
        nftchain_index      nftchain_tables;
        compose_index       compose_tables;
        nftgame_index       game_tables;
        assetmaps_index     assetmap_tables;
        order_index         order_tables;
        account_index       accounts;

        int64_t             contract_owner_id = 0;
		
    private:
    void createorder(std::string owner, id_type nftid, contract_asset amount, std::string side, std::string memo);
    void cancelorder(std::string owner, id_type id, std::string memo);
    void trade(std::string from, std::string to, id_type orderid, const std::string& side, const std::string& memo);

    void parse_memo(std::string memo, std::string& action, std::map<std::string, std::string>& params);
    bool is_account( const std::string & account );        
    bool is_contract_ownner( const int64_t& account_id );
};