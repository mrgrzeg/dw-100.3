//-----------------------------------------------------------------------------

use uo;
use os;

//=============================================================================
//-----------------------------------------------------------------------------
// Inne
//-----------------------------------------------------------------------------
//=============================================================================

const VAR_DICTIONARY  := "Dictionary";
const VAR_STRUCT      := "Struct";
const VAR_INT         := "Integer";
const VAR_DOUBLE      := "Double";
const VAR_ARRAY       := "Array";
const VAR_STRING      := "String";

//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------

function Malloc(who, arraySize := 1, forWhom := 2, power := 3)
	
  __ParamUnused(who);
  __ParamUnused(power);
  __ParamUnused(forWhom);
  __ParamUnused(arraySize);
	return 0;

endfunction

//-----------------------------------------------------------------------------

function IsVarType(byref v, Type)

  return (TypeOf(v) == Type);

endfunction

//-----------------------------------------------------------------------------

function CheckArray(Arr)

  foreach ElmArr in Arr
    if((!ElmArr and CStr(ElmArr != "0") and ElmArr != "") or ElmArr["error{"] or ElmArr["<uninit"])
      ElmArr := "";
    endif
  endforeach

  return Arr;

endfunction

//-----------------------------------------------------------------------------

function MakeIntArray(byref Arr)

  foreach Value in Arr
    Value := CInt(Value);
  endforeach

endfunction

//-----------------------------------------------------------------------------

function GetArraySum(byref Arr)

  var Sum := 0;

  foreach Val in Arr
    Sum += CInt(Val);
  endforeach

  return Sum;

endfunction

//-----------------------------------------------------------------------------

function RetIndex(byref Arr, byref Element)

  var i;

  for (i := 1; i <= Arr.size(); i := i + 1)
    if (Arr[i] == Element)
      return i;
    endif
  endfor

  return 0;

endfunction

//-----------------------------------------------------------------------------

function RetError(Msg)

  var Err := error;
  Err.errortext := Msg;
  return Err;

endfunction

//-----------------------------------------------------------------------------

function ToMin(Value, MinVal)

  if(Value < MinVal)
    return MinVal;
  else
    return Value;
  endif

endfunction

//-----------------------------------------------------------------------------

function ToMax(Value, MaxVal)

  if(Value > MaxVal)
    return MaxVal;
  else
    return Value;
  endif

endfunction

//-----------------------------------------------------------------------------

function MinMax(Value, MinVal, MaxVal)

  if(Value < MinVal)
    return MinVal;
  elseif(Value > MaxVal)
    return MaxVal;
  else
    return Value;
  endif

endfunction

//-----------------------------------------------------------------------------

function GetObjTypePrice(who, City, ObjType, isBuy := 0)

  var ItemCfg := ReadConfigFile(ITEMS_CFG);
  var cost := 0;
  var RLvl := GetPlayerNpcGuildCityLvl(who, City);

  if(!isBuy)
    cost := CInt(ItemCfg[ObjType].VendorBuysFor);
  else
    cost := CInt(ItemCfg[ObjType].VendorSellsFor);
  endif

  if(City)
    RLvl := RLvl * 3;
    if(!isBuy)
      cost := cost * ((100 + RLvl) * 0.01);
    endif
  endif

  return CInt(cost);

endfunction

//--------------------------------------------------------------------------------

function __ParamUnused(Param)

  if(Param)
    return 1;
  else
    return 0;
  endif

endfunction

//--------------------------------------------------------------------------------

function __IfElse(condiction,_if,_else)

  if(condiction)
    return _if;
  else
    return _else;
  endif

endfunction

//--------------------------------------------------------------------------------

function HaveChance(Perc)

  if(RandomInt(100) < Perc)
    return 1;
  endif

  return 0;

endfunction

//--------------------------------------------------------------------------------

function Modulo(Val, Amt)

  // IMPORTANT NOTE!!!
  // Modulo kurwa nie dziala! Przez glupiego pola stracilem godzine, bo duren nie wywala bledu na modulo, a zwraca zawsze ta sama liczbe.
  // dla pola 100%30 jest 100!

  return Val - (CInt(Val / Amt) * Amt);
  
endfunction

//--------------------------------------------------------------------------------

function PercentInterpolate(Amt, Perc)

  // jak zapomnisz o CDbl to utnie do calkowitych i bedzie kupa.
  return (CDbl(Amt) * CDbl(Perc) / 100.0);

endfunction

//--------------------------------------------------------------------------------

function IsDebug(who, Name)

  return (GetObjProperty(who, Name));

endfunction

//--------------------------------------------------------------------------------

function MergeArray(byref Arr, StartIndex := 1, Sep := " ")

  var i;
  var Text := "";

  for(i:=StartIndex; i<=Arr.size(); i+=1)

    if(Text)
      Text := Text + Sep + Arr[i];
    else
      Text := Arr[i];
    endif

  endfor

  return Text;

endfunction

//-----------------------------------------------------------------------------

function IsArray(byref obj)

  return (TypeOf(obj) == VAR_ARRAY);

endfunction

//-----------------------------------------------------------------------------

function IsUninit(byref obj)

  return (TypeOf(obj) == VAR_UNINIT);

endfunction

//-----------------------------------------------------------------------------

function IsStruct(byref obj)

  return (TypeOf(obj) == VAR_STRUCT);

endfunction

//-----------------------------------------------------------------------------

function IsDict(byref obj)

  return (TypeOf(obj) == VAR_DICTIONARY);

endfunction

//-----------------------------------------------------------------------------

function IsItemRef(byref obj)

  return (TypeOf(obj) == VAR_ITEMREF);

endfunction

//-----------------------------------------------------------------------------

function IsString(byref obj)

  return (TypeOf(obj) == VAR_STRING);

endfunction

//--------------------------------------------------------------------------------
