//-----------------------------------------------------------------------------

function CheckPlayerRef(byref who, whoSerial)

  if(!who)
    who := FindPlayerBySerial(whoSerial);
  endif

  if(!who)
    return 0;
  endif

  return 1;

endfunction

//-----------------------------------------------------------------------------

function WaitForLogon(who, maxTime := 5)

  var clockTime := ReadGameClock() + maxTime;

  while(clockTime > ReadGameClock())
    if(IsLoggedIn(who))
      return 1;
    endif
    sleep(1);
  endwhile

  return 0;

endfunction

//-----------------------------------------------------------------------------

function ValidatePlayerOn(who)

  if(IsStaff(who))
    foreach Player in EnumerateOnlineCharacters()
      if(IsStaffp(Player))
        SendSysMessage(Player,"StaffInc:" + GetCmdLevelName(who.cmdlevel) + " " + who.name + " ["+who.acctname+"]",FONT_NORMAL,COLOR_GREEN);
      endif
    endforeach
  endif

  if (IsStaffp(who))
    foreach Player in EnumerateOnlineCharacters()
      if(IsStaff(Player))
        SendSysMessage(who,"StaffOn:" + GetCmdLevelName(Player.cmdlevel) + " " + Player.name + " ["+Player.acctname+"]",FONT_NORMAL,COLOR_GREEN);
      endif
    endforeach
  endif

endfunction

//-----------------------------------------------------------------------------
function ValidatePlayerOff(who)

  if(IsStaff(who))
    foreach Player in EnumerateOnlineCharacters()
      if(IsStaffp(Player))
        SendSysMessage(Player,"StaffOff:" + GetCmdLevelName(who.cmdlevel) + " " + who.name + " ["+who.acctname+"]",FONT_NORMAL,COLOR_GREEN);
      endif
    endforeach
  endif
endfunction
//-----------------------------------------------------------------------------