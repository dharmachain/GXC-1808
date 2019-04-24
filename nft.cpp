#include "nft.hpp"

bool nft::is_account( const std::string & account ) {
    int64_t account_id = get_account_id(account.c_str(), account.size());
    return account_id >= 0;
}

bool nft::is_contract_ownner(const int64_t& account_id){
    return account_id == contract_owner_id;
}

void nft::addadmin(std::string stradmin) 
{    
	graphene_assert(is_contract_ownner(get_trx_sender()), "you must be owner");//require_auth(_self);//todo(liyh)鉴权是发布人的id
    graphene_assert(is_account(stradmin), "admin account does not exist");
    int64_t admin = get_account_id(stradmin.c_str(), stradmin.length());
 
    auto admin_one = admin_tables.find(admin);
    graphene_assert(admin_one == admin_tables.end(), "admin account already authed");

    admin_tables.emplace(get_trx_sender(), [&](auto& admin_data) {
        admin_data.admin = admin;
    });
}

void nft::deladmin(std::string stradmin) 
{
	graphene_assert(is_contract_ownner(get_trx_sender()), "you must be owner");//require_auth(_self);
    graphene_assert(is_account(stradmin), "admin account does not exist");
    int64_t admin = get_account_id(stradmin.c_str(), stradmin.length());

    auto admin_one = admin_tables.find(admin);
    graphene_assert(admin_one != admin_tables.end(), "admin account is not auth");

    admin_tables.erase(admin_one);
}

void nft::create(std::string strcreator, std::string strowner, std::string explain, std::string worldview) 
{
    graphene_assert(is_account(strcreator), "creator account does not exist");
    graphene_assert(is_account(strowner), "owner account does not exist");
    int64_t creator = get_account_id(strcreator.c_str(), strcreator.length());
    int64_t owner = get_account_id(strowner.c_str(), strowner.length());

    graphene_assert(explain.size() <= 256, "explain has more than 256 bytes");
    graphene_assert(worldview.size() <= 20 && worldview.size() > 0, "worldview has more than 20 bytes or is empty");
    //require_auth(creator);

    auto admin_one = admin_tables.find(creator);
    graphene_assert(admin_one != admin_tables.end(), "admin account is not auth");

    id_type index_id = index_tables.available_primary_key();
    index_tables.emplace(get_trx_sender(), [&](auto& index_data) {
        index_data.id = index_id;
        index_data.status = 1;
    });

    // Create new nft
    //auto time_now = time_point_sec(now());
    auto time_now = get_head_block_time();
    nft_tables.emplace(get_trx_sender(), [&](auto& nft_data) {
        nft_data.id = index_id;
        nft_data.creator=creator;
        nft_data.owner = owner;
        nft_data.auth = owner;
        nft_data.explain = explain;
        nft_data.createtime = time_now;
        nft_data.worldview = worldview;
    });

    auto nft_num = nftnumber_tables.find(owner);
    if(nft_num != nftnumber_tables.end()){
        nftnumber_tables.modify(nft_num, get_trx_sender(), [&](auto& nft_num_data) {
            nft_num_data.number = nft_num->number+1;
        });
    }
    else 
    {
        nftnumber_tables.emplace(get_trx_sender(), [&](auto& nft_num_data) {
            nft_num_data.owner = owner;
            nft_num_data.number = 1;
        });   
    }
}

void nft::createother(std::string strcreator, std::string strowner, std::string explain, std::string worldview, id_type chainid, id_type targetid) 
{
    graphene_assert(is_account(strcreator), "creator account does not exist");
    graphene_assert(is_account(strowner), "owner account does not exist");
    int64_t creator = get_account_id(strcreator.c_str(), strcreator.length());
    int64_t owner = get_account_id(strowner.c_str(), strowner.length());

    graphene_assert(explain.size() <= 256, "explain has more than 64 bytes");
    graphene_assert(worldview.size() <= 20 && worldview.size() > 0, "worldview has more than 20 bytes or is empty");

	//require_auth(creator);
    
    auto admin_one = admin_tables.find(creator);
    graphene_assert(admin_one != admin_tables.end(), "admin account is not auth");
    
    auto chain_find = nftchain_tables.find(chainid);
    graphene_assert(chain_find != nftchain_tables.end(), "chainid is not exist");

    // Create new nft
    //auto time_now = time_point_sec(now());
    auto time_now = get_head_block_time();
    id_type indexid = index_tables.available_primary_key();
    index_tables.emplace(get_trx_sender(), [&](auto& index_data) {
        index_data.id = indexid;
        index_data.status = 1;
    });

    //id_type newid = nft_tables.available_primary_key();
    nft_tables.emplace(get_trx_sender(), [&](auto& nft_data) {
        nft_data.id = indexid;
        nft_data.creator = creator;
        nft_data.owner = owner;
        nft_data.auth = owner;
        nft_data.explain = explain;
        nft_data.createtime = time_now;
        
        nft_data.worldview = worldview;
    });

    assetmap_tables.emplace(get_trx_sender(), [&](auto& assetmapping_data) {
        assetmapping_data.mappingid = game_tables.available_primary_key();
        assetmapping_data.fromid = indexid;
        assetmapping_data.targetid = targetid;
        assetmapping_data.chainid = chainid;
    });

    auto nftnum = nftnumber_tables.find(owner);
    if(nftnum != nftnumber_tables.end()){
        nftnumber_tables.modify(nftnum,get_trx_sender(), [&](auto& nftnum_data) {
            nftnum_data.number = nftnum->number+1;
        });
    } else {
        nftnumber_tables.emplace(get_trx_sender(), [&](auto& nftnum_data) {
            nftnum_data.owner = owner;
            nftnum_data.number = 1;
        });   
    }

    //print(time_now);
}

void nft::addaccauth(std::string strowner,std::string strauth) 
{
    //require_auth(owner);
    
    graphene_assert(is_account(strowner), "owner account does not exist");
    graphene_assert(is_account(strauth), "auth account does not exist");
    int64_t owner = get_account_id(strowner.c_str(), strowner.length());
    int64_t auth = get_account_id(strauth.c_str(), strauth.length());

    auto auth_find = accauth_tables.find(owner);
    graphene_assert(auth_find == accauth_tables.end(), "owner account already authed");

    accauth_tables.emplace(get_trx_sender(), [&](auto& auth_data) {
        auth_data.owner = owner;
        auth_data.auth = auth;
    });
}

void nft::addnftattr(std::string strowner, id_type nftid, std::string key, std::string value) 
{
    graphene_assert(is_account(strowner), "owner account does not exist");
    int64_t owner = get_account_id(strowner.c_str(), strowner.length());
    //require_auth(owner);

    //auto admin_one = admin_tables.find(owner);
    //graphene_assert(admin_one != admin_tables.end(), "admin account is not auth");
    
    auto nft_find = nft_tables.find(nftid);
    graphene_assert(nft_find != nft_tables.end(), "nft id is not exist");

    //nft status check
    auto status_iter = index_tables.find(nftid);
    graphene_assert(status_iter != index_tables.end(), "nft index does not exist");
    graphene_assert(status_iter->status == 1, "nft status is close, does not add nft attr");

    std::vector<attrpair> vectAttr = nft_find->attr;
    auto iter = find_if(vectAttr.begin(),
                    vectAttr.end(),
                    [=] (const attrpair& m) -> bool { return key == m.key; });
    graphene_assert(iter == vectAttr.end(), "key is exist");
    vectAttr.push_back({key, value}); 
    nft_tables.modify(nft_find, get_trx_sender(), [&](auto& attr_data) {
        attr_data.attr = vectAttr;
    });
}

void nft::editnftattr(std::string strowner, id_type nftid, std::string key, std::string value) 
{
    graphene_assert(is_account(strowner), "owner account does not exist");
    int64_t owner = get_account_id(strowner.c_str(), strowner.length());
    //require_auth(owner);

    //auto admin_one = admin_tables.find(owner);
    //graphene_assert(admin_one != admin_tables.end(), "admin account is not auth");
    
    auto nft_find = nft_tables.find(nftid);
    graphene_assert(nft_find != nft_tables.end(), "nft id is not exist");
    
	//nft status check
    auto status_iter = index_tables.find(nftid);
    graphene_assert(status_iter != index_tables.end(), "nft index does not exist");
    graphene_assert(status_iter->status == 1, "nft status is close, does not edit nft attr");

    std::vector<attrpair> vectAttr = nft_find->attr;
    auto iter = find_if(vectAttr.begin(),
                    vectAttr.end(),
                    [=] (const attrpair& m) -> bool { return key == m.key; });
    graphene_assert(iter != vectAttr.end(), "key is not exist");
    iter->value = value;

    nft_tables.modify(nft_find, get_trx_sender(), [&](auto& attr_data) {
        attr_data.attr = vectAttr;
    }); 
}

void nft::delnftattr(std::string strowner, id_type nftid, string key) 
{
    graphene_assert(is_account(strowner), "owner account does not exist");
    int64_t owner = get_account_id(strowner.c_str(), strowner.length());
    //require_auth(owner);

    //auto admin_one = admin_tables.find(owner);
    //graphene_assert(admin_one != admin_tables.end(), "admin account is not auth");
    
    auto nft_find = nft_tables.find(nftid);
    graphene_assert(nft_find != nft_tables.end(), "nft id is not exist");

    //nft status check
    auto status_iter = index_tables.find(nftid);
    graphene_assert(status_iter != index_tables.end(), "nft index does not exist");
    graphene_assert(status_iter->status == 1, "nft status is close, does not delete nft attr");

    std::vector<attrpair> vectAttr = nft_find->attr;
    auto iter = find_if(vectAttr.begin(),
                    vectAttr.end(),
                    [=] (const attrpair& m) -> bool { return key == m.key; });
    graphene_assert(iter != vectAttr.end(), "key is not exist");
    vectAttr.erase(iter);

    nft_tables.modify(nft_find, get_trx_sender(), [&](auto& attr_data) {
        attr_data.attr = vectAttr;
    }); 
}

void nft::delaccauth(std::string strowner) 
{
    //require_auth(owner);
    graphene_assert(is_account(strowner), "owner account does not exist");
    int64_t owner = get_account_id(strowner.c_str(), strowner.length());

    auto auth_find = accauth_tables.find(owner);
    graphene_assert(auth_find != accauth_tables.end(), "owner has not auth");

    accauth_tables.erase(auth_find);
}

void nft::addnftauth(std::string strowner, std::string strauth, id_type id)
{
    //require_auth(owner);
    graphene_assert(is_account(strowner), "owner account does not exist");
    graphene_assert(is_account(strauth), "auth account does not exist");
    int64_t owner = get_account_id(strowner.c_str(), strowner.length());
    int64_t auth = get_account_id(strauth.c_str(), strauth.length());

    auto nft_find_id = nft_tables.find(id);
    graphene_assert(nft_find_id != nft_tables.end(), "nft id is not exist");

    if(nft_find_id->owner != owner){
        auto nft_accauth_find = accauth_tables.find(owner);
        graphene_assert(nft_accauth_find != accauth_tables.end(), "account has not auth"); 
        graphene_assert(nft_accauth_find->auth != owner, "account has not auth");         
    }

    nft_tables.modify(nft_find_id, get_trx_sender(), [&](auto& nft_data) {
        nft_data.auth = auth;
    });    
}

void nft::delnftauth(std::string strowner, id_type id)
{
    //require_auth(owner);
    graphene_assert(is_account(strowner), "owner account does not exist");    
    int64_t owner = get_account_id(strowner.c_str(), strowner.length());

    auto nft_find_id = nft_tables.find(id);
    graphene_assert(nft_find_id != nft_tables.end(), "nft id is not exist");

    if(nft_find_id->owner != owner){
        auto nft_accauth_find = accauth_tables.find(owner);
        graphene_assert(nft_accauth_find != accauth_tables.end(), "account has not auth"); 
        graphene_assert(nft_accauth_find->auth != owner, "account has not auth");         
    }

    nft_tables.modify(nft_find_id, get_trx_sender(), [&](auto& nft_data) {
        nft_data.auth = owner;
    });    
}

void nft::transfernft(std::string strfrom, std::string strto, id_type id, string memo)
{
    //require_auth(from);
    graphene_assert(is_account(strfrom), "from auth does not exist");
    graphene_assert(is_account(strto), "to auth does not exist");
    int64_t from = get_account_id(strfrom.c_str(), strfrom.length());
    int64_t to = get_account_id(strto.c_str(), strto.length());

    graphene_assert(memo.size() <= 256, "memo has more than 256 bytes");

    auto nft_find_id = nft_tables.find(id);
    graphene_assert(nft_find_id != nft_tables.end(), "nft id is not exist");

    int64_t owner_nft = nft_find_id->owner;
    if(nft_find_id->owner != from){
        if(nft_find_id->auth != from){
            auto nft_accauth_find = accauth_tables.find(nft_find_id->owner);
            graphene_assert(nft_accauth_find != accauth_tables.end(), "from has not auth"); 
            graphene_assert(nft_accauth_find->auth != from, "from has not auth"); 
        }       
    }

    nft_tables.modify(nft_find_id, get_trx_sender(), [&](auto& nft_data) {
        nft_data.auth = to;
        nft_data.owner = to;
     });

    auto nftnum = nftnumber_tables.find(owner_nft);
    if(nftnum->number != 1){
            nftnumber_tables.modify(nftnum,get_trx_sender(), [&](auto& nftnum_data) {
            nftnum_data.number = nftnum->number-1;
        });
    }
    else 
    {
        nftnumber_tables.erase(nftnum);   
    }

    auto nfttonum = nftnumber_tables.find(to);
    if(nfttonum != nftnumber_tables.end()){
        nftnumber_tables.modify(nfttonum, get_trx_sender(), [&](auto& nftnum_data) {
            nftnum_data.number = nfttonum->number+1;
        });
    }
    else 
    {
        nftnumber_tables.emplace(get_trx_sender(), [&](auto& nftnum_data) {
            nftnum_data.owner = to;
            nftnum_data.number = 1;
        });   
    }  
}

void nft::burn(std::string strowner, id_type nftid) 
{
	graphene_assert(is_account(strowner), "owner account does not exist");
    int64_t owner = get_account_id(strowner.c_str(), strowner.length());
    //require_auth(owner);
    auto admin_one = admin_tables.find(owner);
    graphene_assert(admin_one != admin_tables.end(), "admin account is not auth");
    auto nft_find = nft_tables.find(nftid);
 
    graphene_assert(nft_find != nft_tables.end(), "nft asset is not exist");
    graphene_assert(nft_find->owner == owner, "owner account insufficient privilege");
    nft_tables.erase(nft_find);
    auto nftnum = nftnumber_tables.find(nft_find->owner);
    if(nftnum->number != 1){
        nftnumber_tables.modify(nftnum, get_trx_sender(), [&](auto& nftnum_data) {
            nftnum_data.number = nftnum->number-1;
        });
    }
    else 
    {
        nftnumber_tables.erase(nftnum);   
    }

    auto index_id = index_tables.find(nftid);
    graphene_assert(index_id != index_tables.end(), "nft index does not exist");
    index_tables.modify(index_id, get_trx_sender(), [&](auto& index_data) {
        index_data.status = 0;
    }); 

    auto compose_find=composeattr_tables.find(nftid);
    if(nft_find == nft_tables.end()){
        composeattr_tables.erase(compose_find);
    }

    auto compose_firid = compose_tables.get_index<N(byfir)>();
	auto it = compose_firid.lower_bound(nftid);
    for(; it != compose_firid.end() && it->firid == nftid; ++it){
        auto fir_one = compose_tables.find(it->id);
        compose_tables.erase(fir_one);
	}

    auto compose_secid = compose_tables.get_index<N(bysec)>();
	auto its = compose_secid.lower_bound(nftid);
    for(; its != compose_secid.end() && its->secid == nftid; ++its){
        auto sec_one = compose_tables.find(its->id);
        compose_tables.erase(sec_one);
	}

    auto assetmap_nft = assetmap_tables.get_index<N(byfromid)>();
	auto iter = assetmap_nft.lower_bound(nftid);
    for( ; iter != assetmap_nft.end() && iter->fromid == nftid; ++iter){
        auto asset_one = assetmap_tables.find(iter->mappingid);
        assetmap_tables.erase(asset_one);
	}
}

void nft::addchain(std::string strowner,string chain)
{
    graphene_assert(is_account(strowner), "owner account does not exist");    
    int64_t owner = get_account_id(strowner.c_str(), strowner.length());

    graphene_assert(chain.size() <= 64, "explain has more than 64 bytes");
	//require_auth(owner);
    auto admin_one = admin_tables.find(owner);
    graphene_assert(admin_one != admin_tables.end(), "admin account is not auth");
    
    auto nftchains_data = nftchain_tables.get_index<N(bystatus)>();
	auto it = nftchains_data.lower_bound(1);

	bool found = true;
	id_type id = 0;
	for(; it != nftchains_data.end() && it->status == 1; ++it){
		if(it->chain == chain) {
			id = it->chainid;
			found = false;
			break;
		}
	}

	graphene_assert(found, "chain is exists");
    nftchain_tables.emplace(get_trx_sender(), [&](auto& nftchain_data) {
        nftchain_data.chainid = nftchain_tables.available_primary_key();
        nftchain_data.chain = chain;
        nftchain_data.status = 1;  
    });
}

void nft::setchain(std::string strowner,id_type chainid,id_type status)
{
    graphene_assert(is_account(strowner), "owner account does not exist");    
    int64_t owner = get_account_id(strowner.c_str(), strowner.length());
    //require_auth(owner);

    auto admin_one = admin_tables.find(owner);
    graphene_assert(admin_one != admin_tables.end(), "admin account is not auth");
   
    auto nftchain_find = nftchain_tables.find(chainid);
    graphene_assert(nftchain_find != nftchain_tables.end(), "chainid is not exists");

    bool statusOk = ((status == 0 || status == 1) ? true : false);

    graphene_assert(statusOk, "status must eq 0 or 1");
    nftchain_tables.modify(nftchain_find, get_trx_sender(), [&](auto& nftchain_data) {
        nftchain_data.status = status;  
    });
}

void nft::addcompattr(std::string strowner, id_type id)
{
    graphene_assert(is_account(strowner), "owner account does not exist");    
    int64_t owner = get_account_id(strowner.c_str(), strowner.length());

    //require_auth(owner);
    auto admin_one = admin_tables.find(owner);
    graphene_assert(admin_one != admin_tables.end(), "admin account is not auth");
    
    auto nft_find_id = nft_tables.find(id);
    graphene_assert(nft_find_id != nft_tables.end(), "nft id is not exist");
    auto nft_find = composeattr_tables.find(id);
    graphene_assert(nft_find != composeattr_tables.end(), "id can not support compose");
    
    composeattr_tables.emplace(get_trx_sender(), [&](auto& composeattr_data) {
        composeattr_data.nftid = id;  
    });     
}

void nft::delcompattr(std::string strowner, id_type id)
{
    graphene_assert(is_account(strowner), "owner account does not exist");
    int64_t owner = get_account_id(strowner.c_str(), strowner.length());
    //require_auth(owner);
    auto admin_one = admin_tables.find(owner);
    graphene_assert(admin_one != admin_tables.end(), "admin account is not auth");
    
    auto nft_find_id = composeattr_tables.find(id);
    graphene_assert(nft_find_id != composeattr_tables.end(), "id can not support compose");
    
    composeattr_tables.erase(nft_find_id);     
}

void nft::setcompose(std::string strowner, id_type firid, id_type secid)
{
    graphene_assert(is_account(strowner), "owner account does not exist");
    int64_t owner = get_account_id(strowner.c_str(), strowner.length());
    //require_auth(owner);
    auto admin_one = admin_tables.find(owner);
    graphene_assert(admin_one != admin_tables.end(), "admin account is not auth");
    if(firid == secid)  {
        graphene_assert(false, "can not group self");
    }
    auto nft_find_firid = composeattr_tables.find(firid);
    graphene_assert(nft_find_firid != composeattr_tables.end(), "firid can not support compose");

    auto nft_find_secid = composeattr_tables.find(secid);
    graphene_assert(nft_find_secid != composeattr_tables.end(), "secid can not support compose");

    auto compose_data = compose_tables.get_index<N(byfir)>();
	auto it = compose_data.lower_bound(firid);

	bool found = true;
	id_type id = 0;
	for(; it!= compose_data.end() && it->firid==firid; ++it){
		if(it->secid == secid) {
			id = it->id;
			found = false;
			break;
		}
	}

	graphene_assert(found, "group is exists");
    compose_tables.emplace(get_trx_sender(), [&](auto& compose_data) {
        compose_data.id = compose_tables.available_primary_key();
        compose_data.firid = firid;
        compose_data.secid = secid;
        compose_data.status = 1;  
    });
}

void nft::delcompose(std::string strowner, id_type firid, id_type secid)
{
    graphene_assert(is_account(strowner), "owner account does not exist");
    int64_t owner = get_account_id(strowner.c_str(), strowner.length());
    //require_auth(owner);
    auto admin_one = admin_tables.find(owner);
    graphene_assert(admin_one != admin_tables.end(), "admin account is not auth");
    auto nft_find_firid = nft_tables.find(firid);
    graphene_assert(nft_find_firid != nft_tables.end(), "firid is not exist");

    auto nft_find_secid = nft_tables.find(secid);
    graphene_assert(nft_find_secid != nft_tables.end(), "secid is not exist");

    auto compose_data = compose_tables.get_index<N(byfir)>();
	auto it = compose_data.lower_bound(firid);

	bool found = false;
	id_type id = 0;
	for(; it!= compose_data.end() && it->firid==firid; ++it){
		if(it->secid == secid) {
			id = it->id;
			found = true;
			break;
		}
	}

	graphene_assert(found, "group is not exists");
    auto group_find_id = compose_tables.find(id);
    compose_tables.erase(group_find_id);
}

void nft::addgame(std::string strowner, std::string gamename, std::string introduces) 
{
	graphene_assert(is_account(strowner), "owner account does not exist");
    int64_t owner = get_account_id(strowner.c_str(), strowner.length());
    //require_auth(owner);
    auto admin_one = admin_tables.find(owner);
    graphene_assert(admin_one != admin_tables.end(), "admin account is not auth");
    graphene_assert(gamename.size() <= 32, "gamename has more than 32 bytes");
    graphene_assert(introduces.size() <= 256, "introduces has more than 256 bytes");
    
    auto game_data = game_tables.get_index<N(bystatus)>();
	auto it = game_data.lower_bound(0);
    bool found = true;
	for(; it != game_data.end() && it->status==1; ++it){
		if(it->gamename == gamename) {
			found = false;
			break;
		}
	}

	graphene_assert(found, "gamename is exists");
    //auto time_now = time_point_sec(now());
    auto time_now = get_head_block_time();
    game_tables.emplace(get_trx_sender(), [&](auto& game_data) {
        game_data.gameid = game_tables.available_primary_key();
        game_data.gamename = gamename;
        game_data.introduces = introduces;
        game_data.createtime = time_now;
        game_data.status = 1;
        game_data.index = 0;
    });
}

void nft::editgame(std::string strowner, id_type gameid, std::string gamename, std::string introduces) 
{
	graphene_assert(is_account(strowner), "owner account does not exist");
	int64_t owner = get_account_id(strowner.c_str(), strowner.length());
    //require_auth(owner);
    auto admin_one = admin_tables.find(owner);
    graphene_assert(admin_one != admin_tables.end(), "admin account is not auth");
    graphene_assert(introduces.size() <= 256, "introduces has more than 256 bytes");
    
    auto game_find = game_tables.find(gameid);
    graphene_assert(game_find != game_tables.end(), "game id is not exist");

    game_tables.modify(game_find, get_trx_sender(), [&](auto& game_data) {
        game_data.gamename = gamename;
        game_data.introduces = introduces;
    });
}

void nft::setgame(std::string strowner, id_type gameid, id_type status) 
{
	graphene_assert(is_account(strowner), "owner account does not exist");
	int64_t owner = get_account_id(strowner.c_str(), strowner.length());
    //require_auth(owner);
    auto admin_one = admin_tables.find(owner);
    graphene_assert(admin_one != admin_tables.end(), "admin account is not auth");
    
    bool statusOk = ((status == 0 || status == 1) ? true : false);

    graphene_assert(statusOk, "status must eq 0 or 1");
    auto game_find = game_tables.find(gameid);
    graphene_assert(game_find != game_tables.end(), "gameid is not exist");

    game_tables.modify(game_find, get_trx_sender(), [&](auto& game_data) {
        game_data.status = status;
    });
}

void nft::delgame(std::string strowner, id_type gameid) 
{
	graphene_assert(is_account(strowner), "owner account does not exist");
	int64_t owner = get_account_id(strowner.c_str(), strowner.length());
    //require_auth(owner);
    auto admin_one = admin_tables.find(owner);
    graphene_assert(admin_one != admin_tables.end(), "admin account is not auth");
    
    auto game_find = game_tables.find(gameid);
    graphene_assert(game_find != game_tables.end(), "gameid is not exist");

    game_tables.erase(game_find);
}

void nft::addgameattr(std::string strowner, id_type gameid, string key, string value) 
{
	graphene_assert(is_account(strowner), "owner account does not exist");
	int64_t owner = get_account_id(strowner.c_str(), strowner.length());
    //require_auth(owner);
    auto admin_one = admin_tables.find(owner);
    graphene_assert(admin_one != admin_tables.end(), "admin account is not auth");
    
    auto game_find = game_tables.find(gameid);
    graphene_assert(game_find != game_tables.end(), "gameid is not exist");
    
    std::vector<attrpair> vectAttr = game_find->gameattr;
    auto iter = find_if(vectAttr.begin(),
                    vectAttr.end(),
                    [=] (const attrpair& m) -> bool { return key == m.key; });
    graphene_assert(iter == vectAttr.end(), "key is exist");
	vectAttr.push_back({key, value});

    game_tables.modify(game_find, get_trx_sender(), [&](auto& attr_data) {
        attr_data.gameattr = vectAttr;
    }); 
}

void nft::editgameattr(std::string strowner, id_type gameid, string key, string value) 
{
	graphene_assert(is_account(strowner), "owner account does not exist");
	int64_t owner = get_account_id(strowner.c_str(), strowner.length());

    //require_auth(owner);
    auto admin_one = admin_tables.find(owner);
    graphene_assert(admin_one != admin_tables.end(), "admin account is not auth");
    
    auto game_find = game_tables.find(gameid);
    graphene_assert(game_find != game_tables.end(), "gameid is not exist");
    
    std::vector<attrpair> vectAttr = game_find->gameattr;
    auto iter = find_if(vectAttr.begin(),
                    vectAttr.end(),
                    [=] (const attrpair& m) -> bool { return key == m.key; });
    graphene_assert(iter != vectAttr.end(), "key is not exist");
    iter->value = value;
    
    game_tables.modify(game_find, get_trx_sender(), [&](auto& attr_data) {
        attr_data.gameattr = vectAttr;
    }); 
}

void nft::delgameattr(std::string strowner, id_type gameid, string key) 
{
	graphene_assert(is_account(strowner), "owner account does not exist");
	int64_t owner = get_account_id(strowner.c_str(), strowner.length());

    //require_auth(owner);
    auto admin_one = admin_tables.find(owner);
    graphene_assert(admin_one != admin_tables.end(), "admin account is not auth");
    
    auto game_find = game_tables.find(gameid);
    graphene_assert(game_find != game_tables.end(), "gameid is not exist");
    
    std::vector<attrpair> vectAttr = game_find->gameattr;
    auto iter = find_if(vectAttr.begin(),
                    vectAttr.end(),
                    [=] (const attrpair& m) -> bool { return key == m.key; });
    graphene_assert(iter != vectAttr.end(), "key is not exist");
    vectAttr.erase(iter);

    game_tables.modify(game_find, get_trx_sender(), [&](auto& attr_data) {
        attr_data.gameattr = vectAttr;
    }); 
}

void nft::addmapping(std::string strowner, id_type fromid, id_type targetid, id_type chainid) 
{
	graphene_assert(is_account(strowner), "owner account does not exist");
	int64_t owner = get_account_id(strowner.c_str(), strowner.length());
    //require_auth(owner);
    auto admin_one = admin_tables.find(owner);
    graphene_assert(admin_one != admin_tables.end(), "admin account is not auth");
    
    auto nft_find = nft_tables.find(fromid);
    graphene_assert(nft_find != nft_tables.end(), "fromid is not exist,nft asset is not exist");
    auto chain_find = nftchain_tables.find(chainid);
    graphene_assert(chain_find != nftchain_tables.end(), "chainid is not exist");
    auto assetmapping_tar_data = assetmap_tables.get_index<N(bytargetid)>();
	auto iter = assetmapping_tar_data.lower_bound(targetid);
    bool founds = true;
	for(; iter != assetmapping_tar_data.end() && iter->targetid==targetid; ++iter) {
		if(iter->chainid == chainid) {
            print(iter->chainid);
            print(iter->targetid);
            print(iter->fromid);
            founds = false;
            break;
		}
	}

	// graphene_assert(founds, "nftmapping_target is exists");
    auto assetmapping_data = assetmap_tables.get_index<N(byfromid)>();
	auto it = assetmapping_data.find(fromid);

    bool found = true;
	for(; it != assetmapping_data.end() && it->fromid==fromid; ++it){
		if(it->chainid == chainid) {
			found = false;
			break;
		}
	}

	graphene_assert(found, "nftmapping_from is exists");

    assetmap_tables.emplace(get_trx_sender(), [&](auto& assetmapping_data) {
        assetmapping_data.mappingid = assetmap_tables.available_primary_key();
        assetmapping_data.fromid = fromid;
        assetmapping_data.targetid = targetid;
        assetmapping_data.chainid = chainid;
    });
}

void nft::delmapping(std::string strowner, id_type fromid, id_type chainid) 
{
	graphene_assert(is_account(strowner), "issuer account does not exist");
	int64_t owner = get_account_id(strowner.c_str(), strowner.length());
    //require_auth(owner);

    auto admin_one = admin_tables.find(owner);
    graphene_assert(admin_one != admin_tables.end(), "admin account is not auth");

    auto nft_find = nft_tables.find(fromid);
    graphene_assert(nft_find != nft_tables.end(), "fromid is not exist,nft asset is not exist");

    auto chain_find = nftchain_tables.find(chainid);
    graphene_assert(chain_find != nftchain_tables.end(), "chainid is not exist");

    auto assetmapping_data = assetmap_tables.get_index<N(byfromid)>();
	auto iter = assetmapping_data.lower_bound(fromid);
    bool found = false;
    id_type assetmap_id=0;
	for( ; iter != assetmapping_data.end() && iter->fromid == fromid; ++iter){
		if(iter->chainid == chainid) {
			found = true;
            assetmap_id = iter->mappingid;
			break;
		}
	}

	graphene_assert(found, "nftmapping is not exists");
    auto nftmap_find = assetmap_tables.find(assetmap_id);
    assetmap_tables.erase(nftmap_find);
}

void nft::createorder(std::string strowner, id_type nftid, contract_asset amount, std::string side, std::string memo)
{
    graphene_assert(is_account(strowner), "owner account does not exist");
    int64_t owner = get_account_id(strowner.c_str(), strowner.length());
    //require_auth(owner);

    //param check
    graphene_assert(side == "buy" || side == "sell", "side must eq buy or sell");
    graphene_assert(memo.size() <= 256, "memo has more than 256 bytes");
    graphene_assert(amount.amount > 0, "amount must be positive");
    graphene_assert(amount.asset_id == 1, "currency must be GXC");

    //nft status check
    auto status_iter = index_tables.find(nftid);
    graphene_assert(status_iter != index_tables.end(), "nft index does not exist");
    graphene_assert(status_iter->status == 1, "nft status is close, can't place order");

    //nft check
    auto asset_iter = nft_tables.find(nftid);
    graphene_assert(asset_iter != nft_tables.end(), "asset does not exist");

    if (side == "sell") {
        graphene_assert(asset_iter->owner == owner, "can't sell other nft asset");
        auto order_data = order_tables.get_index<N(byowner)>();
        auto iter = order_data.lower_bound(owner);
        bool isValid = true;
        for( ; iter != order_data.end() && iter->nftid == nftid; ++iter){
            if(iter->side == "sell") {
                isValid = false;  //one nft one sell
                break;  
            }
        }
        graphene_assert(isValid, "nft sell order is not valid");
    } else {
        graphene_assert(asset_iter->owner != owner, "Can't buy your own nft asset");
    }

    order_tables.emplace(get_trx_sender(), [&](auto& order) {
        order.id = order_tables.available_primary_key();
        order.nftid = nftid;
        order.owner = owner;
        order.price = amount;
        order.side = side;
        order.memo = memo;
        order.createtime = get_head_block_time();
    });
}

void nft::cancelorder(std::string strowner, id_type id, std::string memo)
{
    graphene_assert(is_account(strowner), "owner account does not exist");
    int64_t owner = get_account_id(strowner.c_str(), strowner.length());
    //require_auth(owner);

    auto iter = order_tables.find(id);
    graphene_assert(iter != order_tables.end(), std::string(std::to_string(id) + ", cancel failed, order is not exist, " + memo).c_str() );
    graphene_assert(iter->owner == owner, "owner is not equal");
    auto status_iter = index_tables.find(iter->nftid);
    graphene_assert(status_iter != index_tables.end(), "nft index does not exist");
    graphene_assert(status_iter->status == 1, "nft status is close, can't place order");
    order_tables.erase(iter);

    //transfer token//todo
    if(iter->side == "buy") {
        //action(permission_level{ _self, "active"_n },
        //    "eosio.token"_n, "transfer"_n,
        //    std::make_tuple(_self, owner, iter->price, std::string("cancel order")))
        //.send();
    }
}

void nft::trade(std::string strfrom, std::string strto, id_type orderid, const std::string& side, const std::string& memo)
{
    graphene_assert(is_account(strfrom), "from account does not exist");
    graphene_assert(is_account(strto), "to account does not exist");
    graphene_assert(side == "buy" || side == "sell", "side must be buy or sell");
	int64_t from = get_account_id(strfrom.c_str(), strfrom.length());
    int64_t to = get_account_id(strto.c_str(), strto.length());
    //require_auth(from);
    //require_auth(to);
    graphene_assert(memo.size() <= 256, "memo has more than 256 bytes");

    auto iter = order_tables.find(orderid);
    graphene_assert(iter != order_tables.end(), std::string(std::to_string(orderid) + ", trade failed, order is not exist").c_str() );
    order_tables.erase(iter);

    auto status_iter = index_tables.find(iter->nftid);
    graphene_assert(status_iter != index_tables.end(), "nft index does not exist");
    graphene_assert(status_iter->status == 1, "nft status is close");


    auto nft_iter = nft_tables.find(iter->nftid);
    graphene_assert(nft_iter != nft_tables.end(), "nft asset is not exist");
    if (iter->side == "buy" && side == "sell") {
        graphene_assert(nft_iter->owner == to, "buy order, owner is not equal to account");
    } else if (iter->side == "sell" && side == "buy") {
        graphene_assert(nft_iter->owner == iter->owner, "sell order, owner is not equal from account");
    } else {
        graphene_assert(false, "invalid side");
    }
    nft_tables.modify(nft_iter, _self, [&](auto& nft_data) {
        nft_data.auth = to;
        nft_data.owner = to;
     });

    //from nft number -1
    auto nftnum = nftnumber_tables.find(from);
    if(nftnum->number != 1) {
        nftnumber_tables.modify(nftnum, get_trx_sender(), [&](auto& nftnum_data) {
            nftnum_data.number = nftnum->number - 1;
        });
    } else {
        nftnumber_tables.erase(nftnum);
    }

    //to nft number +1
    auto nfttonum = nftnumber_tables.find(to);
    if(nfttonum != nftnumber_tables.end()) {
        nftnumber_tables.modify(nfttonum, get_trx_sender(), [&](auto& nftnum_data) {
            nftnum_data.number = nfttonum->number + 1;
        });
    } else {
        nftnumber_tables.emplace(to, [&](auto& nftnum_data) {
            nftnum_data.owner = to;
            nftnum_data.number = 1;
        });
    }

    auto amount = iter->price;
    amount.amount = amount.amount - FEE;
    if (amount.amount > 0) {
	    //todo()实现合约内部充值转账GXC功能
        //action(permission_level{ _self, "active"_n },
        //    "eosio.token"_n, "transfer"_n,
        //    std::make_tuple(_self, from, amount, memo))
        //.send();
    }
}

void nft::parse_memo(std::string memo, std::string& action, std::map<std::string, std::string>& params) {
    // remove space
    memo.erase(std::remove_if(memo.begin(), memo.end(), [](unsigned char x) { 
            return std::isspace(x); }), memo.end());

    size_t pos;
    std::string container;
    pos = sub2sep(memo, container, '-', 0, true);
    graphene_assert(!container.empty(), "no action");
    action = container;
    if (container == "order") {
        pos = sub2sep(memo, container, '-', ++pos, true);
        graphene_assert(!container.empty(), "no owner");
        params["owner"] = container;

        pos = sub2sep(memo, container, '-', ++pos, true);
        graphene_assert(!container.empty(), "no nft id");
        params["nftid"] = container;

        pos = sub2sep(memo, container, '-', ++pos, true);
        graphene_assert(!container.empty(), "no price");
        params["price"] = container;

        pos = sub2sep(memo, container, '-', ++pos, true);
        graphene_assert(!container.empty(), "no side");
        graphene_assert(container == "buy" || container == "sell", "side must be buy or sell");
        params["side"] = container;
    } else if (container == "trade") {
        pos = sub2sep(memo, container, '-', ++pos, true);
        graphene_assert(!container.empty(), "no from account");
        params["from"] = container;

        pos = sub2sep(memo, container, '-', ++pos, true);
        graphene_assert(!container.empty(), "no to account");
        params["to"] = container;

        pos = sub2sep(memo, container, '-', ++pos, true);
        graphene_assert(!container.empty(), "no order id");
        params["id"] = container;

        pos = sub2sep(memo, container, '-', ++pos, true);
        graphene_assert(!container.empty(), "no side");
        graphene_assert(container == "buy" || container == "sell", "side must be buy or sell");
        params["side"] = container;
    } else if (container == "cancel") {
        pos = sub2sep(memo, container, '-', ++pos, true);
        graphene_assert(!container.empty(), "no from account");
        params["owner"] = container;

        pos = sub2sep(memo, container, '-', ++pos, true);
        graphene_assert(!container.empty(), "no order id");
        params["id"] = container;
    }
}

void nft::transfer(std::string strfrom, std::string strto, const contract_asset& quantity, const std::string& memo) {
    graphene_assert(is_account(strfrom), "from account does not exist");
    graphene_assert(is_account(strto), "to account does not exist");
    //todo()增加使用asset结构体如{1.00, 'GXC'}，
    //graphene_assert(quantity.symbol.is_valid(), "invalid symbol name");
    //check(quantity.symbol.code().to_string() == "EOS", "currency must be EOS");
    graphene_assert(quantity.amount >= FEE, "transfer fee too small");
    graphene_assert(memo.size() <= 256 && !memo.empty(), "memo has more than 256 bytes or is empty");
    int64_t from = get_account_id(strfrom.c_str(), strfrom.length());
    int64_t to = get_account_id(strto.c_str(), strto.length());

    if (from == _self || to != _self) {
        return;
    }

    std::map<std::string, std::string> params;
    std::string action;
    parse_memo(memo, action, params);
    graphene_assert(action == "order" || action == "trade" || action == "cancel", "memo action error");
    graphene_assert(params.size() > 0, "memo error");

    if(action == "order") {
        graphene_assert(params.find("owner") != params.end() && params.find("nftid") != params.end()
            && params.find("price") != params.end() && params.find("side") != params.end(), 
            "create order param error");
        std::string owner = params["owner"];
        std::string id = params["nftid"];
        uint64_t nftid = stoi(id);
        std::string price = params["price"];
        uint64_t nPrice = stoi(price);
        std::string side = params["side"];
        auto amount = quantity;
        if (side == "buy") {
            graphene_assert(amount.amount == nPrice, "buy nft, transfer quantity not equal price");
        } else {
            amount.amount = nPrice;
        }
        createorder(owner, nftid, amount, side, memo);
    } else if(action == "trade") {
        graphene_assert(params.find("from") != params.end() && params.find("to") != params.end()
            && params.find("id") != params.end() && params.find("side") != params.end(),
            "trade param error");
        std::string id = params["id"];
        uint64_t order_id = stoi(id);
        if(params["side"] == "buy") {
            graphene_assert(strfrom == params["to"], "transfer from not equal memo to account");
        } else {
            graphene_assert(strfrom == params["from"], "transfer from not equal memo from account");
        }
        trade(params["from"], params["to"], order_id, params["side"], memo);
    } else if(action == "cancel") {
        graphene_assert(params.find("owner") != params.end() && params.find("id") != params.end(), 
            "cancel order param error");
        graphene_assert(quantity.amount >= FEE, "cancel order, fee not enough");
        graphene_assert(strfrom == params["owner"], "transfer from not equal memo owner");
        std::string id = params["id"];
        uint64_t order_id = stoi(id);
        cancelorder(params["owner"], order_id, memo);
    }
}
GRAPHENE_ABI(nft, (addadmin)(deladmin)(create)(createother)(addnftattr)(editnftattr)(delnftattr)(addaccauth)
    (delaccauth)(addnftauth)(delnftauth)(transfernft)(addchain)(setchain)(addcompattr)(delcompattr)(setcompose)
    (delcompose)(addgame)(setgame)(editgame)(delgame)(addgameattr)(editgameattr)(delgameattr)(addmapping)
    (delmapping)(burn))