//-----------------------------------------------------------------------------

use guilds;

//-----------------------------------------------------------------------------

include "include/proc";
include "include/finding";
include "include/logerror";
include "include/const/props";
include "include/misc";
include "include/checks/stdchecks";
include "include/mobile/titles";

//-----------------------------------------------------------------------------

const MAX_GUILD_NAME_SIZE                 := 30;
const MAX_GUILD_ABREV_SIZE                := 4;
const MAX_GM_TITLE_SIZE                   := 30;
const MAX_GUILD_TITLE_SIZE                := 30;
const MIN_GUILD_MEMBERS                   := 2;
const GD_MEMB_CHECK_DELAY                 := 86400;
const GD_MEMBLEFT_CHECK_DELAY             := 432000;
const GD_MEMBLEFT_NOTIFY_DELAY            := 21600;

const UOBJ_PACKED_GUIDSTONE_GRAPHIC       := 0x1172;
const UOBJ_GUILDSTONE_GRAPHIC             := 0x0ED4;

const GUILD_TIME                          := 1814400;
const GUILD_MOVE_TIME                     := 259200;

const CHAOS_ST                            := "Ksiestwo";
const ORDER_ST                            := "Order";
const NEUTRAL_ST                          := "Neutralna";

const MEMBER_TIME                         := 432000;
const RECRUIT_TIME                        := 172800;

const GUILD_DECDELAY                      := 43200;
const GUILD_ONE_DECDELAY                  := 345600;
const GUILD_DECEXPIRE                     := 172800;
const GUILD_WAR_DELAY                     := 604800; // 7 dni delaya jak odmowil wojny.

const DECWAR_GOLD                         := 1500;

//-----------------------------------------------------------------------------

const GUILDS_CFG                          := ":guildstone:guilds";

//-----------------------------------------------------------------------------

function GetGuildCreateTime(Guild)

  if(!Guild.GetProp("CreateTime"))
    Guild.SetProp("CreateTime",ReadGameClock());
  endif
  return Guild.GetProp("CreateTime");
  
endfunction

//-----------------------------------------------------------------------------

function GetGuildId(who)

  var GuildIdArr := GetObjProperty(who, PROP_GUILDID);
  if(IsPlayer(who))
    if(!IsArray(GuildIdArr))
      var Gid := CInt(GuildIdArr);
      GuildIdArr := array;
      if(Gid)
        GuildIdArr.append(Gid);
      endif
      SetObjProperty(who,PROP_GUILDID,GuildIdArr);
    endif
  endif

  return GuildIdArr;

endfunction
 
//-----------------------------------------------------------------------------

function SetGuildId(who,id)

  if(IsPlayer(who))
    var GuildIdArr := GetObjProperty(who, PROP_GUILDID);
    if(!IsArray(GuildIdArr))
      var Gid := CInt(GuildIdArr);
      GuildIdArr := array;
      if(Gid)
        GuildIdArr.append(Gid);
      endif
    endif

    GuildIdArr.append(CInt(id));
    return SetObjProperty(who,PROP_GUILDID,GuildIdArr);
  else
    return SetObjProperty(who,PROP_GUILDID,id);
  endif

endfunction

//-----------------------------------------------------------------------------

function GetGuildName(guild)

  return ClStr(guild.GetProp("guildname"));

endfunction

//-----------------------------------------------------------------------------

function SetGuildName(guild,name)

  return guild.SetProp("guildname",CStr(name));

endfunction

//-----------------------------------------------------------------------------

function GetGuildStatus(Guild)

  if(!guild.GetProp("GuildStatus"))

    var Rec     := GetGuildRecruits(Guild);
    var Members := {};
    var Murd    := 0;
    var Status  := ORDER_ST;
    var Gm      := FindGuildGm(Guild);
    var Stone   := GetStoneFromGuild(Guild);

    foreach Memb in Rec
      Members.append(Memb);
    endforeach

    foreach Memb in (Guild.members)
      Members.append(Memb);
    endforeach

    foreach Memb in Members
      if(Memb.murderer)
        Murd := Murd + 1;
      endif
    endforeach

    if(GetObjProperty(Stone,"Ksiestwo"))
      Status := CHAOS_ST; //czyli ksiestwo
    elseif(Murd > Members.size())
      Status := NEUTRAL_ST;
    else
      Status := ORDER_ST;
    endif

    if(Status != CHAOS_ST)
      foreach Memb in Members
        if(Memb.murderer)
          DismissGuildMember(Guild,Memb,1);
        endif
      endforeach
    endif
    
    SetGuildStatus(Guild,Status);
  endif

  return guild.GetProp("GuildStatus");

endfunction

//-----------------------------------------------------------------------------

function SetGuildStatus(Guild,St)

  if(St == CHAOS_ST or St == ORDER_ST or St == NEUTRAL_ST)
    guild.SetProp("GuildStatus",St);
    return 1;
  endif
  return 0;

endfunction

//-----------------------------------------------------------------------------

function GetMemberStatus(who)

  var Guild := FindGuild(GetNonChaosGuildID(who));
  if(Guild)
    return GetGuildStatus(Guild);
  else
    return error;
  endif

endfunction

//-----------------------------------------------------------------------------

function GetGuildAbv(guild)

  if(IsPlayer(guild))
    return CInt(GetObjProperty(Guild,"abv"));
  endif

  if(guild.GetProp("guildabv") == "NONE" or !guild.GetProp("guildabv"))
    var Name := SplitWords(GetGuildName(Guild));
    var Abv  := "";
    foreach Word in Name
      Abv := Abv + Upper(Word[1]);
    endforeach
    SetGuildAbv(Guild,Abv);
  endif
  return ClStr(guild.GetProp("guildabv"));

endfunction

//-----------------------------------------------------------------------------

function SetGuildAbv(guild,abv)

  if(IsPlayer(Guild))
    return SetObjProperty(Guild,"abv",CInt(abv));
  endif

  return guild.SetProp("guildabv",CStr(abv));

endfunction

//-----------------------------------------------------------------------------

function GetGuildIcon(guild)

  if(!CInt(guild.GetProp("guildicon")))
    SetGuildIcon(Guild,5545);
  endif
  return CInt(guild.GetProp("guildicon"));

endfunction

//-----------------------------------------------------------------------------

function SetGuildIcon(guild,icon)

  return guild.SetProp("guildicon",CInt(icon));

endfunction

//-----------------------------------------------------------------------------

function GetGuildRecruits(guild)

  var Recs := Guild.GetProp("recruits");
  if(!Recs.size() or Recs[1] == "NONE" or !Recs)
    Recs := {};
  endif
  
  foreach Rec in Recs
    if(!FindPlayerBySerial(Rec))
      Recs.erase(RetIndex(Recs,Rec));
    endif
  endforeach
  SetGuildRecruits(Guild,Recs);

  return Recs;

endfunction

//-----------------------------------------------------------------------------

function GetGuildRecruit(Guild,Nr)

  return FindPlayerBySerial(GetGuildRecruits(Guild)[Nr]);

endfunction

//-----------------------------------------------------------------------------

function SetGuildRecruits(guild,Recruits)

  return guild.SetProp("recruits",Recruits);

endfunction

//-----------------------------------------------------------------------------

function IsGuildRecruit(guild,WhoSer)

  var Recruits := GetGuildRecruits(guild);
  foreach Recruit in Recruits
    if(CInt(Recruit) == WhoSer)
      return 1;
    endif
  endforeach
  return 0;
  
endfunction

//-----------------------------------------------------------------------------

function AddGuildRecruit(Guild,Recruit)

  if(IsGuildRecruit(Guild,Recruit.serial))
    return error;
  endif
  
  if(GetObjProperty(Recruit,"GdSTime") + RECRUIT_TIME > ReadGameClock())
    SendSysMessage(Recruit,"Zbyt malo czasu minelo od twej ostatniej przynaleznosci do gildii.",FONT_NORMAL,COLOR_RED);
    return 0;
  endif
  
  var Recruits := GetGuildRecruits(Guild);
  Recruits.append(Recruit.serial);
  SetGuildRecruits(Guild,Recruits);
  SetGuildId(Recruit,Guild.guildid);
  SetGuildAbv(Recruit,1);
  if(!GetObjProperty(GetStoneFromGuild(Guild), "Ksiestwo"))
    SetMemberTitle(Recruit,"Rekrut"+ggm(Recruit,14));
    RefreshGuildTitle(Recruit, Guild.guildid);
  else
    RefreshPrincipalityTitle(Recruit, "Rekrut", GetStoneFromGuild(Guild));
  endif

  return 1;

endfunction

//-----------------------------------------------------------------------------

function EraseGuildRecruit(Guild,Recruit)

  if(!IsGuildRecruit(Guild,Recruit.serial))
    return error;
  endif

  var Recruits := GetGuildRecruits(Guild);
  Recruits.erase(retindex(Recruits,Recruit.serial));
  SetGuildRecruits(Guild,Recruits);
  return 1;

endfunction

//-----------------------------------------------------------------------------

function FindGuildGm(Guild)

  var Stone := GetStoneFromGuild(Guild);

  if(!GetObjProperty(Stone, "Ksiestwo"))
    return FindPlayerBySerial(CInt(Guild.GetProp("gm")));
  else
    return FindPlayerBySerial(GetObjProperty(Stone, "GuildGm"));
  endif

endfunction

//-----------------------------------------------------------------------------

function SetGuildGm(guild, gm, Stone := 0)

  if(IsPlayer(gm))
    if(!Stone)
      Stone := GetStoneFromGuild(Guild);
    endif

    if(!GetObjProperty(Stone, "Ksiestwo"))
      if(guild.SetProp("gm",CInt(gm.serial)))
        if(IsGuildSeer(Guild,gm))
          SetGuildSeer(Guild, 0);
        endif
        return 1;
      endif
    else
      if(SetObjProperty(Stone, "GuildGm", gm.serial))
        if(IsGuildSeer(Guild,gm))
          SetGuildSeer(Guild, 0);
        endif
        return 1;
      endif
    endif
  endif
  return 0;

endfunction

//-----------------------------------------------------------------------------

function IsGuildGm(Guild,who)

  if(who == FindGuildGm(Guild))
    return 1;
  endif
  return 0;

endfunction

//-----------------------------------------------------------------------------

function FindGuildSeer(guild)

  if(!GetObjProperty(GetStoneFromGuild(Guild), "Ksiestwo"))
    return FindPlayerBySerial(CInt(guild.GetProp("seer")));
  else
    return FindPlayerBySerial(GetObjProperty(GetStoneFromGuild(Guild), "GuildSeer"));
  endif

endfunction

//-----------------------------------------------------------------------------

function SetGuildSeer(Guild, seer)

  var Stone := GetStoneFromGuild(Guild);
  var Ksiestwo := GetObjProperty(GetStoneFromGuild(Guild), "Ksiestwo");

  if(!seer)
    if(!Ksiestwo)
      return Guild.EraseProp("seer");
    else
      return EraseObjProperty(Stone, "GuildSeer");
    endif
  endif

  if(IsPlayer(seer))
    if(!Ksiestwo)
      return Guild.SetProp("seer", CInt(seer.serial));
    else
      return SetObjProperty(Stone, "GuildSeer", CInt(seer.serial));
    endif
  endif

  return 0;

endfunction

//-----------------------------------------------------------------------------

function IsGuildSeer(Guild, who)

  if(who == FindGuildSeer(Guild))
    return 1;
  endif
  return 0;

endfunction

//-----------------------------------------------------------------------------

function IsGuildMember(guild,who)

  if(who in Guild.members or who.serial in GetObjProperty(GetStoneFromGuild(Guild), "PrinceMembers"))
    return 1;
  endif
  return 0;

endfunction

//-----------------------------------------------------------------------------

function GetGuildMember(Guild,Nr)

  if(!GetObjProperty(GetStoneFromGuild(Guild), "Ksiestwo"))
    var Members := Guild.members,i;
    for(i:=1;i<=Members.size();i:=i+1)
      if(i == Nr)
        return Members[i];
      endif
    endfor
    return 0;
  else
    var PrinceMembers := GetObjProperty(GetStoneFromGuild(Guild), "PrinceMembers");
    return FindPlayerBySerial(PrinceMembers[Nr]);
  endif

endfunction

//-----------------------------------------------------------------------------

function GetMemberTitle(who)

  if(!GetObjProperty(who,"guildtitle"))
    var Guild := FindGuild(GetGuildID(who));
    if(IsGuildGm(Guild,who))
      SetMemberTitle(who,"Mistrz Gildii");
    else
      SetMemberTitle(who,GetGuildName(Guild));
    endif
  endif
  return ClStr(GetObjProperty(who,"guildtitle"));

endfunction

//-----------------------------------------------------------------------------

function SetMemberTitle(who,title)

  return SetObjProperty(who,"guildtitle",ClStr(title));

endfunction

//-----------------------------------------------------------------------------

function RefreshGuildTitle(who, GuildId)

  var Title := GetMemberTitle(who);
  var Guild := FindGuild(GuildId);
  var Abv   := GetGuildAbv(Guild);
  var Stone := GetStoneFromGuild(Guild);

  if(GetGuildAbv(who))
    if(Abv)
      Abv := " [" + Abv + "]";
    endif
    if(GetObjProperty(GetStoneFromGuild(Guild), "Ksiestwo"))
      RefreshPrincipalityTitle(who, Title, Stone);
    else
      SetTitleGuild(who, Title + Abv);
    endif
  else
    if(GetObjProperty(GetStoneFromGuild(Guild), "Ksiestwo"))
      RefreshPrincipalityTitle(who, Title, Stone);
    else
      SetTitleGuild(who, Title);
    endif
  endif
  
endfunction

//-----------------------------------------------------------------------------

function RefreshPrincipalityTitle(who, Title, Stone)

  if(!GetObjProperty(who, "TitleRaceBakSet"))
    SetObjProperty(who, "title_race_backup", who.title_race);
    SetObjProperty(who, "TitleRaceBakSet", 1);
  endif

  Title := Title+" Ksiestwa "+SplitWords(Stone.name)[2];
  who.title_race := Title;
  
endfunction

//-----------------------------------------------------------------------------

function AddGuildMember(guild,NewMemb,ByWho := 0)

  if(IsGuildMember(Guild,NewMemb))
    return error;
  endif
  
  if(!IsGuildRecruit(Guild,NewMemb.serial))
    SendSysMessage(NewMemb,"By dolaczyc do Gildii, musisz najpierw zostac rekrutem.",FONT_NORMAL,COLOR_RED);
    if(ByWho)
      SendSysMessage(ByWho,"Przylaczyc do Gildii mozesz tylko rekruta.",FONT_NORMAL,COLOR_RED);
    endif
    return;
  endif

  if(EraseGuildRecruit(Guild,NewMemb))
    if(!GetObjProperty(GetStoneFromGuild(Guild), "Ksiestwo"))
      Guild.addmember(NewMemb);
      SetMemberTitle(NewMemb,GetGuildName(Guild));
      RefreshGuildTitle(NewMemb, guild);
    else
      var PrinceMembers := GetObjProperty(GetStoneFromGuild(Guild), "PrinceMembers");
      if(!IsArray(PrinceMembers))
        PrinceMembers := array;
      endif
      PrinceMembers.append(NewMemb.serial);
      SetObjProperty(GetStoneFromGuild(Guild), "PrinceMembers", PrinceMembers);
      RefreshGuildTitle(NewMemb, guild);
    endif
    SetObjProperty(NewMemb,"GdSTime",ReadGameClock());
    SendSysMessage(NewMemb,"Jestes teraz czlonkiem Gildii "+GetGuildName(Guild)+".",FONT_NORMAL,COLOR_BLUE);
    if(ByWho)
      SendSysMessage(ByWho, GetName(NewMemb) + " jest teraz czlonkiem Gildii.",FONT_NORMAL,COLOR_BLUE);
    endif
  endif

endfunction

//-----------------------------------------------------------------------------

function DismissGuildMember(guild,Member,Force := 0)

  if(!IsGuildMember(Guild,Member) and !IsGuildRecruit(Guild,Member.serial) and !IsGuildGm(Guild,Member))
    return error;
  endif

  var Ksiestwo := GetObjProperty(GetStoneFromGuild(Guild), "Ksiestwo");

  if(IsGuildGm(Guild,Member))
    if(Guild.members.size() > 1 and !Force)
      SendSysMessage(Member,"Mistrz nie moze opuscic Gildii gdy nadal sa czlonkowie.",FONT_NORMAL,COLOR_RED);
      return 0;
    endif

    if(GetGuildCreateTime(Guild) + GUILD_TIME > ReadGameClock() and !Force)
      SendSysMessage(Member,"Nie mozesz zniszczyc Gildii tak wczesnie.",FONT_NORMAL,COLOR_RED);
      return 0;
    endif
    
    if(Force)
      ClearGuildData(Guild);
    endif
  endif
  
  if(IsGuildRecruit(Guild,Member.serial))
    EraseGuildRecruit(Guild,Member);
    SendSysMessage(Member,"Nie jestes juz rekrut"+ggm(Member,9)+" Gildii "+GetGuildName(Guild)+".",FONT_NORMAL,COLOR_GREEN);
  else
    if(!Force)
      if(!CInt(GetObjProperty(Member,"GdSTime")))
        SetObjProperty(Member,"GdSTime",ReadGameClock()-MEMBER_TIME);
      endif

      if(GetObjProperty(Member,"GdSTime") + MEMBER_TIME > ReadGameClock())
        SendSysMessage(Member,"Nie mozesz opuscic gildii tak wczesnie.",FONT_NORMAL,COLOR_RED);
        return 0;
      //elseif(Guild.members.size() <= MIN_GUILD_MEMBERS and !Ksiestwo)
      //  if(!Guild.GetProp("MembersLeft"))
      //    Guild.SetProp("MembersLeft", ReadGameClock());
      //  endif
      endif
    endif

    SetObjProperty(Member,"GdSTime",ReadGameClock());
    if(Ksiestwo)
      var Stone := GetStoneFromGuild(Guild);
      var PrinceMembers := GetObjProperty(Stone, "PrinceMembers");
      PrinceMembers.erase(RetIndex(PrinceMembers, Member.serial));
      SetObjProperty(Stone, "PrinceMembers", PrinceMembers);
    else
      Guild.removemember(Member);
    endif
    SendSysMessage(Member,"Nie jestes juz czlonkiem Gildii "+GetGuildName(Guild)+".",FONT_NORMAL,COLOR_GREEN);
  endif

  UndoMod(Member, MOD_TGUILD, UMDF_WAIT);
  if(GetObjProperty(GetStoneFromGuild(Guild), "Ksiestwo"))
    if(GetObjProperty(Member, "TitleRaceBakSet"))
      Member.title_race := GetObjProperty(Member, "title_race_backup");
      EraseObjProperty(Member, "TitleRaceBakSet");
      EraseObjProperty(Member, "title_race_backup");
    endif
  else
    SetTitleGuild(Member, "");
  endif
  EraseObjProperty(Member,PROP_GUILDID);
  EraseObjProperty(Member,"abv");
  EraseObjProperty(Member,"guildname");
  EraseObjProperty(Member,"fealty");

  if(IsGuildSeer(Guild, Member))
    SetGuildSeer(Guild, 0);
  endif

  if(Guild.members.size() <= 0)
    SetGuildGm(Guild,0);
    EraseGuild(Guild);
  endif

  return 1;

endfunction

//-----------------------------------------------------------------------------

function ResignGuild(who,Guild,ByWho := 0)

  var Res := DismissGuildMember(Guild,who,IsAdmin(ByWho));

  if(Res == error)
    SendSysMessage(who,"Nie jestes czlonkiem Gildii "+GetGuildName(Guild)+".",FONT_NORMAL,COLOR_GREEN);
    return error;
  elseif(ByWho and !Res)
    SendSysMessage(ByWho,who.name + " nie moze opuscic gildii "+GetGuildName(Guild)+".",FONT_NORMAL,COLOR_GREEN);
    return 0;
  elseif(ByWho)
    SendSysMessage(ByWho,who.name + " nie jest juz czlonkiem Gildii "+GetGuildName(Guild)+".",FONT_NORMAL,COLOR_GREEN);
  endif
  return 1;
  
endfunction

//-----------------------------------------------------------------------------

function EraseGuild(Guild)

  if(Guild.members.size())
    return 0;
  endif
  
  var Stone := GetStoneFromGuild(Guild);

  if(Stone)
    if(!ExPid(GetObjProperty(Stone,"#Destroying")))
      if(!DestroyItem(Stone))
        LogError("EraseGuild: Nie mozna bylo zniszczyc kamienia gildii "+Stone.serial+" ["+Stone.x+","+Stone.y+","+Stone.z+" "+Stone.realm+"]");
      endif
    endif
  endif
  return 1;
  
endfunction

//-----------------------------------------------------------------------------

function GetStoneFromGuild(Guild)

  return SystemFindObjectBySerial(CInt(Guild.GetProp("StoneSerial")));

endfunction

//-----------------------------------------------------------------------------

function SetGuildStone(Guild,Stone)

  Guild.SetProp("StoneSerial",Stone.serial);

endfunction

//-----------------------------------------------------------------------------

function GetAllyDecs(Guild)

  var Decs := Guild.GetProp("AllyDeclarations");

  if(!Decs)
    Decs := dictionary;
    SetAllyDecs(Guild, Decs);
  else
    var Keys := Decs.keys();
    var bSave := 0;

    foreach GuildId in Keys
      if(!FindGuild(GuildId))
        Decs.erase(GuildId);
        bSave := 1;
      endif
    endforeach

    if(bSave)
      SetAllyDecs(Guild, Decs);
    endif
  endif

  return Decs;

endfunction

//-----------------------------------------------------------------------------

function SetAllyDecs(Guild,Decs)

  return Guild.SetProp("AllyDeclarations",Decs);

endfunction

//-----------------------------------------------------------------------------

function GetEnemyDecs(Guild)

  var Decs := Guild.GetProp("EnemyDeclarations");

  if(!Decs)
    Decs := dictionary;
    SetEnemyDecs(Guild, Decs);
  else
    var Keys := Decs.keys();
    var bSave := 0;

    foreach GuildId in Keys
      if(!FindGuild(GuildId))
        Decs.erase(GuildId);
        bSave := 1;
      endif
    endforeach

    if(bSave)
      SetEnemyDecs(Guild, Decs);
    endif
  endif

  return Decs;

endfunction

//-----------------------------------------------------------------------------

function SetEnemyDecs(Guild,Decs)

  return Guild.SetProp("EnemyDeclarations",Decs);

endfunction

//-----------------------------------------------------------------------------

function ClearGuildData(Guild)

  if(ExPid(Guild.GetProp("#Clearing")))
    return;
  endif

  Guild.SetProp("#Clearing",GetPid());

  var Gm    := FindGuildGm(Guild);
  
  foreach Rec in GetGuildRecruits(Guild)
    Rec := FindPlayerBySerial(Rec);
    if(Rec)
      DismissGuildMember(Guild,Rec,1);
    endif
  endforeach

  foreach AGuild in (Guild.allyguilds)
    Guild.removeallyguild(AGuild);
  endforeach

  foreach EGuild in (Guild.enemyguilds)
    Guild.removeenemyguild(EGuild);
  endforeach
  
  foreach Member in (Guild.members)
    if(Member != Gm)
      DismissGuildMember(Guild,Member,1);
    endif
  endforeach

  if(Gm)
    DismissGuildMember(Guild,Gm,1);
  endif

  if(Guild.multi)
    ResetHousePayment(Guild.multi);
  endif

endfunction

//-----------------------------------------------------------------------------

function SetGuildDecDelay(Guild,GuildId := error)

  if(Guild)
    Guild.SetProp("DecDelay",ReadGameClock());
  endif
  
  if(GuildId)
    var GDDelay := Guild.GetProp("DecDelayIds"),i := 1;
    if(!GDDelay)
      GDDelay := dictionary;
    endif
    foreach Guild in GDDelay
      if(CInt(Guild) + GUILD_ONE_DECDELAY < ReadGameClock())
        GDDelay.erase(i);
      else
        i := i + 1;
      endif
    endforeach
    
    GDDelay[GuildId] := ReadGameClock();
    Guild.SetProp("DecDelayIds",GDDelay);
  endif

endfunction

//-----------------------------------------------------------------------------

function GetGuildDecDelay(Guild,GuildId := error)

  if(GuildId == error)
    return CInt(Guild.GetProp("DecDelay"));
  else
    var GDDelay := Guild.GetProp("DecDelayIds");
    if(GDDelay)
      return CInt(GDDelay[GuildId]);
    endif
  endif
  
  return 0;

endfunction

//-----------------------------------------------------------------------------

function GetGuildStat(Guild)

  var GuildSt  := Guild.GetProp("GuildsStat");

  if(!GuildSt)
    GuildSt := dictionary;
  endif
  
  return GuildSt;
  
endfunction

//-----------------------------------------------------------------------------

function RefreshGuildsStat(Guild)

  var NGuildSt := dictionary;
  var GuildSt  := GetGuildStat(Guild);

  foreach Guild in (Guild.allyguilds)
    if(!GuildSt[Guild.guildid])
      NGuildSt[Guild.guildid] := ReadGameClock();
    else
      NGuildSt[Guild.guildid] := GuildSt[Guild.guildid];
    endif
  endforeach

  foreach Guild in (Guild.enemyguilds)
    if(!GuildSt[Guild.guildid])
      NGuildSt[Guild.guildid] := ReadGameClock();
    else
      NGuildSt[Guild.guildid] := GuildSt[Guild.guildid];
    endif
  endforeach
  
  Guild.SetProp("GuildsStat",NGuildSt);

endfunction

//-----------------------------------------------------------------------------

function SetDeclinePunisment(who,Guild,EGuild)

  var Price := DECWAR_GOLD * Guild.members.size();

  if(!Spendgold(who, Price))
    return 0;
  else
    var Gm   := FindGuildGm(EGuild);
    var Bank := FindBankBox(Gm);
    CreateItemInCont(Bank,UOBJ_GOLDCOIN,Price);
  endif
  
  Guild.SetProp("DeclineCounter",GetDecCn(Guild)+1);
  return 1;
  
endfunction

//-----------------------------------------------------------------------------

function GetDecCn(Guild)

  return CInt(Guild.GetProp("DeclineCounter"));

endfunction
 
//-----------------------------------------------------------------------------

function AlarmMembers(Guild,Text,Color := COLOR_RED)

  foreach Memb in (Guild.members)
    SendSysMessage(Memb,Text,FONT_NORMAL,Color);
  endforeach

endfunction

//-----------------------------------------------------------------------------

function GetNonChaosGuildID(who)

  var GuildIdArr := GetGuildId(who);
  if(!GuildIdArr.size())
    return error;
  endif

  foreach id in GuildIdArr
    var Guild := FindGuild(id);
    if(Guild)
      var Status := GetGuildStatus(Guild);;
      if(Status != CHAOS_ST)
        return Status;
      endif
    endif
  endforeach

  return error;

endfunction