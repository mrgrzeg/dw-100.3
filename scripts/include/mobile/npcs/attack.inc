//-----------------------------------------------------------------------------

function SetDelayedAttack(Npc, Time)

  var Res := CInt(GetObjProperty(Npc, "#NoAttack"));
  if(Res == 1 or Res == -1)
    return 0;
  endif

  SetObjProperty(Npc, "#NoAttack", ReadGameClock()+Time);
  return 1;

endfunction

//-----------------------------------------------------------------------------

function SetInvulToAttack(Npc)

  SetObjProperty(Npc, "#NoAttack", 1);

endfunction

//-----------------------------------------------------------------------------

function EraseDelayedAttack(Npc)

  EraseObjProperty(Npc, "#NoAttack");

endfunction

//-----------------------------------------------------------------------------

function EraseInvulToAttack(Npc)

  EraseObjProperty(Npc, "#NoAttack");

endfunction

//-----------------------------------------------------------------------------

function IsInvulToAttack(Npc)

  if(!Npc.isa(POLCLASS_NPC))
    return 0;
  endif
  
  var Delayed := CInt(GetObjProperty(Npc, "#NoAttack"));

  if(Delayed == 1)
    if(HaveChance(20))
      PrintTextAbove(Npc, "*pssss*");
    endif
    PlayPcEffect(Npc, FX_HEAL_EFFECT, 1322, 24, 24, 3);
    PlaySoundEffect(Npc, SFX_DMG_ABSORBED);
    return 1;
  elseif(Delayed == -1)
    PlayPcEffect(Npc, FX_MARK_EFFECT, 999, 24, 24, 3);
    PlaySoundEffect(Npc, SFX_DMG_ABSORBED);
    return 1;
  elseif(Delayed)
    if(Delayed > ReadGameClock())
      if(HaveChance(10))
        PrintTextAbove(Npc, "*pssss*");
      endif
      PlayPcEffect(Npc, FX_SPARK_EFFECT, 1244, 24, 24, 11);
      PlaySoundEffect(Npc, SFX_DMG_ABSORBED);
      return 1;
    else
      EraseObjProperty(Npc, "#NoAttack");
    endif
  endif
  
  return 0;

endfunction

//-----------------------------------------------------------------------------

const LSDMF_ONLY_PLAYERS := 0x01;

//-----------------------------------------------------------------------------

function ListSpecialDamageMobs(me, Range, Flags := 0, SpecName := 0)

  var Objects;
  var Mobs := array;
  var SpecArea := NpcInSpecArea(me, SpecName);

  if(!IsArray(SpecArea))
    Objects := ListMobilesNearLocationEx( me.x, me.y, LIST_IGNORE_Z, Range, LISTEX_FLAG_HIDDEN | LISTEX_FLAG_NORMAL, me.realm);
  else
    Objects := ListMobilesInBox( SpecArea[1], SpecArea[2], -127, SpecArea[3], SpecArea[4], 127, me.realm);
  endif

  foreach Obj in Objects
    if(IsPlayer(Obj) and !Obj.dead)
      Mobs.append(Obj);
    endif

    if(Flags & LSDMF_ONLY_PLAYERS)
      continue;  
    endif

    if(IsTamed(Obj))
      var Master := GetMaster(Obj);
      if(IsPlayer(Master) and !Master.dead and CanBeDamagedBySpec(Obj, SpecName))
        Mobs.append(Obj);
      endif
    endif
  endforeach

  return Mobs;

endfunction

//-----------------------------------------------------------------------------

function NpcInSpecArea(mob, SpecName)

  var SpecAreaCfg := ReadConfigFile(":specattack:specarea");

  foreach idx in GetConfigStringKeys(SpecAreaCfg)
    var SpecArea  := SplitWords(SpecAreaCfg[idx].Area);
    var Templates := SplitWords(SpecAreaCfg[idx].Npc);
    var Realm := SpecArea[5];
    MakeIntArray(SpecArea);
    SpecArea[5] := Realm;
    if(mob.x >= SpecArea[1] and mob.y >= SpecArea[2] and mob.x <= SpecArea[3] and mob.y <= SpecArea[4] and mob.realm == Realm)
      if(mob.npctemplate in Templates and SpecName == SpecAreaCfg[idx].SpecName)
        return SpecArea;
      endif
    endif
  endforeach

  return 0;

endfunction

//-----------------------------------------------------------------------------

function CanBeDamagedBySpec(mob, SpecName)

  if(mob.npctemplate == "bazyliszek")
    case(SpecName)
      "StoneFall":
      "PoisonousCloud":
      "AbyssForce":
      "conflagration":
        return 0;
    endcase
  endif

  return 1;

endfunction

//-----------------------------------------------------------------------------

function GetSpecAreaIdx(mob)

  var SpecAreaCfg := ReadConfigFile(":specattack:specarea");

  foreach idx in GetConfigStringKeys(SpecAreaCfg)
    var SpecArea := SplitWords(SpecAreaCfg[idx].Area);
    var Realm := SpecArea[5];
    MakeIntArray(SpecArea);
    if(mob.x >= SpecArea[1] and mob.y >= SpecArea[2] and mob.x <= SpecArea[3] and mob.y <= SpecArea[4] and mob.realm == Realm)
      return idx;
    endif
  endforeach

  return 0;

endfunction

//-----------------------------------------------------------------------------

function GetSpecAreaRandomLocs(SpecArea, z)

  var Loc := LocStruct();
  Loc.x := RandomIntMinMax(SpecArea[1], SpecArea[3]);
  Loc.y := RandomIntMinMax(SpecArea[2], SpecArea[4]);
  Loc.z := z;
  Loc.realm := SpecArea[5];

  return Loc;

endfunction

//-----------------------------------------------------------------------------

function IsInSpecArea(whoAreaID, mobAreaID)

  return (whoAreaID and mobAreaID and whoAreaID == mobAreaID);

endfunction

//-----------------------------------------------------------------------------

function NpcAttackRandomMobile(Npc)

  var Mobs := ListSpecialDamageMobs(Npc, 15);

  if(Mobs.size())
    var Mob := Mobs[RandomInt(Mobs.size())+1];
    SendChEvent(Npc, Mob, EVID_DAMAGED);
    return 1;
  endif

  return 0;

endfunction

//-----------------------------------------------------------------------------

function GetNpcAggroTable(Npc)

  return GetObjProperty(Npc, "AggroTable");

endfunction

//-----------------------------------------------------------------------------

function GetAggroLevelForPlayer(Npc, Player, TimeExpires := 0)

  var Table := GetNpcAggroTable(Npc);

  if(Table)
    if(Table.exists(Player.serial))
      if(TimeExpires)
        if(ReadGameClock() - Table[1] > TimeExpires)
          return 0;
        endif
      endif

      return Table[Player.serial][2];
    endif
  endif

  return error;

endfunction

//-----------------------------------------------------------------------------

