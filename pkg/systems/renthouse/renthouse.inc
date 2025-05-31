//-----------------------------------------------------------------------------

include "include/findcity";
include "include/gumpboxes";
include "include/clock";
include ":banking:common";
include "include/key";
include "include/logerror";
include "include/buildings";
include "include/checks/objchecks";
include ":banking:gold";
include "include/player/target";
include ":housing:housing";
include ":fences:fences";
include ":carpets:rolls";

include "common";
include ":carpentry:deeds";
include ":mail:mail";

//-----------------------------------------------------------------------------

const HR_CFG                           := ":renthouse:renthouse";
const HR_RESIGN_TIME                   := 604800; //7 dni
const HR_INFORM_START_TIME             := 259200;
const HR_INFORM_DELAY                  := 86400;

const UOBJ_HRCONT_N                    := 0xA900;
const UOBJ_HRCONT_W                    := 0xA901;

const MAX_HR_BLOCKED_ITEMS             := 965;
const MAX_HR_BLOCKED_CONTS             := 965;
const MAX_HR_FRIENDS                   := 15;

//-----------------------------------------------------------------------------

function RentHouse(Npc, who)

  var NpcCity  := FindCity(Npc);
  var Res := CanRent(NpcCity,who);

  if(Res == error)
    PrintTextAbove(Npc,"Przykro mi, ale nie mam nic do wynajecia w tej chwili.");
    return;
  elseif(!Res)
    PrintTextAbovePrivate(Npc,"Juz wynaj"+ggm(who,5)+" dom w tym miescie.",who);
    return;
  endif
  
  PrintTextAbovePrivate(Npc,"Jaki dom chcial"+ggm(who,1)+" wynajac?",who);
  ev := SendNpcHandleEvent(EVID_NH_RENT_HOUSE,who,Npc,NpcCity);
  
  if(Distance(Npc,who) > 2)
    Say("*wzdycha*");
    return;
  endif
  
  var House := ev.house;
  
  if(!House)
    PrintTextAbovePrivate(Npc,"Moze innym razem.",who);
    return;
  endif
  
  PrintTextAbovePrivate(Npc,"Na jak dlugo?",who);
  ev := SendNpcHandleEvent(EVID_NH_RENT_TIME,who,Npc,House);

  if(Distance(Npc,who) > 2)
    Say("*wzdycha*");
    return;
  endif

  var Time := ev.time;

  if(!Time)
    PrintTextAbovePrivate(Npc,"Moze innym razem.",who);
    return;
  endif

  SetRentHouse(Npc,who,House,Time);

endfunction

//-----------------------------------------------------------------------------

function CanRent(HrCity,who)

  var Houses  := GetCityHouses(HrCity);
  var Sign;

  if(!Houses.size())
    return error;
  endif
  
  foreach House in Houses
    Sign := FindHrSign(House);
    if(Sign)
      if(GetOwnerSerial(Sign) == who.serial)
        return 0;
      endif
    endif
  endforeach

  return 1;

endfunction

//-----------------------------------------------------------------------------

function SetOwnerSerial(Sign,Ser)

  return SetObjProperty(Sign,PROP_OWNER,CInt(Ser));

endfunction

//-----------------------------------------------------------------------------

function GetStartTime(Sign)

  return CInt(GetObjProperty(Sign,"StartTime"));

endfunction

//-----------------------------------------------------------------------------

function GetRentTime(Sign)

  return CInt(GetObjProperty(Sign,"RentTime"));

endfunction

//-----------------------------------------------------------------------------

function SetRentTime(Sign,Time)

  return SetObjProperty(Sign,"RentTime",CInt(Time));

endfunction

//-----------------------------------------------------------------------------

function FindHrCont(Sign)

  if(!Sign)
    return 0;
  endif

  var Cont := GetObjProperty(Sign,"HrContSer");

  if(!Cont.size())
    Cont := CreateHrCont(Sign);
  endif
  
  return Cont;

endfunction

//-----------------------------------------------------------------------------

function FindHrSign(House)

  if(!House.SignX or !House.SignY)
    return 0;
  endif

  foreach Item in ListItemsAtLocation(House.SignX,House.SignY,House.SignZ,GetCityRealm(House.City))
    if(GetHrNr(Item))
      return Item;
    endif
  endforeach
  
  return CreateHrSign(House);

endfunction

//-----------------------------------------------------------------------------

function FindHrDoors(House)

  var Doors := {};
  var Rlm   := GetCityRealm(House.City);

  if(House.Door1X and House.Door1Y)
    foreach Item in ListItemsAtLocation(CInt(House.Door1X), CInt(House.Door1Y), CInt(House.Door1Z), Rlm)
      if(Item.isa(POLCLASS_DOOR))
        Doors.append(Item);
      endif
    endforeach
  endif

  if(House.Door2X and House.Door2Y)
    foreach Item in ListItemsAtLocation(CInt(House.Door2X), CInt(House.Door2Y), CInt(House.Door2Z), Rlm)
      if(Item.isa(POLCLASS_DOOR))
        Doors.append(Item);
      endif
    endforeach
  endif

  return Doors;

endfunction

//-----------------------------------------------------------------------------
// Creation
//-----------------------------------------------------------------------------

function CreateHrCont(who)

  var Sign := CheckNearHr(who.x,who.y,who.z,who.realm);
  
  if(!Sign)
    return;
  endif

  if(GetOwnerSerial(Sign) != who.serial)
    return;
  endif

  if(GetRhSecureNum(Sign) >= GetRhSecureMaxNum(Sign))
    SendSysMessage(who,"Nie mozesz postawic wiecej zabezpieczonych pojemnikow.",FONT_NORMAL,COLOR_RED);
    return;
  endif
  
  var Facing;

  var HrCity   := GetHrCity(Sign);
  var HouseNr  := GetHrNr(Sign);
  var House    := GetHr(HrCity,HouseNr);

  var Place := LocTarget(who);

  if(!Place)
    return;
  endif

  NoLongerBusy(who);

  if(!IsInHr(Place.x, Place.y, Place.z, Place.Realm,House))
    SendSysMessage(who,"Wybrane miejsce nie jest w domu.",FONT_NORMAL,COLOR_RED);
    return;
  endif

  if(!CanPlaceInHouseAtLoc(Place, Place))
    SendSysMessage(who,"Nie mozesz postawic bezpiecznego pojemnika tak blisko drzwi.",FONT_NORMAL,COLOR_RED);
    return;
  endif

  var z := GetStandZ(Place.x, Place.y, who.realm, Place.z);

  if(z == error or DistZ(z, Place.z) > 10)
    SendSysMessage(who, "To miejsce jest niedostepne.", FONT_NORMAL, COLOR_RED);
    return 0;
  endif

  case(who.facing)
    2:
    6:  Facing := UOBJ_HRCONT_N;

    0:
    4:  Facing := UOBJ_HRCONT_W;

    default:  SendSysMessage(who,"W tym kierunku nie postawisz skrzyni. Patrz w inna strone.",FONT_NORMAL,COLOR_RED);
              return 0;
  endcase

  var HrCont := CreateItemAtLoc(Place.x, Place.y, Place.z, Facing, 1, who.realm);

  if(HrCont)
    var SecNum := CInt(GetObjProperty(Sign,"SecNum"));
    var HrContSer := GetObjProperty(Sign,"HrContSer");
    var OldHrContSer;

    if(!HrContSer.size())
      if(HrContSer)
        OldHrContSer := HrContSer;
      endif
      HrContSer := array;
      if(OldHrContSer)
        HrContSer.append(OldHrContSer);
      endif
    endif
    if(!HrContSer)
      HrContSer := array;
    endif

    HrContSer.append(HrCont.serial);
    SecNum := SecNum + 1;

    SetObjProperty(Sign,"HrContSer", HrContSer);
    SetObjProperty(HrCont,"SignSer",Sign.serial);
    SetObjProperty(HrCont,"Secured",1);
    SetObjProperty(Sign,"SecNum",SecNum);
    SendSysMessage(who,"Postawil"+ggm(who,3)+" bezpieczny pojemnik.",FONT_NORMAL,COLOR_GREEN);
    return HrCont;
  else
    SendSysMessage(who,"Nie udalo Ci sie postawic bezpiecznego pojemnika.",FONT_NORMAL,COLOR_RED);
    return 0;
  endif
  
endfunction

//-----------------------------------------------------------------------------

function EraseHrCont(who)

  var Sign := CheckNearHr(who.x,who.y,who.z,who.realm);

  if(!Sign)
    return;
  endif

  if(GetOwnerSerial(Sign) != who.serial)
    return;
  endif

  //var SecContNum := GetRhSecureNum(Sign);
  var SecureCont := ObjTarget(who);

  if(!StandardItemCheck(who,SecureCont,NO_MOVABLE_CHECK))
    ReleaseItem(SecureCont);
    NoLongerBusy(who);
    return;
  endif

  ReleaseItem(SecureCont);
  NoLongerBusy(who);

  if(!SecureCont.isa(POLCLASS_CONTAINER))
    SendSysMessage(who,"To nie pojemnik.",FONT_NORMAL,COLOR_RED);
    return;
  endif

 if(GetObjProperty(SecureCont,"SignSer") != Sign.serial)
    SendSysMessage(who,"Ten przedmiot nie znajduje sie w twoim domu.",FONT_NORMAL,COLOR_RED);
    return;
  endif

  if(len(EnumerateItemsInContainer(SecureCont)))
    SendSysMessage(who,"Ten pojemnik nie jest pusty.",FONT_NORMAL,COLOR_RED);
    return;
  endif

  var ContSer := SecureCont.serial;

  if(DestroyItem(SecureCont))
    var ContList := GetObjProperty(Sign,"HrContSer");
    var SecNum   := GetObjProperty(Sign,"SecNum");
    var NewList  := array;
    var i;

    for(i:=1;i<=ContList.size();i+=1)
      if(ContList[i] != ContSer)
        NewList.append(ContList[i]);
      endif
    endfor

    SetObjProperty(Sign,"HrContSer", NewList);
    SetObjProperty(Sign,"SecNum",SecNum-1);
    SendSysMessage(who,"Zniszczyl"+ggm(who,3)+" bezpieczny pojemnik.",FONT_NORMAL,COLOR_GREEN);
    return 1;
  else
    SendSysMessage(who,"Nie udalo Ci sie zniszczyc pojemnika.",FONT_NORMAL,COLOR_RED);
    return 0;
  endif  

endfunction

//-----------------------------------------------------------------------------

function CreateHrTrashBarrel(who)

  var Sign := CheckNearHr(who.x,who.y,who.z,who.realm);

  if(!Sign)
    return;
  endif

  if(GetOwnerSerial(Sign) != who.serial)
    return;
  endif

  if(GetObjProperty(Sign, "TrashBarrelSerial"))
    SendSysMessage(who,"Ten dom ma juz smietnik.",FONT_NORMAL,COLOR_RED);
    return;
  endif

  var Place := LocTarget(who);

  if(!StandardLocCheck(who,Place))
    return;
  endif

  NoLongerBusy(who);

  var HrCity   := GetHrCity(Sign);
  var HouseNr  := GetHrNr(Sign);
  var House    := GetHr(HrCity,HouseNr);

  if(!IsInHr(Place.x, Place.y, Place.z, Place.Realm,House))
    SendSysMessage(who,"Wybrane miejsce nie jest w domu.",FONT_NORMAL,COLOR_RED);
    return;
  endif

  if(!CanPlaceInHouseAtLoc(Place, Place))
    SendSysMessage(who,"Nie mozesz postawic smietnika tak blisko drzwi.",FONT_NORMAL,COLOR_RED);
    return;
  endif

  var z := GetStandZ(Place.x, Place.y, who.realm, Place.z);

  if(z == error or DistZ(z, Place.z) > 10)
    SendSysMessage(who, "To miejsce jest niedostepne.", FONT_NORMAL, COLOR_RED);
    return 0;
  endif

  var TrashBarrel := CreateItemAtLoc(Place.x, Place.y, Place.z, UOBJ_TRASHBARREL, 1, who.realm);
  
  if(TrashBarrel)
    SetObjProperty(Sign, "TrashBarrelSerial", TrashBarrel.serial);
    SetObjProperty(TrashBarrel, "SignSer", Sign.serial);
    SendSysMessage(who,"Postawil"+ggm(who,3)+" smietnik.",FONT_NORMAL,COLOR_GREEN);
    return 1;
  else
    SendSysMessage(who,"Nie udalo Ci sie postawic smietnika.",FONT_NORMAL,COLOR_RED);
    return 0;
  endif

endfunction

//-----------------------------------------------------------------------------

function DestroyHrTrashBarrel(who)

  var Sign := CheckNearHr(who.x,who.y,who.z,who.realm);

  if(!Sign)
    return;
  endif

  if(GetOwnerSerial(Sign) != who.serial)
    return;
  endif

  var TrashBarrel := FindItemBySerial(GetObjProperty(Sign, "TrashBarrelSerial"));

  if(!TrashBarrel)
    SendSysMessage(who,"Ten dom nie ma smietnika.",FONT_NORMAL,COLOR_RED);
    return;
  endif

  if(TrashBarrel)
    if(DestroyItem(TrashBarrel))
      EraseObjProperty(Sign, "TrashBarrelSerial");
      SendSysMessage(who,"Zniszczyl"+ggm(who,3)+" smietnik.",FONT_NORMAL,COLOR_GREEN);
    else
      SendSysMessage(who," Nie udalo Ci sie zniszczyc smietnika.",FONT_NORMAL,COLOR_RED);
    endif
  else
    SendSysMessage(who,"Ten dom nie ma smietnika.",FONT_NORMAL,COLOR_RED);
    return;
  endif

  return;

endfunction

//-----------------------------------------------------------------------------

function CreateHrSign(House)

  var HrCfg  := ReadConfigFile(HR_CFG);

  var SignOt := GetConfigInt(HrCfg["Signs"],House.SignType+"_"+House.SignDir);
  var PostOt := GetConfigInt(HrCfg["Signs"],House.PostType+"_"+House.SignDir);
  var Rlm    := GetCityRealm(House.City);

  var Sign,Post;

  foreach Item in ListItemsAtLocation(CInt(House.SignX),CInt(House.SignY),CInt(House.SignZ),Rlm)
    if(Item.objtype == SignOt)
      Sign := Item;
    elseif(Item.objtype == PostOt)
      Post := Item;
    endif
  endforeach

  if(!Sign)
    Sign := CreateItemAtLoc(House.SignX,House.SignY,House.SignZ,SignOt,1,Rlm);
  endif
  
  if(!Post)
    Post := CreateItemAtLoc(House.SignX,House.SignY,House.SignZ,PostOt,1,Rlm);
  endif
  
  if(Sign)
    SetObjProperty(Sign,"HrNr",CInt(House.HouseNr));
    SetObjProperty(Sign,"HrCity",House.City);
    SetObjProperty(Sign,"SignPost",Post.serial);
  endif
  
  return Sign;

endfunction

//-----------------------------------------------------------------------------

function CheckRentHouse(Sign, Npc := 0)

  if(Sign)
    var TimeLeft := (GetStartTime(Sign) + GetRentTime(Sign)) - ReadGameClock();
    var Owner := GetOwner(Sign);
    
    if(!Owner or (TimeLeft <= 0))
      if(Owner)
        ExtensionRentPostInformMail(Owner, Npc, Sign);
      endif
      return 0;
    endif

    if(Npc)
      if(TimeLeft <= HR_INFORM_START_TIME)
        var BeginPeriods := CInt(HR_INFORM_START_TIME / HR_INFORM_DELAY);
        var Periods := BeginPeriods - CInt(TimeLeft / HR_INFORM_DELAY);
        var InfPeriods := CInt(GetObjProperty(Sign, "InformPeriods"));

        if(Periods > InfPeriods)
          SetObjProperty(Sign, "InformPeriods", Periods);
          HrPostInformMail(Owner, Npc, Sign);
        endif
      else
        EraseObjProperty(Sign, "InformPeriods");
      endif
    endif

    return 1;
  endif

  return 0;
  
endfunction

//-----------------------------------------------------------------------------

function HrPostInformMail(ToWho, FromWho, Sign)

  var Text;
  var TimeLeft := (GetStartTime(Sign) + GetRentTime(Sign)) - ReadGameClock();
  var Owner := GetOwner(Sign);
  var City := FindCity(Sign);

  Text := array("Witam!", "", "Chcial"+ggm(FromWho, 25)+" poinformowac, iz w niedlugim czasie", "zakonczy sie okres wynajmowania domu o numerze "+GetHrNr(Sign), "znajdujacego sie w miescie "+City+" w lokacji:", ToSextant(Sign.x, Sign.y, Sign.realm),
                "Pamietaj, ze zawsze mozesz przedluzyc wynajem!");

  Text += array("", "Wlasciciel domu: "+GetName(Owner));
  Text += array("Pozostaly czas wynajmu: "+DescGameTime(TimeLeft));

  var Res := PostTextMail(ToWho, FromWho, "Nadchodzacy koniec okresu wynajmu domu", Text);

  if(!Res)
    LogError("[HR] Nie udalo sie wyslac maila paymentu ["+Owner.serial+"/"+GetName(Owner)+" "+hex(Sign.serial)+"]: "+Res.errortext);
  endif


endfunction

//-----------------------------------------------------------------------------

function ExtensionRentPostInformMail(ToWho, FromWho, Sign)

  var Text;
  var TimeLeft := (GetStartTime(Sign) + GetRentTime(Sign)) - ReadGameClock();
  var Owner := GetOwner(Sign);
  var City := FindCity(Sign);

  Text := array("Witam!", "", "Chcial"+ggm(FromWho, 25)+" poinformowac, iz przedluzono wynajem domu o numerze "+GetHrNr(Sign), "znajdujacego sie w miescie "+City+" w lokacji:", ToSextant(Sign.x, Sign.y, Sign.realm));

  Text += array("", "Wlasciciel domu: "+GetName(Owner));
  Text += array("Pozostaly czas wynajmu: "+DescGameTime(TimeLeft));

  var Res := PostTextMail(ToWho, FromWho, "Przedluzony okresu wynajmu domu", Text);

  if(!Res)
    LogError("[HR] Nie udalo sie wyslac maila paymentu ["+Owner.serial+"/"+GetName(Owner)+" "+hex(Sign.serial)+"]: "+Res.errortext);
  endif


endfunction

//-----------------------------------------------------------------------------

function GetCityHouses(City)

  if(!City)
    return array;
  endif

  var CityCfg := ReadConfigFile(RENTHOUSES_CFG+lower(City));

  var Houses  := {};

  if(!CityCfg)
    return Houses;
  endif

  var i;
  
  for(i:=1;i<=GetConfigMaxIntKey(CityCfg);i:=i+1)
    Houses.append(FindConfigElem(CityCfg,i));
  endfor

  return Houses;

endfunction

//-----------------------------------------------------------------------------

function GetCityHousesCoords(City, AddRoom := 1)

  var Cities := GetCityHouses(City);
  var x, y;
  var Coords := dictionary;

  foreach CityElem in Cities

    if(!CityElem.Room or AddRoom)
      for(y:=CInt(CityElem.HouseYS); y<CInt(CityElem.HouseYE); y+=1)
        for(x:=CInt(CityElem.HouseXS); x<CInt(CityElem.HouseXE); x+=1)
          Coords[CStr(x)+"-"+CStr(y)] := 1;
        endfor
      endfor
    endif

  endforeach

  return Coords;

endfunction

//-----------------------------------------------------------------------------

function SelectRentHouse(who, Houses, City)

  GInit(50,50);
  GResPic(0,0,3600,400,380);
  GPage();
  GTextLine(120, 20, 550, "Wybierz Dom");
  GTextLine(260, 20, 999, "cena za 12 dni");
  GTextLine(60,340,550,"Anuluj");
  GButton(30,340,2714, 2715,0);

  GAddPageData(367,19,2650,2651,367,343,2648,2647);

  var i     := 1;
  var Rlm   := GetCityRealm(City);
  var GLvl  := GetPlayerNpcGuildCityLvl(who, City);
  
  foreach Hr in Houses
    
    GCheckNextPage(14);
    GTextLine(60,(GNPCount()*20)+50,560,i + ".");
    GTextLine(90,(GNPCount()*20)+50,570,GetRentHousePrice(Hr, 0, GLvl) + " gp");
    GTextLine(170,(GNPCount()*20)+50,550,ToSextant(CInt(Hr.Mx),CInt(Hr.My),Rlm));
    GTextLine(360,(GNPCount()*20)+50,550,CInt(Hr.NumSecure));
    if(!GetOwnerSerial(FindHrSign(Hr)))
      GButton(30,(GNPCount()*20)+50,2714, 2715,i);
    endif
    i := i + 1;
  endforeach

  return Houses[GSend(who)[0]];

endfunction

//-----------------------------------------------------------------------------

function SelectRentTime(who,House)

  GInit(50,50);
  GResPic(0,0,3600,400,380);
  GPage();
  GTextLine(120,20,550, "Wybierz Czas");
  GTextLine(60,340,550,"Anuluj");
  GButton(30,340,2714, 2715,0);

  GAddPageData(367,19,2650,2651,367,343,2648,2647);

  var i := 1;
  var HrCfg := ReadConfigFile(HR_CFG);
  var Times := GetConfigStringArray(HrCfg["RentTime"],"Time");
  var GLvl  := GetPlayerNpcGuildCityLvl(who, House.City);

  foreach Time in Times
    Time := CInt(Time);
    GCheckNextPage(14);
    GButton(30,(GNPCount()*20)+50,2714, 2715,i);
    GTextLine(60 ,(GNPCount()*20)+50,560,DescGameTime(Time));
    GTextLine(200,(GNPCount()*20)+50,560,GetRentHousePrice(House, Time, GLvl) + " gp");
    GTextLine(330 ,(GNPCount()*20)+50,560,((Time / 3600) / 24)+" dni");
    i := i + 1;
  endforeach

  return Times[GSend(who)[0]];

endfunction

//-----------------------------------------------------------------------------

function GetRentHousePrice(House, Time, GLvl)

  if(Time > 86400)
    Time := Time - 86400;
  else
    Time := 0;
  endif

  return CInt((CInt(House.Price) + CInt(House.Price) * (CInt(Time / 3600) * 0.03)) * ((10 - GLvl)*0.1));

endfunction

//-----------------------------------------------------------------------------

function SetRentHouse(Npc,who,House,Time)

  var Sign  := FindHrSign(House);
  var GLvl  := GetPlayerNpcGuildCityLvl(who, House.City);
  var Price := GetRentHousePrice(House, Time, GLvl);

  if(GetOwnerSerial(Sign))
    PrintTextAbovePrivate(Npc,"Ten dom jest juz wynajety.",who);
    return;
  endif

  var Doors := FindHrDoors(House);

  if(!Sign or !Doors.size())
    LogError("SetRentHouse: Dom Nr: "+House.HouseNr+" nie posiada drzwi "+Doors+" lub znaku "+CInt(Sign.serial)+" w miescie "+House.City+" Me: "+me.serial);
    PrintTextAbovePrivate(Npc,"Nie mozesz narazie wynajac tego domu.",who);
    return;
  endif

  if(!SpendGold(who, price))
    PrintTextAbovePrivate(Npc,"Nie widze bys mial"+ggm(who,4)+" w banku "+Price+" gp na wynajem tego domu.",who);
    return;
  endif

  var Key := CreateItemInPack(who,UOBJ_GOLD_KEY);
  
  if(!Key)
    PrintTextAbovePrivate(Npc,"Jak ja mam Ci dac klucz jak nie masz miejsca w plecaku?",who);
    PrintTextAbovePrivate(Npc,"*wzdycha*",who);
    return;
  endif

  var LockID := AlLocLockId();

  foreach Door in Doors
    SetItemLockID(Door,LockID);
    SetLock(Door,95);
  endforeach

  SetItemLockID(Key,LockID);
  Key.newbie := 1;

  SetObjProperty(Sign,PROP_OWNER,who.serial);

  SetRentTime(Sign,Time);
  SetObjProperty(Sign,"StartTime",ReadGameClock());
  
  PrintTextAbovePrivate(Npc,"Gratuluje",who);
  PrintTextAbovePrivate(Npc,"Wynaj"+ggm(who,5)+" dom, o to klucz do niego.",who);
  PrintTextAbovePrivate(Npc,"Kosztowalo Cie to "+Price+" gp.",who);
  PrintTextAbovePrivate(Npc,"Milego pobytu.",who);
  
endfunction

//-----------------------------------------------------------------------------

function EraseRentHouse(House, player := 0)

  var Sign  := FindHrSign(House);
  var Conts := FindHrCont(Sign);
  var Doors := FindHrDoors(House);
  var Owner := GetOwner(Sign);
  var Data;
  
  foreach Door in Doors
    EraseItemLockID(Door);
  endforeach

  if(IsPlayer(Owner))
    player := 1;
  endif

  if(player)
    Data  := GetEscrowData(Owner);
    
    if(Data)
      SetEscrowFilling(Data, 1);
      foreach box in Conts
        var Cont := FindItemBySerial(box);
        foreach Item in ListRootItemsInContainer(Cont)
          MoveItemToEscrow(item, Data, 1);
        endforeach
        DestroyItem(Cont);
      endforeach
    endif

    HrMoveItemsToEscrow(Owner, House, Sign);
  else
    var Bank  := FindBankBox(Owner);
    var Rlm   := GetCityRealm(House.City);

    if(Bank)
      foreach box in Conts
        var Cont := FindItemBySerial(box);
        foreach Item in ListRootItemsInContainer(Cont)
          if(!MoveItemToContainer(Item,Bank))
            if(!MoveObject(Item,Cont.x,Cont.y,Cont.z,Cont.realm,MOVEITEM_FORCELOCATION))
              DestroyItem(Item);
            endif
          endif
        endforeach
      endforeach

      foreach Item in ListObjectsInBox(CInt(House.HouseXS),CInt(House.HouseYS),-127,CInt(House.HouseXE),CInt(House.HouseYE),127,Rlm)
        if(Item.isa(POLCLASS_ITEM))
          if(IsHrItem(Item,Sign))
            if(IsItemFromDeed(Item))
              DestroyDeedItem(Item,Owner,Bank,1);
            elseif(IsHrBlockedItem(Item))
              EraseHrBlock(Item,Sign);
              MoveItemToContainer(Item,Bank);
            else
              DestroyItem(Item);
            endif
          endif
        endif
      endforeach
    endif
    
  endif

  DestroyHrTrashBarrel(Owner);
  EraseObjProperty(Sign,PROP_OWNER);
  EraseObjProperty(Sign,"StartTime");
  EraseObjProperty(Sign,"RentTime");
  EraseObjProperty(Sign,"HrContSer");
  EraseObjProperty(Sign,"SecNum");
  EraseObjProperty(Sign,"InformPeriods");
  SetHrFriends(Sign,{});

endfunction

//-----------------------------------------------------------------------------
// Blocking
//-----------------------------------------------------------------------------

function IsHrBlockedItem(Item)

  return CInt(GetObjProperty(Item,"HrBlock"));

endfunction

//-----------------------------------------------------------------------------

function EraseHrBlock(Item,Sign)

  if(!IsHrBlockedItem(Item))
    return 0;
  endif

  var Items := GetHrBLockedItems(Sign),i := 1;
  foreach ItemSer in Items
    if(ItemSer == Item.serial)
      Items.erase(i);
      break;
    endif
    i := i + 1;
  endforeach
  
  EraseObjProperty(Item,"HrBlock");
  Item.movable := 1;

  if(Item.isa(POLCLASS_CONTAINER))
    SetObjProperty(Sign, "HrConts", CInt(GetObjProperty(Sign, "HrConts"))-1);
  endif

  SetHrBlockedItems(Sign,Items);
  
  return 1;

endfunction

//-----------------------------------------------------------------------------

function AddHrBlock(Item,Sign,who := 0)

  if(IsHrBlockedItem(Item) or !Item.movable)
    if(who)
      SendSysMessage(who,"Ten przedmiot juz jest zablokowany.",FONT_NORMAL,COLOR_RED);
    endif
    return 0;
  endif

  var House := GetHr(GetHrCity(Sign),GetHrNr(Sign));
  
  if(!IsInHr(Item.x,Item.y,Item.z,Item.realm,House) or !CanControlHr(who,Sign))
    if(who)
      SendSysMessage(who,"Ten przedmiot nie znajduje sie w twoim domu.",FONT_NORMAL,COLOR_RED);
    endif
    return 0;
  endif
  
  if(Item.amount > 2 and (GetItemConfig(Item).StoreAble or IsStackableObj(Item.objtype)))
    SendSysMessage(who,"Tego przedmiotu nie mozesz zablokowac w tej ilosci.", FONT_NORMAL, COLOR_RED);
    SendSysMessage(who,"Mozliwe jest zablokowanie w ilosci maksymalnie dwoch sztuk.", FONT_NORMAL, COLOR_RED);
    return;
  endif

  if(Item.isa(POLCLASS_CONTAINER))
    if(CInt(GetObjProperty(Sign, "HrConts")) >= MAX_HR_BLOCKED_CONTS)
      SendSysMessage(who,"Nie mozesz zablokowac wiecej pojemnikow.", FONT_NORMAL, COLOR_RED);
      return;
    endif
  endif

  var Items := GetHrBlockedItems(Sign);
  
  if(Items.size() >= MAX_HR_BLOCKED_ITEMS)
    SendSysMessage(who,"Nie mozesz zablokowac wiecej przedmiotow.",FONT_NORMAL,COLOR_RED);
    return 0;
  endif
  
  Items.append(Item.serial);
  SetHrBlockedItems(Sign,Items);
  
  SetObjProperty(Item,"HrBlock",1);
  AddItemToHr(Item,Sign);
  Item.movable := 0;

  SendSysMessage(who,"Zablokowal"+ggm(who,3)+" przedmiot.",FONT_NORMAL,COLOR_GREEN);
  return 1;
  
endfunction

//-----------------------------------------------------------------------------

function GetHrBlockedItems(Sign)

  var Items := GetObjProperty(Sign,"HrBlockedItems");
  if(!Items)
    Items := {};
  endif
  return Items;

endfunction

//-----------------------------------------------------------------------------

function SetHrBlockedItems(Sign,Items)

  return SetObjProperty(Sign,"HrBlockedItems",Items);

endfunction

//-----------------------------------------------------------------------------

function CheckHrSpeech(who, byref text)

  if(text["chcialbym to unieruchomic"] or text["chcialabym to unieruchomic"])
    SelectBlockItem(who);
  elseif(text["chcialbym to odblokowac"] or text["chcialabym to odblokowac"])
    SelectEraseBlockItem(who);
  elseif(text["chcialbym postawic bezpieczny pojemnik"] or text["chcialabym postawic bezpieczny pojemnik"])
    CreateHrCont(who);
  elseif(text["chcialbym zniszczyc bezpieczny pojemnik"] or text["chcialabym zniszczyc bezpieczny pojemnik"])
    EraseHrCont(who);
  elseif(text["chcialbym postawic smietnik"] or text["chcialabym postawic smietnik"])
    CreateHrTrashBarrel(who);
  elseif(text["chcialbym zniszczyc smietnik"] or text["chcialabym zniszczyc smietnik"])
    DestroyHrTrashBarrel(who);
  elseif(text["chce wyrzucic"])
    HrKickFromHouse(who);
  endif

endfunction

//-----------------------------------------------------------------------------

function SelectBlockItem(who)

  var Sign := CheckNearHr(who.x,who.y,who.z,who.realm);
  if(!Sign)
    return;
  endif
  
  if(!CanControlHr(who,Sign))
    return;
  endif
  
  var Item := ObjTarget(who);

  if(!StandardItemCheck(who,Item))
    ReleaseItem(Item);
    NoLongerBusy(who);
    return;
  endif

  ReleaseItem(Item);
  NoLongerBusy(who);

  AddHrBlock(Item,Sign,who);

endfunction

//-----------------------------------------------------------------------------

function SelectEraseBlockItem(who)

  var Sign := CheckNearHr(who.x,who.y,who.z,who.realm);
  
  if(!Sign)
    return;
  endif

  if(!CanControlHr(who,Sign))
    return;
  endif

  var Item := ObjTarget(who);
  
  if(!StandardItemCheck(who,Item,NO_MOVABLE_CHECK))
    ReleaseItem(Item);
    NoLongerBusy(who);
    return;
  endif

  if(EraseHrBlock(Item,Sign))
    SendSysMessage(who,"Odblokowal"+ggm(who,3)+" przedmiot.",FONT_NORMAL,COLOR_GREEN);
  else
    SendSysMessage(who,"Ten przedmiot nie jest unieruchomiony.",FONT_NORMAL,COLOR_RED);
  endif
  
  ReleaseItem(Item);
  NoLongerBusy(who);
  
endfunction

//-----------------------------------------------------------------------------
// Friends
//-----------------------------------------------------------------------------

function SetHrFriends(Sign,Friends)

  SetObjProperty(Sign,"HrFriends",Friends);

endfunction

//-----------------------------------------------------------------------------

function AddHrVendor(Sign,me)

  var House := GetHr(GetHrCity(Sign),GetHrNr(Sign));

  if(House)
    var Doors := FindHrDoors(House);
    foreach Door in Doors
      Door.locked := 0;
      SetObjProperty(Door,"HrPublic",Sign.serial);
    endforeach
    SetObjProperty(Sign,"HrVendor",me.serial);
    return 1;
  endif
  
  return 0;

endfunction

//-----------------------------------------------------------------------------

function GetRhSecureNum(Sign)

  var HrCity   := GetHrCity(Sign);
  var HouseNr  := GetHrNr(Sign);
  var House    := GetHr(HrCity,HouseNr);
  var Rlm      := GetCityRealm(House.City);

  var c := 0;
  foreach Item in ListObjectsInBox(House.HouseXS, House.HouseYS, -127, House.HouseXE, House.HouseYE, 127, Rlm)
    if(CInt(GetObjProperty(Item, "Secured")))
      c := c + 1;
    endif
  endforeach
  SetObjProperty(Sign,"SecNum",c);

  return CInt(GetObjProperty(Sign,"SecNum"));

endfunction

//-----------------------------------------------------------------------------

function GetRhSecureMaxNum(Sign)

  var HrCity   := GetHrCity(Sign);
  var HouseNr  := GetHrNr(Sign);
  var House    := GetHr(HrCity,HouseNr);

  return CInt(House.NumSecure);

endfunction

//-----------------------------------------------------------------------------

function HrMoveItemsToEscrow(Owner, House, Sign)

  var Data := GetEscrowData(Owner);
  var HrItems := array;

  if(Data)
    HrItems := ListRentHouseItems(House,Sign,Owner);

    foreach item in HrItems
      MoveItemToEscrow(item, Data, 1);
    endforeach

    SetEscrowFilling(Data, 0);

  else
    LogError("[RentHouseResign]: "+Sign.serial+" + brak EscrowBox - nie ma Ownera! Critical Failure! Destroy it allll!");
    foreach item in HrItems
      DestroyItem(item);
    endforeach
  endif

endfunction

//-----------------------------------------------------------------------------

function ListRentHouseItems(House,Sign,Owner)

  var Bank  := FindBankBox(Owner);
  var HrItems := array;

  foreach Item in ListObjectsInBox(CInt(House.HouseXS),CInt(House.HouseYS),-127,CInt(House.HouseXE),CInt(House.HouseYE),127,Owner.realm)
    if(Item.isa(POLCLASS_ITEM))
      if(IsHrItem(Item,Sign))
        if(IsItemFromDeed(Item))
          var Deed := DestroyDeedItem(Item,Owner,Bank,1);
          HrItems.append(Deed);
        elseif(IsHrBlockedItem(Item))
          EraseHrBlock(Item,Sign);
          HrItems.append(Item);
        else
          DestroyItem(Item);
        endif
      endif
    endif
  endforeach

  return HrItems;

endfunction

//-----------------------------------------------------------------------------

function HrKickFromHouse(who)

  var Sign := CheckNearHr(who.x, who.y, who.z, who.realm);
  
  if(!Sign)
    return;
  endif

  if(GetOwnerSerial(Sign) != who.serial)
    return;
  endif

  SendSysMessage(who,"Kogo chcesz wyrzucic ?",FONT_NORMAL,COLOR_GREEN);

  var Victim := ObjTarget(who);

  var HrCity   := GetHrCity(Sign);
  var HouseNr  := GetHrNr(Sign);
  var House    := GetHr(HrCity,HouseNr);

  if(!StandardMobCheck(who,Victim))
    return;
  endif

  NoLongerBusy(Victim);

  if(Victim == who)
    return;
  endif

  if(!IsInHr(Victim.x, Victim.y, Victim.z, Victim.Realm, House))
    SendSysMessage(who,"Ta osoba nie znajduje sie w twoim domu.",FONT_NORMAL,COLOR_RED);
    return;
  endif

  var LocS;

  if(GetStrength(Victim) > GetStrength(who) and RandomInt(10) > 5)
    SendSysMessage(who,"Jestes za slaby by moc wykopac ta osobe.",FONT_NORMAL,COLOR_RED);
  else
    if(!GetStandZ(Sign.x, Sign.y, Sign.realm, Sign.z))
      while(1)
        LocS := FindStandableLocNear(Sign, 10);
        if(!IsInHr(LocS.x, LocS.y, LocS.z, LocS.Realm, House))
          break;
        endif
      endwhile
    endif
    if(MoveObject(Victim, LocS.x, LocS.y, LocS.z, Sign.realm))
      SendSysmessage(who,"Wykopal"+ggm(who,3)+" osobe "+victim.name+".",FONT_NORMAL,COLOR_GREEN);
      SendSysMessage(Victim,"Zostal"+ggm(Victim,3)+" wyrzucon"+ggm(Victim,6)+" przez "+who.name+".",FONT_NORMAL,COLOR_RED);
    else
      SendSysMessage(who,"Nie udalo Ci sie wykopac tej osoby.",FONT_NORMAL,COLOR_RED);
    endif
  endif

endfunction
