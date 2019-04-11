#include "nft.hpp"

bool is_account( const graphenelib::name& account ) {
  int64_t account_id = get_account_id(account.to_string().c_str(), account.to_string().size());
  return account_id >= 0;
}
   
void nft::addadmin(graphenelib::name admin) 
{
	//require_auth(_self);
    graphene_assert(is_account(admin), "admin account does not exist");
 
    auto admin_one = admin_tables.find(admin.value);
    graphene_assert(admin_one == admin_tables.end(), "admin account already authed");

    admin_tables.emplace(_self, [&](auto& admin_data) {
        admin_data.admin = admin;
    });
}

void nft::deladmin(graphenelib::name admin) 
{
	//require_auth(_self);
    graphene_assert(is_account(admin), "admin account does not exist");

    auto admin_one = admin_tables.find(admin.value);
    graphene_assert(admin_one != admin_tables.end(), "admin account is not auth");

    admin_tables.erase(admin_one);
}

void nft::create(graphenelib::name creator, graphenelib::name owner, std::string explain, std::string worldview) 
{
    graphene_assert(is_account(creator), "creator account does not exist");
    graphene_assert(is_account(owner), "creator account does not exist");

    graphene_assert(explain.size() <= 256, "explain has more than 256 bytes");
    graphene_assert(worldview.size() <= 20, "worldview has more than 20 bytes");
    //require_auth(creator);

    auto admin_one = admin_tables.find(creator.value);
    graphene_assert(admin_one != admin_tables.end(), "admin account is not auth");

    id_type index_id = index_tables.available_primary_key();
    index_tables.emplace(creator, [&](auto& index_data) {
        index_data.id = index_id;
        index_data.status = 1;
    });

    // Create new nft
    //auto time_now = time_point_sec(now());
    auto time_now = get_head_block_time();
    nft_tables.emplace(creator, [&](auto& nft_data) {
        nft_data.id = index_id;
        nft_data.creator=creator;
        nft_data.owner = owner;
        nft_data.auth = owner;
        nft_data.explain = explain;
        nft_data.createtime = time_now;
        nft_data.worldview = worldview;
    });

    auto nft_num = nftnumber_tables.find(owner.value);
    if(nft_num != nftnumber_tables.end()){
        nftnumber_tables.modify(nft_num, creator, [&](auto& nft_num_data) {
            nft_num_data.number = nft_num->number+1;
        });
    }
    else 
    {
        nftnumber_tables.emplace(creator, [&](auto& nft_num_data) {
            nft_num_data.owner = owner;
            nft_num_data.number = 1;
        });   
    }
}

void nft::createother(graphenelib::name creator, graphenelib::name owner, std::string explain, std::string worldview, id_type chainid, id_type targetid) 
{
    graphene_assert(is_account(creator), "creator account does not exist");
    graphene_assert(is_account(owner), "creator account does not exist");

    graphene_assert(explain.size() <= 256, "explain has more than 64 bytes");
    graphene_assert(worldview.size() <= 20, "worldview has more than 20 bytes");

	//require_auth(creator);
    
    auto admin_one = admin_tables.find(creator.value);
    graphene_assert(admin_one != admin_tables.end(), "admin account is not auth");
    
    auto chain_find = nftchain_tables.find(chainid);
    graphene_assert(chain_find != nftchain_tables.end(), "chainid is not exist");

    // Create new nft
    //auto time_now = time_point_sec(now());
    auto time_now = get_head_block_time();
    id_type indexid = index_tables.available_primary_key();
    index_tables.emplace(creator, [&](auto& index_data) {
        index_data.id = indexid;
        index_data.status = 1;
    });

    //id_type newid = nft_tables.available_primary_key();
    nft_tables.emplace(creator, [&](auto& nft_data) {
        nft_data.id = indexid;
        nft_data.creator=creator;
        nft_data.owner = owner;
        nft_data.auth = owner;
        nft_data.explain = explain;
        nft_data.createtime = time_now;
        
        nft_data.worldview = worldview;
    });

    assetmap_tables.emplace(creator, [&](auto& assetmapping_data) {
        assetmapping_data.mappingid = game_tables.available_primary_key();
        assetmapping_data.fromid = indexid;
        assetmapping_data.targetid = targetid;
        assetmapping_data.chainid = chainid;
    });

    auto nftnum = nftnumber_tables.find(owner.value);
    if(nftnum != nftnumber_tables.end()){
        nftnumber_tables.modify(nftnum,creator, [&](auto& nftnum_data) {
            nftnum_data.number = nftnum->number+1;
        });
    }
    else 
    {
        nftnumber_tables.emplace(creator, [&](auto& nftnum_data) {
            nftnum_data.owner = owner;
            nftnum_data.number = 1;
        });   
    }

    //print(time_now);
}

void nft::addaccauth(graphenelib::name owner,graphenelib::name auth) 
{
    //require_auth(owner);
    
    graphene_assert(is_account(owner), "account owner does not exist");
    graphene_assert(is_account(auth), "account auth does not exist");

    auto auth_find = accauth_tables.find(owner.value);
    graphene_assert(auth_find == accauth_tables.end(), "owner account already authed");

    accauth_tables.emplace(owner, [&](auto& auth_data) {
        auth_data.owner = owner;
        auth_data.auth = auth;
    });
}

void nft::delaccauth(graphenelib::name owner) 
{
    //require_auth(owner);
    graphene_assert(is_account(owner), "account owner does not exist");

    auto auth_find = accauth_tables.find(owner.value);
    graphene_assert(auth_find != accauth_tables.end(), "owner has not auth");

    accauth_tables.erase(auth_find);
}

void nft::addnftauth(graphenelib::name owner, graphenelib::name auth, id_type id)
{
    //require_auth(owner);
    graphene_assert(is_account(owner), "account owner does not exist");
    graphene_assert(is_account(auth), "account auth does not exist");

    auto nft_find_id = nft_tables.find(id);
    graphene_assert(nft_find_id != nft_tables.end(), "nft id is not exist");

    if(nft_find_id->owner != owner){
        auto nft_accauth_find = accauth_tables.find(owner.value);
        graphene_assert(nft_accauth_find != accauth_tables.end(), "account has not auth"); 
        graphene_assert(nft_accauth_find->auth != owner, "account has not auth");         
    }

    nft_tables.modify(nft_find_id, owner, [&](auto& nft_data) {
        nft_data.auth = auth;
    });    
}

void nft::delnftauth(graphenelib::name owner, id_type id)
{
    //require_auth(owner);
    graphene_assert(is_account(owner), "account owner does not exist");

    auto nft_find_id = nft_tables.find(id);
    graphene_assert(nft_find_id != nft_tables.end(), "nft id is not exist");

    if(nft_find_id->owner != owner){
        auto nft_accauth_find = accauth_tables.find(owner.value);
        graphene_assert(nft_accauth_find != accauth_tables.end(), "account has not auth"); 
        graphene_assert(nft_accauth_find->auth != owner, "account has not auth");         
    }

    nft_tables.modify(nft_find_id, owner, [&](auto& nft_data) {
        nft_data.auth = owner;
    });    
}

void nft::transfer(graphenelib::name from, graphenelib::name to, id_type id, string memo)
{
    //require_auth(from);
    graphene_assert(is_account(from), "from auth does not exist");
    graphene_assert(is_account(to), "to auth does not exist");

    graphene_assert(memo.size() <= 256, "memo has more than 256 bytes");

    auto nft_find_id = nft_tables.find(id);
    graphene_assert(nft_find_id != nft_tables.end(), "nft id is not exist");

    graphenelib::name owner_nft = nft_find_id->owner;
    if(nft_find_id->owner != from){
        if(nft_find_id->auth != from){
            auto nft_accauth_find = accauth_tables.find(nft_find_id->owner.value);
            graphene_assert(nft_accauth_find != accauth_tables.end(), "from has not auth"); 
            graphene_assert(nft_accauth_find->auth != from, "from has not auth"); 
        }       
    }

    nft_tables.modify(nft_find_id, from, [&](auto& nft_data) {
        nft_data.auth = to;
        nft_data.owner = to;
     });

    auto nftnum = nftnumber_tables.find(owner_nft.value);
    if(nftnum->number != 1){
            nftnumber_tables.modify(nftnum,from, [&](auto& nftnum_data) {
            nftnum_data.number = nftnum->number-1;
        });
    }
    else 
    {
        nftnumber_tables.erase(nftnum);   
    }

    auto nfttonum = nftnumber_tables.find(to.value);
    if(nfttonum != nftnumber_tables.end()){
        nftnumber_tables.modify(nfttonum, from, [&](auto& nftnum_data) {
            nftnum_data.number = nfttonum->number+1;
        });
    }
    else 
    {
        nftnumber_tables.emplace(from, [&](auto& nftnum_data) {
            nftnum_data.owner = to;
            nftnum_data.number = 1;
        });   
    }  
}

void nft::addchain(graphenelib::name owner,string chain)
{
    graphene_assert(is_account(owner), "issuer account does not exist");

    graphene_assert(chain.size() <= 64, "explain has more than 64 bytes");
	//require_auth(owner);
    auto admin_one = admin_tables.find(owner.value);
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
    nftchain_tables.emplace(owner, [&](auto& nftchain_data) {
        nftchain_data.chainid = nftchain_tables.available_primary_key();
        nftchain_data.chain = chain;
        nftchain_data.status = 1;  
    });
}

void nft::setchain(graphenelib::name owner,id_type chainid,id_type status)
{
    graphene_assert(is_account(owner), "issuer account does not exist");
    //require_auth(owner);

    auto admin_one = admin_tables.find(owner.value);
    graphene_assert(admin_one != admin_tables.end(), "admin account is not auth");
   
    auto nftchain_find = nftchain_tables.find(chainid);
    graphene_assert(nftchain_find != nftchain_tables.end(), "chainid is not exists");

    bool found = true;
    if(status == 0 || status == 1){
        found = true;
    }
    else
    {
        found = false;
    }

    graphene_assert(found, "status must eq 0 or 1");
    nftchain_tables.modify(nftchain_find, owner, [&](auto& nftchain_data) {
        nftchain_data.status = status;  
    });
}

void nft::addcompattr(graphenelib::name owner, id_type id)
{
    graphene_assert(is_account(owner), "issuer account does not exist");

    //require_auth(owner);
    auto admin_one = admin_tables.find(owner.value);
    graphene_assert(admin_one != admin_tables.end(), "admin account is not auth");
    
    auto nft_find_id = nft_tables.find(id);
    graphene_assert(nft_find_id != nft_tables.end(), "nft id is not exist");
    auto nft_find = composeattr_tables.find(id);
    graphene_assert(nft_find != composeattr_tables.end(), "id can not support compose");
    
    composeattr_tables.emplace(owner, [&](auto& composeattr_data) {
        composeattr_data.nftid = id;  
    });     
}

void nft::delcompattr(graphenelib::name owner, id_type id)
{
    graphene_assert(is_account(owner), "issuer account does not exist");
    //require_auth(owner);
    auto admin_one = admin_tables.find(owner.value);
    graphene_assert(admin_one != admin_tables.end(), "admin account is not auth");
    
    auto nft_find_id = composeattr_tables.find(id);
    graphene_assert(nft_find_id != composeattr_tables.end(), "id can not support compose");
    
    composeattr_tables.erase(nft_find_id);     
}

void nft::setcompose(graphenelib::name owner, id_type firid, id_type secid)
{
    graphene_assert(is_account(owner), "issuer account does not exist");
    //require_auth(owner);
    auto admin_one = admin_tables.find(owner.value);
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
    compose_tables.emplace(owner, [&](auto& compose_data) {
        compose_data.id = compose_tables.available_primary_key();
        compose_data.firid = firid;
        compose_data.secid = secid;
        compose_data.status = 1;  
    });
}

void nft::delcompose(graphenelib::name owner, id_type firid, id_type secid)
{
    graphene_assert(is_account(owner), "issuer account does not exist");
    //require_auth(owner);
    auto admin_one = admin_tables.find(owner.value);
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

void nft::addgame(graphenelib::name owner, std::string gamename, std::string introduces) 
{
	graphene_assert(is_account(owner), "issuer account does not exist");
    //require_auth(owner);
    auto admin_one = admin_tables.find(owner.value);
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
    game_tables.emplace(owner, [&](auto& game_data) {
        game_data.gameid = game_tables.available_primary_key();
        game_data.gamename = gamename;
        game_data.introduces = introduces;
        game_data.createtime = time_now;
        game_data.status = 1;
        game_data.index = 0;
    });
}

void nft::editgame(graphenelib::name owner, id_type gameid, std::string gamename, std::string introduces) 
{
	graphene_assert(is_account(owner), "issuer account does not exist");
    //require_auth(owner);
    auto admin_one = admin_tables.find(owner.value);
    graphene_assert(admin_one != admin_tables.end(), "admin account is not auth");
    graphene_assert(introduces.size() <= 256, "introduces has more than 256 bytes");
	graphene_assert(is_account(owner), "issuer account does not exist");
    
    auto game_find = game_tables.find(gameid);
    graphene_assert(game_find != game_tables.end(), "game id is not exist");

    game_tables.modify(game_find,owner, [&](auto& game_data) {
        game_data.gamename = gamename;
        game_data.introduces = introduces;
    });
}

void nft::setgame(graphenelib::name owner, id_type gameid, id_type status) 
{
	graphene_assert(is_account(owner), "issuer account does not exist");
    //require_auth(owner);
    auto admin_one = admin_tables.find(owner.value);
    graphene_assert(admin_one != admin_tables.end(), "admin account is not auth");
    
    // Create new nft
    bool found = true;
    if(status == 0 || status == 1)
    {
        found = true;
    }
    else
    {
        found = false;
    }

    graphene_assert(found, "status must eq 0 or 1");
    auto game_find = game_tables.find(gameid);
    graphene_assert(game_find != game_tables.end(), "gameid is not exist");

    game_tables.modify(game_find,owner, [&](auto& game_data) {
        game_data.status = status;
    });
}

void nft::delgame(graphenelib::name owner, id_type gameid) 
{
	graphene_assert(is_account(owner), "issuer account does not exist");
    //require_auth(owner);
    auto admin_one = admin_tables.find(owner.value);
    graphene_assert(admin_one != admin_tables.end(), "admin account is not auth");
    
    auto game_find = game_tables.find(gameid);
    graphene_assert(game_find != game_tables.end(), "gameid is not exist");

    game_tables.erase(game_find);
}

// void nft::addgameattr(name owner, id_type gameid, string key, string value) 
// {
// 	graphene_assert(is_account(owner), "issuer account does not exist");
//     //require_auth(owner);
//     auto admin_one = admin_tables.find(owner.value);
//     graphene_assert(admin_one != admin_tables.end(), "admin account is not auth");
    
//     auto game_find = game_tables.find(gameid);
//     graphene_assert(game_find != game_tables.end(), "gameid is not exist");
//     std::map<string, string> introducesmap;

//     introducesmap = game_find->gameattr;
//     auto iter = introducesmap.find(key);
//     graphene_assert(iter == introducesmap.end(), "key is exist");
//     introducesmap.insert(std::pair<string, string>(key, value));  
//     game_tables.modify(game_find, owner, [&](auto& attr_data) {
//         attr_data.gameattr = introducesmap;
//     }); 
// }

// void nft::editgameattr(graphenelib::name owner, id_type gameid, string key, string value) 
// {
// 	graphene_assert(is_account(owner), "issuer account does not exist");
//     //require_auth(owner);
//     auto admin_one = admin_tables.find(owner.value);
//     graphene_assert(admin_one != admin_tables.end(), "admin account is not auth");
    
//     auto game_find = game_tables.find(gameid);
//     graphene_assert(game_find != game_tables.end(), "gameid is not exist");
//     std::map<string, string> introducesmap;

//     introducesmap = game_find->gameattr;
//     auto iter = introducesmap.find(key);
//     graphene_assert(iter != introducesmap.end(), "key is not exist");
//     introducesmap[key] = value;  
//     game_tables.modify(game_find, owner, [&](auto& attr_data) {
//         attr_data.gameattr = introducesmap;
//     }); 
// }

// void nft::delgameattr(graphenelib::name owner, id_type gameid, string key) 
// {
// 	graphene_assert(is_account(owner), "issuer account does not exist");
//     //require_auth(owner);
//     auto admin_one = admin_tables.find(owner.value);
//     graphene_assert(admin_one != admin_tables.end(), "admin account is not auth");
    
//     auto game_find = game_tables.find(gameid);
//     graphene_assert(game_find != game_tables.end(), "gameid is not exist");
//     std::map<string, string> introducesmap;

//     introducesmap = game_find->gameattr;
//     auto iter = introducesmap.find(key);
//     graphene_assert(iter != introducesmap.end(), "key is not exist");
//     introducesmap.erase(key); 

//     game_tables.modify(game_find, owner, [&](auto& attr_data) {
//         attr_data.gameattr = introducesmap;
//     }); 
// }

void nft::addmapping(graphenelib::name owner, id_type fromid, id_type targetid, id_type chainid) 
{
	graphene_assert(is_account(owner), "issuer account does not exist");
    //require_auth(owner);
    auto admin_one = admin_tables.find(owner.value);
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
        // print(it->fromid);
        // //print(it->mappingid);
		if(it->chainid == chainid) {
			found = false;
			break;
		}
	}

	graphene_assert(found, "nftmapping_from is exists");

    assetmap_tables.emplace(owner, [&](auto& assetmapping_data) {
        assetmapping_data.mappingid = assetmap_tables.available_primary_key();
        assetmapping_data.fromid = fromid;
        assetmapping_data.targetid = targetid;
        assetmapping_data.chainid = chainid;
    });
}

void nft::delmapping(graphenelib::name owner, id_type fromid, id_type chainid) 
{
	graphene_assert(is_account(owner), "issuer account does not exist");
    //require_auth(owner);

    auto admin_one = admin_tables.find(owner.value);
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

void nft::burn(graphenelib::name owner, id_type nftid) 
{
	graphene_assert(is_account(owner), "issuer account does not exist");
    //require_auth(owner);
    auto admin_one = admin_tables.find(owner.value);
    graphene_assert(admin_one != admin_tables.end(), "admin account is not auth");
    auto nft_find = nft_tables.find(nftid);
 
    graphene_assert(nft_find != nft_tables.end(), "fromid is not exist,nft asset is not exist");
    graphene_assert(nft_find->owner == owner, "owner account insufficient privilege");
    nft_tables.erase(nft_find);
    auto nftnum = nftnumber_tables.find(nft_find->owner.value);
    if(nftnum->number != 1){
        nftnumber_tables.modify(nftnum,owner, [&](auto& nftnum_data) {
            nftnum_data.number = nftnum->number-1;
        });
    }
    else 
    {
        nftnumber_tables.erase(nftnum);   
    }

    auto index_id = index_tables.find(nftid);
    index_tables.modify(index_id, _self, [&](auto& index_data) {
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

GRAPHENE_ABI(nft, (addadmin)(deladmin)(create)(createother)(addaccauth)(delaccauth)(addnftauth)(delnftauth)
     (transfer)(addchain)(setchain)(addcompattr)(delcompattr)(setcompose)(delcompose)(addgame)(setgame)(editgame)
     (delgame)(addgameattr)(editgameattr)(delgameattr)(addmapping)(delmapping)(burn))