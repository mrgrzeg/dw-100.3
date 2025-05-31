//Aby strzałki działały prawidłowo to obie funkcje tworząca i niszcząca muszą znajdować się w tym samym pliku
//a jako serial celu można podać serial gracza któremu strzałka jest pokazywana
//-----------------------------------------------------------------------------
function CreateQuestArrow(who, TargX, TargY, TargSerial)

  var SendArrow := BuildPacketQuestArrow(who, TargX, TargY, TargSerial);
  SendArrow.SetInt8(1,1);
  SendArrow.SendPacket(who);

endfunction

//-----------------------------------------------------------------------------

function KillQuestArrow(who, TargX, TargY, TargSerial)

  var SendArrow := BuildPacketQuestArrow(who, TargX, TargY, TargSerial);
  SendArrow.SetInt8(1,0);
  SendArrow.SendPacket(who);

endfunction

//-----------------------------------------------------------------------------

function BuildPacketQuestArrow(who, TargX, TargY, TargSerial)

  var Client := who.clientver_detail;
  var PacketSize := 6;

  var SendArrow := CreatePacket(0xBA, PacketSize);
  SendArrow.SetInt16(2,TargX);
  SendArrow.SetInt16(4,TargY);
  
  Sleepms(50);
  return SendArrow;

endfunction