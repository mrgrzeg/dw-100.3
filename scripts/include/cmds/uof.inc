//-----------------------------------------------------------------------------

include "include/const/uof";

//-----------------------------------------------------------------------------

const UOF_PCID     := 0xEE;

//-----------------------------------------------------------------------------

function VerToStr(intVer)

  return (intVer&0xFFFF)+"."+((intVer>>16)&0xFFFF);

endfunction

//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------

function PcBuildProcInfo(byref Procs)

  var i,ProcId,fSize,fName,Found,lfName;
  var BadSizes := array(ADREN3_SIZE, ADREN3_SIZE1, ADREN3_SIZE2, VELOCITY_SIZE, NINJA_SIZE, GESTURE_SIZE, TEXTER_SIZE, COLORCONF_SIZE);
  var BadNames := array(ADREN3_NAME, ADREN3_NAME, ADREN3_NAME, VELOCITY_NAME, NINJA_NAME, GESTURE_NAME, TEXTER_NAME, COLORCONF_NAME);

  for(i:=1;i<=Procs.size();i:=i+1)
    ProcId := Procs[i][1];
    fSize  := Procs[i][2];
    fName  := Procs[i][3];
        
    GCheckNextPage(18);
    GTextLine(20, (67+(GNPCount()*20)),550,hex(CInt(ProcId)));
    
    Found := RetIndex(BadSizes, fSize);

    if(Found)
      GTextLine(100, (67+(GNPCount()*20)),1210,fSize);
      GTextLine(170, (67+(GNPCount()*20)),1210,BadNames[Found][1,5]);
    else
      GTextLine(100, (67+(GNPCount()*20)),550,fSize);
    endif

    Found  := 0;
    lfName := lower(fName);

    foreach Name in BadNames
      if(lfName[Name])
        Found := 1;
        break;
      endif
    endforeach

    if(Found)
      GTextLine(220,(67+(GNPCount()*20)),1210,fName);
    else
      GTextLine(220,(67+(GNPCount()*20)),570,fName);
    endif
  endfor

endfunction

//-----------------------------------------------------------------------------

function WaitForEv(byref ev, Time)

  ev := 0;

  var rTime := ReadGameClock();

  while(!ev)
    ev := wait_for_event(1);
    if(rTime + Time < ReadGameClock())
      break;
    endif
  endwhile

endfunction

//-----------------------------------------------------------------------------

function SendShpsGump(who, Nr)

  var Procs := UnPack(Elems[Nr][6]);

  GInit(20,20);

  GPage(0);
  GResPic(0,0,BKGND_BLACKSLAB,600,450);
  GTextLine(20,20,500,CUnixTime(Elems[Nr][1]) + " "+Elems[Nr][2]+"/"+Elems[Nr][3]+" D: "+Elems[Nr][4]+","+Elems[Nr][5]);
  GAddPageData(565,25,2650,2651,565,413,2648,2647);
  PcBuildProcInfo(Procs);
  GSend(who);

endfunction

//-----------------------------------------------------------------------------
