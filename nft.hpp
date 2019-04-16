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
typedef uint128_t uuid;
typedef uint64_t id_type;
typedef string uri_type;

enum order_side {
    BUY     =   0, 
    SELL    =   1
};

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
            order_tables(_self, _self)
        {}
    
    /// @abi action
    void addadmin(graphenelib::name admin);
    /// @abi action
    void deladmin(graphenelib::name admin);

    /// @abi action
    void create(graphenelib::name creator, graphenelib::name owner, std::string explain, std::string worldview);
    /// @abi action
    void createother(graphenelib::name creator, graphenelib::name owner, std::string explain, std::string worldview, id_type chainid, id_type targetid);
    /// @abi action
    void addnftattr(graphenelib::name owner, id_type nftid, std::string key, std::string value);
    /// @abi action
    void editnftattr(graphenelib::name owner, id_type nftid, std::string key, std::string value);
    /// @abi action
    void delnftattr(graphenelib::name owner, id_type nftid, std::string key);

    /// @abi action
    void addaccauth(graphenelib::name owner, graphenelib::name auth);
    /// @abi action
    void delaccauth(graphenelib::name owner);
    
    /// @abi action
    void addnftauth(graphenelib::name owner, graphenelib::name auth, id_type id);
    /// @abi action
    void delnftauth(graphenelib::name owner, id_type id);
    /// @abi action
    void transfer(graphenelib::name from, graphenelib::name to, id_type id, std::string memo);
    /// @abi action
    void burn(graphenelib::name owner, id_type nftid);
	/// @abi action
    void addchain(graphenelib::name owner, std::string chain);
    /// @abi action
    void setchain(graphenelib::name owner, id_type chainid, id_type status);
    /// @abi action
    void addcompattr(graphenelib::name owner, id_type id);
    /// @abi action
    void delcompattr(graphenelib::name owner, id_type id);
    /// @abi action
    void setcompose(graphenelib::name owner, id_type firid, id_type secid);
    /// @abi action
    void delcompose(graphenelib::name owner, id_type firid, id_type secid);

    /// @abi action
    void addmapping(graphenelib::name owner, id_type fromid, id_type targetid, id_type chainid);
    /// @abi action
    void delmapping(graphenelib::name owner, id_type fromid, id_type chainid);
   
    /// @abi action
    void addgame(graphenelib::name owner, std::string name, std::string introduces);
    /// @abi action
    void editgame(graphenelib::name owner, id_type gameid, std::string name, std::string introduces);
    /// @abi action
    void setgame(graphenelib::name owner, id_type gameid, id_type status);
    /// @abi action
    void delgame(graphenelib::name owner, id_type gameid);
    /// @abi action
    void addgameattr(graphenelib::name owner, id_type gameid, std::string key, std::string value);
    /// @abi action
    void editgameattr(graphenelib::name owner, id_type gameid, std::string key, std::string value);
    /// @abi action
    void delgameattr(graphenelib::name owner, id_type gameid, std::string key);
 
 	/// @abi action
    void createorder(graphenelib::name owner, id_type nftid, contract_asset amount, std::string side, std::string memo);
    /// @abi action
	void cancelorder(graphenelib::name owner, int64_t id);
	/// @abi action
    void trade(graphenelib::name from, graphenelib::name to, id_type id, std::string memo);

    //@abi table admins i64
    struct admins
    {
        graphenelib::name admin;
        uint64_t primary_key() const { return admin.value; }
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
        graphenelib::name owner;
        id_type number;
        uint64_t primary_key() const { return owner.value; }
    };

    struct attrpair{
        std::string key;
        std::string value;
    };

    //@abi table nftts i64
    struct nftts
    {
        id_type id;
        graphenelib::name creator;
        graphenelib::name owner;
        graphenelib::name auth;
        std::string explain;
        uint64_t createtime;
        std::string worldview;
        std::vector<attrpair> attr;

        uint64_t primary_key() const { return id; }
        uint64_t get_owner() const { return owner.value; }
        uint64_t get_creator() const { return creator.value; }
        
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
        graphenelib::name owner;
        graphenelib::name auth;
        uint64_t primary_key() const { return owner.value; }
        uint64_t get_auth() const { return auth.value; }
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
        int64_t         id;
        id_type         nftid;
        graphenelib::name            owner;
        contract_asset  price;
        std::string     side;
        std::string     memo;
        uint64_t        createtime;

        uint64_t primary_key() const { return id; }
        uint64_t get_nftid() const { return nftid; }
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
            indexed_by<N(bynftid), const_mem_fun<order, uint64_t, &order::get_nftid> > >;

    private:
        void contractDeposit(graphenelib::name user, contract_asset quantity, std::string memo);
        void contractTransfer(graphenelib::name from, graphenelib::name to, contract_asset quantity, std::string memo);
        void contractWithdraw(graphenelib::name user, contract_asset quantity, std::string memo);
 
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
};