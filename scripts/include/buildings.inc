//-----------------------------------------------------------------------------

include "include/logerror";
include "include/coords";

//-----------------------------------------------------------------------------

const BUILDINGS_CFG := ":datacfg:bld_";

//-----------------------------------------------------------------------------

var BuildCfg, BuildCfgRealm;

//=============================================================================
//-----------------------------------------------------------------------------
// Buildings
//-----------------------------------------------------------------------------
//=============================================================================

function IsInBuilding(x,y,Realm)
  
  if(!x or !y or !Realm)
    return 0;
  endif
  
  if(!BuildCfg or BuildCfgRealm != Realm)
    BuildCfg := ReadConfigFile(BUILDINGS_CFG+Realm);
    BuildCfgRealm := Realm;
  endif

  if(!BuildCfg)
    LogError("IsInBuilding: Nie znaleziono konfiga budynkow R: "+Realm+" Cfg: "+BuildCfg);
    return 0;
  endif

  var City := FindCityCoords(x,y,Realm);
  var Cfg := ReadConfigFile(":rhousecfg:"+lower(City));
  var ExGrounds := GetCityBuildingsExGround(City);
  var IsDoor := 1;
  var i;

  if(ExGrounds[CStr(x)+"-"+CStr(y)])
    return 0;
  endif

  for(i:=1;i<=GetConfigMaxIntKey(Cfg); i:=i+1)
    var DoorX := FindConfigElem(Cfg, i).Door1X;
    var DoorY := FindConfigElem(Cfg, i).Door1Y;

    if(DoorX == x and DoorY == y)
      IsDoor := 0;
    endif
  endfor

  if(BuildCfg[CStr(x)+"-"+CStr(y)] and IsDoor)
    return 1;
  endif
  return 0;

endfunction

//-----------------------------------------------------------------------------

function IsInBuildingEx(x,y,Realm, z := error)

  if(z == error)
    z := GetStandZ(x, y, Realm);
  endif

  if(z == error)
    return 0;
  endif

  var mZ := GetStandZ(x, y, Realm, z+20);
  if(DistZ(mZ, z) >= 20 or mZ == error)
    return 1;
  else
    mZ := GetStandZ(x, y, Realm, z+30);
    if(DistZ(mZ, z) >= 30 or mZ == error)
      return 1;
    else
      mZ := GetStandZ(x, y, Realm, z+40);
      if(DistZ(mZ, z) >= 40 or mZ == error)
        return 1;
      endif
    endif
  endif

  return 0;

endfunction

//-----------------------------------------------------------------------------

function AddBuildingCoords(x,y,City)

  UnloadConfigFile(BUILDINGS_CFG);
  BuildCfg := ReadConfigFile(BUILDINGS_CFG);

  if(CInt(x) and CInt(y))
    if(!IsInBuilding(x,y,GetCityRealm(City)))
      AppendConfigFileElem(BUILDINGS_CFG+GetCityRealm(City),"Bd",CStr(CInt(x))+"-"+CStr(CInt(y)),{});
      return 1;
    endif
  endif
  
  return 0;

endfunction

//-----------------------------------------------------------------------------
