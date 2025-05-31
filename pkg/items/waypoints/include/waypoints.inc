//-----------------------------------------------------------------------------

include "include/objtypes/misc";

//-----------------------------------------------------------------------------

const PROP_WAYPOINTS := "Waypoints";
const PROP_WPDELAY   := "WPDelay";
const PROP_WAYPOINT_NAME := "WaypointName";
const CFG_WAYPOINTS := ":waypoints:waypoints";

//---------------------------------------------------------------------

const MIN_WAYPOINT_DIST := 200;
const MAX_WAYPOINT_DIST := 15000;

const WP_RETURN_COST_MULT := 0.8;
const WP_BASE_PRICE     := 800;
const WP_STEP_PRICE     := 10;

const WAYPOINT_DELAY    := 900; // domyœlny delay

//-----------------------------------------------------------------------------

function GetMarkedWaypoints(who)

  var Data := GetObjProperty(who, PROP_WAYPOINTS);

  if(!Data)
    Data := dictionary;
  endif

  return Data;

endfunction

//-----------------------------------------------------------------------------

function MarkWaypoint(who, Name)

  var Data := GetMarkedWaypoints(who);

  if(Data.exists(Name))
    return 0;
  endif

  Data[Name] := ReadGameClock();
  SetObjProperty(who, PROP_WAYPOINTS, Data);

  return 1;

endfunction

//-----------------------------------------------------------------------------

function GetTravelLocations(who, srcLoc)

  var Cfg := ReadConfigFile(CFG_WAYPOINTS);
  var Travels := array;

  SendSysMessage(who, "Cfg len is " + len(Cfg));

  var keys := GetConfigStringKeys(Cfg);

  foreach key in keys
    var CfgElem := Cfg[key];
    var Loc := GetLocFromString(CfgElem.Coords);
    var Dist := coordist(Loc.x, Loc.y, srcLoc.x, srcLoc.y);

    if(Dist <= MAX_WAYPOINT_DIST and Dist >= MIN_WAYPOINT_DIST)
      var Data := struct;
      Data.+Dist := Dist;
      Data.+PointDist := CInt(ceil(CDbl(Dist)/100.0));
      Data.+Price := CalcTravelPrice(CfgElem.Level, Data.PointDist);
      Data.+Desc := CfgElem.Desc;
      Data.+Loc := Loc;
      Data.+Level := CfgElem.Level;
      Travels.Append(Data);
    endif

  endforeach

  var TeleCrystals := GetObjProperty(who, "PersonalTeleCrysLoc");

  if(!IsArray(TeleCrystals[1]) and TeleCrystals)
    TeleCrystals := array{TeleCrystals};
    SetObjProperty(who, "PersonalTeleCrysLoc", TeleCrystals);
  endif

  if(TeleCrystals)
    foreach TeleCrystal in TeleCrystals
      var Loc := struct;
      Loc.+x := TeleCrystal[1];
      Loc.+y := TeleCrystal[2];
      Loc.+z := TeleCrystal[3];
      Loc.+realm := TeleCrystal[4];
      var Dist := coordist(Loc.x, Loc.y, srcLoc.x, srcLoc.y);
      var Data := struct;
      Data.+Dist := Dist;
      Data.+PointDist := CInt(ceil(CDbl(Dist)/100.0));
      Data.+Price := CalcTravelPrice("Normal", Data.PointDist);
      Data.+Desc := TeleCrystal[5];
      Data.+Loc := Loc;
      Data.+Level := "Normal";
      Travels.Append(Data);
    endforeach
  endif

  return Travels;

endfunction

//-----------------------------------------------------------------------------

function GetWaypointElem(Stone)

  var Name := __IfElse(Stone.isa(POLCLASS_ITEM), GetObjProperty(Stone, PROP_WAYPOINT_NAME), Stone);

  if(!Name)
    return 0;
  endif

  var Cfg := ReadConfigFile(CFG_WAYPOINTS);

  return Cfg[Name];

endfunction

//-----------------------------------------------------------------------------

function CalcTravelPriceForWaypoint(Name, srcLoc)

  var CfgElem := GetWaypointElem(Name);
  
  if(CfgElem)
    var Loc := GetLocFromString(CfgElem.Coords);
    var Dist := coordist(Loc.x, Loc.y, srcLoc.x, srcLoc.y);
    Dist := CInt(ceil(CDbl(Dist)/100.0));
    return CalcTravelPrice(CfgElem.Level, Data.PointDist);
  endif

  return 0;

endfunction

//-----------------------------------------------------------------------------

function CalcTravelPrice(Level, Dist)

  var Mult;

  case(Level)
    "City":   Mult := 1.0;
    "Normal": Mult := 2.5;
    "Hard":   Mult := 4.5;
    default:  Mult := 2.4;
  endcase

  var Price := Dist * WP_STEP_PRICE;

  return WP_BASE_PRICE + CInt(Price * Mult);

endfunction

//-----------------------------------------------------------------------------

function CanUseWaypoint(who)

  var Delay := CInt(GetObjProperty(who, PROP_WPDELAY));

  if(Delay and Delay > ReadGameClock())
    return 0;
  endif

  return 1;

endfunction

//-----------------------------------------------------------------------------

function SetWaypointDelay(who, Delay := WAYPOINT_DELAY)

  if(!IsStaff(who))
    SetObjProperty(who, PROP_WPDELAY, ReadGameClock() + Delay);
  endif

endfunction

//-----------------------------------------------------------------------------
