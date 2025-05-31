//--------------------------------------------------------------------------------

include ":npcguilds:common";

//--------------------------------------------------------------------------------

const MAX_SKILLS_COUNT       := 20;

const MSKT_CLASS             := 3;
const MSKT_NPCGUILD          := 1;

const MSK_CLASS_COUNT        := 3;
const MSK_NPCGUILD_COUNT     := 2; //max ilosc skili z gildii

const MSK_DELAYED_INTERVAL   := 43200; //co 12 h obniza skill z gildii z ktorej gracz zrezygnowal
const MSK_DELAYED_AMOUNT     := 5;

//================================================================================
//--------------------------------------------------------------------------------
// Max Skill Values
//--------------------------------------------------------------------------------
//================================================================================

function GetSkillBaseMaxValue(who, SkID)

  if(IsBlockedSkillForClass(who, SkID))
    return 0;
  endif

  var AttrId := GetAttributeIdBySkillId(SkID);
  var SkVal := GetObjProperty(who, PROP_MAXSK + "_" + AttrId);

  if(SkVal)
    SkVal := CInt(SkVal);

    if(SkVal < 0 or SkVal > MAX_SK_MAX_VALUE)
      SkVal := MinMax(SkVal, 0, MAX_SK_MAX_VALUE);
      SetObjProperty(who, PROP_MAXSK + "_" + AttrId, SkVal);
    endif

    return SkVal;
  else
    return DEF_SK_MAX_VALUE;
  endif

endfunction

//--------------------------------------------------------------------------------

function GetSkillMaxValue(who, SkID)

  return CDbl(GetSkillBaseMaxValue(who, SkID)) / 10.0;

endfunction

//--------------------------------------------------------------------------------

function GetMaxSkills(who)

  var Data := GetObjProperty(who, PROP_MAXSK);
  
  if(!Data or !IsVarType(Data, VAR_DICTIONARY))
    return dictionary;
  endif

  return Data;

endfunction

//--------------------------------------------------------------------------------
//usuwa wybrany skill z gildii
function EraseNpcGuildSkill(who, byref MaxSk, SkID, GuildId := 0)

  var AttrId := GetAttributeIdBySkillId(SkID);

  if(MaxSk.exists(AttrId))
    if(MaxSk[AttrId][1] == MSKT_NPCGUILD)
      var PlayerData := GetObjProperty(who, PROP_NGDDATA);

      if(GuildId and PlayerData.MaxSkIDdict[GuildId] == SkID)
        PlayerData.MaxSkIDdict.erase(GuildId); //nowy sposob zapisu wybranego skila
        PlayerData.SkChTimedict.erase(GuildId);
      endif
      
      if(PlayerData.MaxSkID == SkID)
        PlayerData.MaxSkID := error; //stary sposob zapisu wybranego skila
        PlayerData.SkChTime := 0;
        //foreach Guild in (PlayerData.Guilds)
        //  __ParamUnused(Guild);
        //  SetBitOff(PlayerData.Guilds[_Guild_iter].Flags, NGPF_MAXSK_CHOOSEN); //uzyc tego tylko gdy z danej gildii gracz rezygnuje ze skila
        //endforeach

        SetObjProperty(who, PROP_NGDDATA, PlayerData);
      endif
    endif
  endif

endfunction

//--------------------------------------------------------------------------------

function CheckDelayedMaxSkill(who)

  var MaxSk := GetMaxSkills(who);

  foreach Data in MaxSk
    if(Data.size() == 3)
      if(Data[3] + MSK_DELAYED_INTERVAL < ReadGameClock())

        var SkId := GetSkillIdByAttributeId(_Data_iter);
        var whoSk := GetBaseSkillBaseValue(who, SkID);
        var CurSkMax := CInt(GetObjProperty(who, PROP_MAXSK+"_"+_Data_iter));
        var SkElem := GetSkillConfigElem(SkId);
        CurSkMax := Max(CurSkMax - MSK_DELAYED_AMOUNT, DEF_SK_MAX_VALUE);

        if(whoSk > CurSkMax)
          SetBaseSkillBaseValue(who, SkID, CurSkMax);
        endif

        if(CurSkMax == DEF_SK_MAX_VALUE)
          MaxSk.erase(_Data_iter);
          SetObjProperty(who, PROP_MAXSK, MaxSk);
          EraseObjProperty(who, PROP_MAXSK+"_"+_Data_iter);
          return;
        endif
        
        Data[3] := ReadGameClock() + MSK_DELAYED_INTERVAL;
        SetObjProperty(who, PROP_MAXSK, MaxSk);
        SetObjProperty(who, PROP_MAXSK+"_"+_Data_iter, CurSkMax);
        SendSysMessage(who, "Twoja udoskonalona umiejetnosc "+SkElem.Name+" zanika...", FONT_NORMAL, COLOR_ORANGE);
        SetObjProperty(who, "#LastMaxSkillDelay", ReadGameClock() + 1800);
      else
        var Delay := CInt(GetObjProperty(who, "#LastMaxSkillDelay"));

        if(!Delay or Delay < ReadGameClock())
        
          var SkId := GetSkillIdByAttributeId(_Data_iter);
          var SkElem := GetSkillConfigElem(SkId);

          SendSysMessage(who, "Twoja udoskonalona umiejetnosc "+SkElem.Name+" niedlugo zaniknie...", FONT_NORMAL, COLOR_RED_LIGHT);
          Delay := ReadGameClock() + 900;
          SetObjProperty(who, "#LastMaxSkillDelay", Delay);
        endif
      endif
    endif
  endforeach
  
endfunction

//--------------------------------------------------------------------------------

function EraseMaxSkillDelayed(who, SkID, GuildId := 0)

  SkID  := CInt(SkID);

  var MaxSk := GetMaxSkills(who);
  var AttrId := GetAttributeIdBySkillId(SkID);

  if(!MaxSk.exists(AttrId))
    return error;
  endif

  if(!GuildId)
    GuildId := GetNpcGuildIdFromSkId(who, SkID);
  endif
  
  EraseNpcGuildSkill(who, MaxSk, SkID, GuildId);

  if(MaxSk[AttrId].size() < 3)
    MaxSk[AttrId][3] := ReadGameClock();
    SetObjProperty(who, PROP_MAXSK, MaxSk);
  endif

  return 1;

endfunction

//--------------------------------------------------------------------------------

function GetNpcGuildIdFromSkId(who, SkID)

  var PlayerData := GetObjProperty(who, PROP_NGDDATA);
  var Keys := PlayerData.MaxSkIDdict.Keys();

  foreach GuildId in Keys
    if(PlayerData.MaxSkIDdict[GuildId] == SkID)
      return GuildId;
    endif
  endforeach

  return 0;

endfunction

//--------------------------------------------------------------------------------
//resetuje max wartosc skila do bazowej 100
function EraseMaxSkill(who, SkID, GuildId := 0)

  SkID  := CInt(SkID);

  var MaxSk := GetMaxSkills(who);
  var AttrId := GetAttributeIdBySkillId(SkID);

  if(!MaxSk.exists(AttrId))
    return error;
  endif

  EraseNpcGuildSkill(who, MaxSk, SkID, GuildId);

  var whoSk := GetBaseSkillBaseValue(who, SkID);

  if(whoSk > DEF_SK_MAX_VALUE)
    SetBaseSkillBaseValue(who, SkID, DEF_SK_MAX_VALUE);
  endif

  MaxSk.erase(AttrId);
  EraseObjProperty(who, PROP_MAXSK+"_"+AttrId);
  SetObjProperty(who, PROP_MAXSK, MaxSk);

  SendAttrRefresh(who);

  return 1;

endfunction

//--------------------------------------------------------------------------------

function GetMaxSkillsForType(who, Type)

  var MaxSk := GetMaxSkills(who);
  var Skills := array;

  foreach SkData in MaxSk
    if(Type == SkData[1])
      Skills.append(GetSkillIdByAttributeId(_SkData_iter));
    endif
  endforeach

  return Skills;

endfunction

//--------------------------------------------------------------------------------

function GetMaxSkillTypeName(Type)

  case(Type)
    MSKT_CLASS:     
      return "Klasa";
    MSKT_NPCGUILD:
      return "Gildia";
  endcase

  return 0;

endfunction

//--------------------------------------------------------------------------------

function GetMaxSkillMaxCountForType(Type)

  case(Type)
    MSKT_CLASS:     
      return MSK_CLASS_COUNT;
    MSKT_NPCGUILD:
      return MSK_NPCGUILD_COUNT;
  endcase

  return 0;

endfunction

//--------------------------------------------------------------------------------

function CanAddMaxSkill(who, Type)

  var CurType  := GetMaxSkillsForType(who, Type);
  var MaxCount := GetMaxSkillMaxCountForType(Type);

  return (MaxCount and MaxCount > CurType.size());

endfunction

//--------------------------------------------------------------------------------

function RecheckGuildSkill(who, SkID, Type, GuildId := 0)

  if(Type == MSKT_NPCGUILD)
    var PlayerData := GetObjProperty(who, PROP_NGDDATA);
    var MaxSkillsArr := GetMaxSkillsForType(who, MSKT_NPCGUILD); //tablica wybranych skili z gildii
    if(GuildId)
      if(!IsDict(PlayerData.MaxSkIDdict))
        PlayerData.+MaxSkIDdict := dictionary;
      endif
      if(!IsDict(PlayerData.SkChTimedict))
        PlayerData.+SkChTimedict := dictionary;
      endif
      PlayerData.MaxSkIDdict[GuildId] := SkID;
      PlayerData.SkChTimedict[GuildId] := ReadGameClock();
    endif
    PlayerData.MaxSkID  := SkID;
    PlayerData.SkChTime := ReadGameClock();
    SetObjProperty(who, PROP_NGDDATA, PlayerData);
  endif

endfunction

//--------------------------------------------------------------------------------

function SetMaxSkillValue(who, SkID, SkVal, Type, GuildId := 0, SkillTime := 0)

  SkID  := CInt(SkID);
  SkVal := CInt(SkVal);

  var AttrId := GetAttributeIdBySkillId(SkID); //zwraca np. "Macefighting" dla SkID
  var MaxSk := GetMaxSkills(who); //np. dict{ "Macefighting" -> { 1, 34766198 }, "Swordsmanship" -> { 1, 34766163 } }
  var whoSk := GetBaseSkillBaseValue(who, SkID);
  var PlayerData := GetObjProperty(who, PROP_NGDDATA); //np: struct{ Guilds = dict{ 2 -> struct{ Delay = 0, Exp = 1000000, Flags = 1, JoinTime = 34599352, LastQuest = struct{ From = 0, QId = 2, QuestGroup = "quests", Ticket = 0, Time = 0 }, LastRepLose = 0, LastVisit = 34599387, Level = 1 } }, MaxSkID = error{ errortext = "Unable to unpack struct elemcount. Length given must be positive integer!" }, SkChTime = 0 }

  if(Type == MSKT_NPCGUILD)
    var GuildMax := 0;
    if(GuildId)
      if(!IsDict(PlayerData.MaxSkIDdict))
        PlayerData.+MaxSkIDdict := dictionary;
      endif
    endif

    foreach GdData in (PlayerData.Guilds)
      if(_GdData_iter == GuildId and GdData.Flags & NGPF_MAXSK_CHOOSEN)
        GuildMax := 1; //jesli NGPF_MAXSK_CHOOSEN jest ustawione w tej gildii to znaczy ze mozna zmienic graczowi skill na wybrany w tej gildii
        break;
      endif
    endforeach
    
    if(!GuildMax)
      return error; // gracz musi sam wybrac najpierw jakis skill w gildii, aby byla gildia przypisana.
    endif
  endif

  if(MaxSk.exists(AttrId) or PlayerData.MaxSkIDdict[GuildId] == SkID) //gracz wybral ten sam skill lub zwiekszenie max skila przy awansie w gildii
    if(GetSkillBaseMaxValue(who, SkID) == SkVal) // and MaxSk[AttrId][1] == Type and MaxSk[AttrId].size() == 2
      RecheckGuildSkill(who, SkID, Type, GuildId); //ustawia wybrany skill w gildii zapisujac do slownika PlayerData.MaxSkIDdict[GuildId] := nr skila
      return 1;
    endif
  endif

  if(whoSk > SkVal) //ma zalozonego magola do tego skila
    SetBaseSkillBaseValue(who, SkID, SkVal);
  endif

  var CurType  := GetMaxSkillsForType(who, Type);  //skile wybrane z gildii przez gracza
  var MaxCount := GetMaxSkillMaxCountForType(Type); //max ilosc skili jakie gracz moze umiec w tym przypadku Type = pobiera skile gildyjne
  if(!MaxCount)
    return 0;
  endif

  if(PlayerData.MaxSkIDdict[GuildId] and GetBaseSkillBaseValue(who, SkID) <= 1000) //wybiera skill ponownie w tej gildii, jesli nowo wybrany skill jest mniejszy od 100 to resetuj poprzedni
    EraseMaxSkill(who, PlayerData.MaxSkIDdict[GuildId], GuildId); //ustawia max wartosc wybranego skila na bazowa czyli 100
    MaxSk := GetMaxSkills(who);
    EraseNpcGuildSkill(who, MaxSk, SkID, GuildId); //usuwa wybrany skill kasujac wpis w slowniku PlayerData.MaxSkIDdict[GuildId]
  endif

  RecheckGuildSkill(who, SkID, Type, GuildId); //ustawia wybrany skill w gildii zapisujac do slownika PlayerData.MaxSkIDdict[GuildId] := nr skila

  if(SkillTime)
    MaxSk[AttrId] := array(Type, SkillTime);
  else
    MaxSk[AttrId] := array(Type, ReadGameClock());
  endif

  SetObjProperty(who, PROP_MAXSK, MaxSk); //np: dict{ "Fencing" -> { 1, 34599390 } } 1 - to typ w tym przypadku gildia, a moze te¿ byc udoskonalenie skila = typ 3 chyba
  SetObjProperty(who, PROP_MAXSK+"_"+AttrId, SkVal);

  SendAttrRefresh(who);
  return 1;

endfunction

//--------------------------------------------------------------------------------
