//-----------------------------------------------------------------------------

include "../pkg/magic/magicitems/magicitems";
include "include/key";
include "include/items";
include "include/logerror";
include "include/mobile/age";

//-----------------------------------------------------------------------------

const GOLDCOIN_YEAR_MULT  := 0.35;
const GOLDCOIN_NORMAL_MULT := 0.69;

//-----------------------------------------------------------------------------

var LootData;

//================================================================================
//--------------------------------------------------------------------------------
// Loots
//--------------------------------------------------------------------------------
//================================================================================

function CreateLoot(Cont, LootGroup, LootsCfgFile := LOOTS_CFG, MagicLvl := 0, QuantMult := 1.0, GoldMult := 1.0)

  LootData := struct;
  LootData.+LootsCfg;
  LootData.+GroupCfg;
  LootData.+CanBeMagic;
  LootData.+LootItem;
  LootData.+Count;
  LootData.+ItmGroup;
  LootData.+Rc;
  LootData.+ItemCfg;
  LootData.+QuantMult := QuantMult;
  LootData.+GoldMult := GoldMult;
  LootData.+Umy;
  LootData.+Cont := Cont;

  LootData.ItemCfg  := ReadConfigFile(ITEMS_CFG);
  LootData.LootsCfg := ReadConfigFile(LootsCfgFile);
  LootData.GroupCfg := ReadConfigFile(ITEMGROUPS_CFG);
  
  var npcTemp := GetObjProperty(Cont, PROP_NPCTEMP);

  if(npcTemp)
    LootData.+npcCfgElem := GetNpcConfig(npcTemp);
  endif

  if(!Cont.isa(POLCLASS_CONTAINER))
    return;
  endif
  
  if(Cont.isa(POLCLASS_CORPSE))
    if(LootData.npcCfgElem)
      LootData.CanBeMagic := SplitWords(GetConfigString(LootData.npcCfgElem, "MagicItem"));
      LootData.Umy := SplitWords(GetConfigString(LootData.npcCfgElem, "Umy"));
      if(IsSetFlag(Cont,NPCF_DOUBLE_LOOT))
        LootData.CanBeMagic[1] := Cint(LootData.CanBeMagic[1]) + 20;
      endif
    endif
  else
    if(!MagicLvl)
      LootData.CanBeMagic    := array;
      if(Cont.objtype == UOBJ_TREASURE_CHEST)
        MagicLvl      := CInt(GetTreasureLvl(Cont))+8; //poziom magola na mapie skarbu, dodajemy 8 bo mapa smoka ma poziom 5 + 8 = 13 i + 6 ze zwoju identa daje 19
        LootData.CanBeMagic[1] := 100;
        LootData.CanBeMagic[2] := MagicLvl;
        if(GetTreasureLvl(Cont) >= 4) //od 4 lvlu map w gore
          LootData.Umy  := array{100, 2, MagicLvl};
        endif
      elseif(Cont.objtype == UOBJ_PS_CONT or Cont.objtype == UOBJ_SCHEST)
        MagicLvl      := CInt(GetObjProperty(Cont, PROP_CHEST_LVL));
        LootData.CanBeMagic[1] := 100;
        LootData.CanBeMagic[2] := MagicLvl * 2;
      endif
    endif

  endif

  CreateAllGroups(Cont,LootGroup);
  LootData := 0;

endfunction

//-----------------------------------------------------------------------------

function CreateAllGroups(Cont,LootGroup)

  if(CInt(LootGroup))
    LootGroup := CInt(LootGroup);
  endif
  
  if(!LootData.LootsCfg[LootGroup])
    return;
  endif

  foreach ItmString in GetConfigStringArray(LootData.LootsCfg[LootGroup],"Stack")
    CreateItemFromStackString(Cont,SplitWords(ItmString));
  endforeach
  
  foreach ItmString in GetConfigStringArray(LootData.LootsCfg[LootGroup],"Item")
    CreateItemFromString(Cont,SplitWords(ItmString));
  endforeach

  foreach ItmString in GetConfigStringArray(LootData.LootsCfg[LootGroup],"Random")
    CreateItemFromRandomString(Cont,SplitWords(ItmString));
  endforeach

  foreach ItmString in GetConfigStringArray(LootData.LootsCfg[LootGroup],"CItem")
    CreateItemFromColorString(Cont,SplitWords(ItmString));
  endforeach

  if(LootData.Umy)
    MagicSkillItems(LootData);
  endif

endfunction

//-----------------------------------------------------------------------------

function CreateItemFromStackString(Cont,ItmString)

  if(!Cont or !ItmString)
    return;
  endif

  if(!CInt(ItmString[3]))
    ItmString[3] := 100;
  else
    ItmString[3] := CInt(ItmString[3]);
  endif
  
  if(!CInt(ItmString[2]))
    ItmString[4] := ItmString[2];
    ItmString[2] := GetObjTypeByName(ItmString[2]);
  endif

  if(!CInt(ItmString[2]))
    LogError("CreateItemFromStackString: bledny OT "+ItmString[4]);
    return;
  endif
  
  var Amount;
  
  if(RandomInt(100) < ItmString[3])

    Amount := RandomDiceRoll(ItmString[1]);

    if(LootData.npcCfgElem.OldAge)
      if(ItmString[2] == UOBJ_GOLDCOIN)
        var age := GetRealAge(Cont);
        if(age > NPC_START_AGE)
          var yearAmt := CInt(amount * GOLDCOIN_YEAR_MULT);
          age := age - NPC_START_AGE;
          amount := amount + age * yearAmt;
        endif

        amount := CInt(amount * GOLDCOIN_NORMAL_MULT);
        Amount := CInt(Amount * LootData.GoldMult);
      endif
    endif

    if(Cont.objtype != UOBJ_TREASURE_CHEST)
      Amount := CInt(Amount * LootData.QuantMult);
    endif
    
    if(Amount)
      LootData.LootItem := CreateItemInCont(Cont,ItmString[2], Amount);
      if(!LootData.LootItem)
        LogError("CreateItemFromStackString: nie udalo sie stworzyc OT "+ItmString[4]+" ["+LootData.LootItem.errortext+"] Cont: ["+Cont.serial+"]");
      endif
    endif
  endif
  sleepms(5);

endfunction

//-----------------------------------------------------------------------------

function CreateItemFromRandomString(Cont,ItmString)

  if(!Cont or !ItmString)
    return;
  endif

  var RItmGroup     := LootData.GroupCfg[ItmString[2]];
  if(RItmGroup)
  
    if(!CInt(ItmString[3]))
      ItmString[3] := 100;
    else
      ItmString[3] := CInt(ItmString[3]);
    endif
    
    var Amount := RandomDiceRoll(ItmString[1]);
    
    if(LootData.npcCfgElem.OldAge)
      var age := GetRealAge(Cont);
      if(age > NPC_START_AGE)
        if(ItmString[2] == "NormalEquipment")
          age := age - NPC_START_AGE;
          Amount := Amount + age;
        endif
      endif
    endif
    
    RItmGroup     := GetConfigStringArray(RItmGroup,"Item");
    if(!RItmGroup or !RItmGroup.size())
      LogError("CreateItemFromRandomString: Cfg - Grupa ["+ItmString[2]+"] nie istnieje.");
      return;
    endif

    for(LootData.Rc:=1;LootData.Rc<=Amount;LootData.Rc:=LootData.Rc+1)
      if(RandomInt(100) < ItmString[3])
        ItmString[4] := RItmGroup[RandomInt(len(RItmGroup))+1];
        if(!ItmString[4])
          ItmString[4] := RItmGroup[1];
        endif
        CreateItemFromString(Cont,SplitWords(ItmString[4]));
      endif
    endfor
    
  endif
  
endfunction

//-----------------------------------------------------------------------------

function CreateItemFromString(Cont,ItmString)

  if(!Cont or !ItmString)
    return;
  endif

  if(!CInt(ItmString[3]))
    ItmString[3] := 100;
  else
    ItmString[3] := CInt(ItmString[3]);
  endif
  
  LootData.ItmGroup := LootData.GroupCfg[ItmString[2]];
  if(LootData.ItmGroup)
    LootData.ItmGroup     := GetConfigStringArray(LootData.ItmGroup,"Item");
    if(!LootData.ItmGroup or !LootData.ItmGroup.size())
      LogError("CreateItemFromString: Cfg - Grupa ["+ItmString[2]+"] nie istnieje.");
      return;
    endif
    LootData.ItmGroup     := LootData.ItmGroup[RandomInt(len(LootData.ItmGroup))+1];
    if(!LootData.ItmGroup)
      LootData.ItmGroup := GetConfigString(LootData.GroupCfg[ItmString[2]],"Item");
    endif
    LootData.ItmGroup     := SplitWords(LootData.ItmGroup);
    LootData.ItmString[1] := LootData.ItmGroup[1];
    LootData.ItmString[2] := LootData.ItmGroup[2];
  endif

  if(!CInt(ItmString[2]))
    ItmString[4] := ItmString[2];
    ItmString[2] := GetObjTypeByName(ItmString[2]);
  endif

  if(!CInt(ItmString[2]))
    LogError("CreateItemFromString: bledny OT "+ItmString[4]);
    return;
  endif

  var Amount := RandomDiceRoll(ItmString[1]);
  
  if(LootData.npcCfgElem.OldAge)
    var age := GetRealAge(Cont);
    if(age > NPC_START_AGE)
      if(ItmString[2] in array(UOBJ_LVL1_MAP, UOBJ_LVL2_MAP, UOBJ_LVL3_MAP, UOBJ_LVL4_MAP, UOBJ_LVL5_MAP))
        age := age - NPC_START_AGE;
        Amount := Amount + CInt(age / 2);
      endif
    endif
  endif
  
  if(RandomInt(100) < ItmString[3])

    if(Amount == 1)
      if(CInt(LootData.QuantMult * 100) < RandomInt(100))
        Amount := 0;
      endif
    else
      Amount := CInt(Amount * LootData.QuantMult);
    endif

    if(Amount)

      if(IsStackableObj(ItmString[2]))
        LootData.LootItem := CreateItemInCont(Cont,ItmString[2], Amount);
        if(!LootData.LootItem)
          LogError("CreateItemFromString: nie udalo sie stworzyc OT "+ItmString[4]+" ["+LootData.LootItem.errortext+"] Cont: ["+Cont.serial+"]");
        endif
      else
        for(LootData.Count:=1;LootData.Count<= Amount;LootData.Count:=LootData.Count+1)
          if(!Cont)
            break;
          endif
          LootData.LootItem := CreateItemInCont(Cont,ItmString[2]);
          if(LootData.ItemCfg[LootData.LootItem.objtype].AddNewbie)
            LootData.LootItem.newbie := 1;
          endif
          if(LootData.ItemCfg[LootData.LootItem.objtype].SpecLoot)
            SetRandomSpecItem(LootData.LootItem, LootData.ItemCfg);
          endif
          if(!LootData.LootItem)
            LogError("CreateItemFromString: nie udalo sie stworzyc OT "+ItmString[4]+" ["+LootData.LootItem.errortext+"] Cont: ["+Cont.serial+"]");
            break;
          endif
          if(LootData.CanBeMagic)
            if(Cont.objtype == UOBJ_TREASURE_CHEST)
              TryToMakeItemMagic(LootData.LootItem, CInt(LootData.CanBeMagic[1]), Cint(LootData.CanBeMagic[2]) - RandomInt(2));
            else 
              TryToMakeItemMagic(LootData.LootItem, CInt(LootData.CanBeMagic[1]), Cint(LootData.CanBeMagic[2]) - RandomInt(3));
            endif
          endif
        endfor
      endif

    endif

  endif

  sleepms(5);

endfunction

//-----------------------------------------------------------------------------

function CreateItemFromColorString(Cont,ItmString)

  if(!Cont or !ItmString)
    return;
  endif

  if(!CInt(ItmString[2]))
    ItmString[4] := ItmString[2];
    ItmString[2] := GetObjTypeByName(ItmString[2]);
  endif

  if(!CInt(ItmString[2]))
    LogError("CreateItemFromColorString: bledny OT "+ItmString[4]);
    return;
  endif

  var ColorsCfg := ReadConfigFile(COLORS_CFG);

  var Amount   := RandomDiceRoll(ItmString[1]);
  if(CInt(ItmString[3]))
    LootData.Rc := CInt(ItmString[3]);
  else
    LootData.Rc := GetConfigStringArray(ColorsCfg[ItmString[3]],"color");
    LootData.Rc := CInt(LootData.Rc[RandomInt(len(LootData.Rc))+1]);
  endif

  Amount := CInt(Amount * LootData.QuantMult);
  
  for(LootData.Count:=1;LootData.Count<=Amount;LootData.Count:=LootData.Count+1)
    if(!Cont)
      break;
    endif
    LootData.LootItem := CreateItemInCont(Cont,ItmString[2]);
    if(!LootData.LootItem)
      LogError("CreateItemFromColorString: nie udalo sie stworzyc OT "+ItmString[4]+" ["+LootData.LootItem.errortext+"] Cont: ["+Cont.serial+"]");
      break;
    endif
    if(LootData.Rc)
      LootData.LootItem.color := LootData.Rc;
    endif
  endfor

endfunction

//-----------------------------------------------------------------------------

function SetRandomSpecItem(Item, ItemsCfg)

  case(Item.objtype)
    0xf139:
    0xf13A:
    0xf13B:
    0xf13C:
      SetTimeToDestroy(Item, CInt(ItemsCfg[Item.objtype].DecayTime));
      var Time := GetTimeToDestroy(item) - PolCore().systime;
      SetToolTipProp(Item, ITEM_DECAY_TIME_DESC, RealTime(Time));
  endcase

endfunction

//-----------------------------------------------------------------------------

function MagicSkillItems(LootData)
//umy szansa ile_umow poziom_uma
//Umy  100      2        20

  var itemsArr := GetConfigStringArray( LootData.GroupCfg["ItemyDoUmow"], "Item" );

  for i:=1 to CInt(LootData.Umy[2])
    if(RandomInt(100) < CInt(LootData.Umy[1]))
      var ItemName := SplitWords(itemsArr.RandomEntry())[2];
      LootData.LootItem := CreateItemInCont(LootData.Cont, ItemName, 1);
      MakeMagicMods(LootData, CInt(LootData.Umy[3]));
    endif
  endfor

endfunction

//-----------------------------------------------------------------------------

function MakeMagicMods(LootData, MagicLvl)

  var item := LootData.LootItem;
  var PrefixMods := array;
  var SuffixMods := array;

  if(item.isa(POLCLASS_WEAPON))
    PrefixMods.append(MAGICMOD_FIREDMG);
    PrefixMods.append(MAGICMOD_COLDDMG);
    PrefixMods.append(MAGICMOD_LIGHTDMG);
    PrefixMods.append(MAGICMOD_POISONDMG);
  else
    PrefixMods.append(MAGICMOD_LIFE);
    PrefixMods.append(MAGICMOD_MANA);
    PrefixMods.append(MAGICMOD_STAMINA);
  endif

  if(item.isa(POLCLASS_WEAPON))
    SuffixMods := GetWeaponSuffix();
  else
    SuffixMods := GetCustomSkilMod();
  endif

  var MagicPrefixMod := PrefixMods.RandomEntry();
  var MagicSuffixMod := SuffixMods.RandomEntry();

  if(item.isa(POLCLASS_WEAPON))
    var SkillID := GetSkillIdByAttributeId(LootData.ItemCfg[item.objtype].Attribute);
    if(SkillID != MagicSuffixMod)
      MagicSuffixMod := SkillID;
    endif
  endif

  var MagicItemArray := array{};
  MagicItemArray.append(MagicPrefixMod);
  MagicItemArray.append(MAGICMOD_SKILLMOD + MagicSuffixMod);
  MagicItemArray.append(MagicLvl);

  SetMagicMods(item,MagicItemArray);

  AddMagicName(item);

endfunction

//-----------------------------------------------------------------------------

function GetCustomSkilMod()

  return array{
    SKILLID_ANATOMY,
    SKILLID_PARRY,
    SKILLID_INSIGHT,
    SKILLID_ENTICEMENT,
    SKILLID_EVALINT,
    SKILLID_HEALING,
    SKILLID_DRUID,
    SKILLID_PROVOCATION,
    SKILLID_MAGERY,
    SKILLID_MAGICRESISTANCE,
    SKILLID_TACTICS,
    SKILLID_ARCHERY,
    SKILLID_SPIRITSPEAK,
    SKILLID_SWORDSMANSHIP,
    SKILLID_MACEFIGHTING,
    SKILLID_FENCING,
    SKILLID_WRESTLING,
    SKILLID_MEDITATION,
    SKILLID_WEARWOLF,
    SKILLID_NECRO,
    SKILLID_VAMPIRE,
    SKILLID_BARBARIAN,
    SKILLID_AXESMANSHIP,
    SKILLID_HUNTER,
    SKILLID_ZEAL,
    SKILLID_PALADINISM,
    SKILLID_KENSAI
  };

endfunction

//-----------------------------------------------------------------------------

function GetWeaponSuffix()

  return array{
    SKILLID_ANATOMY,
    SKILLID_SWORDSMANSHIP,
    SKILLID_MACEFIGHTING,
    SKILLID_FENCING,
    SKILLID_WRESTLING,
    SKILLID_AXESMANSHIP
  };

endfunction