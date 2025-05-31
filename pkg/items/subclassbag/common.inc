function GetSubClassReagentsList(who, bag)

  var MultipleCfgArr := 0;

  case(GetCharClass(who))
    CLASS_WARRIOR:
    CLASS_ARCHER:
      return GetWarcastReagentsArr(bag);

    CLASS_MAGE:
      MultipleCfgArr := GetConfigStringArray( ReadConfigFile(ITEMS_CFG)[bag.objtype], "ReagentsCfgAlt" ); //mag
      if(IsWearwolf(who))
        return RemoveDuplicateRegs( GetWarcastReagentsArr(bag) + GetReagentArrFromMultipleCfgs(MultipleCfgArr) ); //obie funkcje zwracaja tablice
      else
        return GetReagentArrFromMultipleCfgs(MultipleCfgArr);
      endif
  endcase

endfunction

//-----------------------------------------------------------------------------

function GetWarcastReagentsArr(bag)

  var RegsDict := dictionary;
  var RegKeysArr := array;
  var cfg := ReadConfigFile(ITEMS_CFG)[bag.objtype].ReagentsCfg; //wojownik
  var subclasscfg := ReadConfigFile(cfg);

  foreach idx in GetConfigIntKeys( subclasscfg )
    RegsDict := GetConfigStringDictionary( subclasscfg[idx], "Reagent" );
    RegKeysArr := RegKeysArr + RegsDict.keys();
  endforeach

  return RemoveDuplicateRegs(RegKeysArr);

endfunction

//-----------------------------------------------------------------------------

function GetReagentArrFromMultipleCfgs(byref MultipleCfgArr)

  var ReagentsArr := array;

  foreach CfgPath in MultipleCfgArr
    var RegsDict := dictionary;
    var RegKeysArr := array;
    var subclasscfg := ReadConfigFile(CfgPath);

    foreach idx in GetConfigIntKeys( subclasscfg )
      RegsDict := GetConfigStringDictionary( subclasscfg[idx], "SReagent" );
      RegKeysArr := RegKeysArr + RegsDict.keys();
    endforeach

    ReagentsArr += RemoveDuplicateRegs(RegKeysArr);
  endforeach

  return ReagentsArr;

endfunction

//-----------------------------------------------------------------------------

function RemoveDuplicateRegs(byref IncomingArr)

  var OutputArr := array;

  foreach reg in IncomingArr
    if(!CInt(reg) and reg)
      reg := GetObjTypeByName(reg);
    endif
    if(!(CInt(reg) in OutputArr))
      OutputArr.append(CInt(reg));
    endif
  endforeach

  return OutputArr;

endfunction
