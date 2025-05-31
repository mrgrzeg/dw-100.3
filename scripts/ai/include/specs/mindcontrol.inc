//-----------------------------------------------------------------------------

include "include/portals";

//-----------------------------------------------------------------------------
// Dane w Data: 
// 1 - odstep czasu pomiedzy specjalem
// 2 - od jakiego % hp zaczyna rzucac
// 3 - szansa na rzucenie w loopie wyrazona w procentach
// 4 - czas trwania rzucania w sekundach
// 5 - sposob uzycia kontroli umyslu w razie sukcesu, czyli co ma zrobic graczowi ?
// przyklad: MindControl 60 50 10 10 kill
//-----------------------------------------------------------------------------

function CheckMindControl()

  var HpPerc := GetHpPercent(me);

  if(HpPerc > Specs[NSPEC_MINDCONTROL][2])
    return 0;
  endif

  var Chance := Specs[NSPEC_MINDCONTROL][3] + RandomInt(CInt((50-HpPerc*100)/5));

  if(IsSpecOn(NSPECF_DEBUG))
    syslog("[MindControl] "+GetName(me)+": Chance: "+Chance+"% Hp: "+CInt(HpPerc*100)+"% Limit: "+CInt(Specs[NSPEC_MINDCONTROL][2]*100)+"% Delay: "+GetNpcSpecDelayTime(NSPEC_MINDCONTROL)+"s");
  endif

  if(!HaveChance(Chance))
    return 0;
  endif

  if(CheckNpcSpecDelay(NSPEC_MINDCONTROL))
    return PerformMindControl();
  endif
  
  return 0;

endfunction

//-----------------------------------------------------------------------------

function PerformMindControl()

  if(IsSpecOn(NSPECF_DEBUG))
    syslog("[MindControl] "+GetName(me)+": Perform with delay: "+Specs[NSPEC_MINDCONTROL][3]);
  endif

  var Player;

  if(IsPlayer(me.opponent) and HaveChance(70) and Distance(me, me.opponent) <= 10)
    Player := me.opponent;
  endif

  var Mobs := ListSpecialDamageMobs(me, 10, LSDMF_ONLY_PLAYERS);

  if(!Player)
    Player := Mobs[RandomInt(Mobs.size())+1];
  endif

  if(!Player)
    LogError("[MindControl] "+me.npctemplate+"/"+me.serial+" "+GetLocString(me)+" chcial pocisnac kontrole umyslu, ale zabraklo gracza! wth?");
    return;
  endif

  var PlayerSerial := Player.serial;
  var Secs := Specs[NSPEC_MINDCONTROL][4] * 1000; //zamiana sekund na milisekundy
  var TimeToBreakSpecFromPlayer := 15; //sekund, gracz moze zbic specjal zanim zostanie sparalizowany

  SetInvulToAttack(me);
  SetNpcSpecPerforming(me, NSPEC_MINDCONTROL, 150); //dmg do zbicia przez gracza

  foreach Mob in Mobs
    BlockPortals(Mob, CInt(Specs[NSPEC_MINDCONTROL][3]*1.5) );
  endforeach

  if(PerformMindControlAnimation(TimeToBreakSpecFromPlayer, Player, PlayerSerial)) //czas na zbicie specjala

    SetNpcSpecPerforming(me, NSPEC_MINDCONTROL, 210); //dmg do zbicia przez moba

    case(Specs[NSPEC_MINDCONTROL][5]) 
      "kill":
        SayUC(ToUnicode("Haha!"));
        PerformAction(me, 19);
        PlaySoundEffect(me, SFX_CONFIDENCE);
        sleepms(800);        
        SayUC(ToUnicode("jestes moj"+ggm(Player,4)+" "+GetName(Player)+", zginiesz!"));
        TurnToward(Player);

        PerformAction(me, 18);

        PlayPcStEffect(Player.x, Player.y, Player.z, Player.realm, FX_FLAMESTRIKE, 1244, 16, 16, 4);
        PlayPcStEffect(Player.x, Player.y, Player.z, Player.realm, FX_EXPLODE_2, 1244, 16, 16, 4);

        var Count := 0;
        var Dmg := 12;

        while(!Player.dead and me and Secs > 0)
          if(!Player)
            Player := FindPlayerBySerial(PlayerSerial);
            if(!Player)
              break;
            endif
          endif

          if(Modulo(Count, 4) == 0)
            DealDamage(Player, Dmg);
            PlayPcEffect(Player, FX_MARK_EFFECT, 37, 0x10, 0x10, 2);
            Dmg := Min(30, Dmg+4+RandomInt(3));
          endif

          Count += 1;
          sleepms(500);
          
          if(!IsNpcSpecPerforming(me, NSPEC_MINDCONTROL))
            PerformNpcSpecialBreak();
            break;
          endif

          if(Modulo(Count, 2) == 0)
            PerformAction(me, 16);
            PlaySoundEffect(me, SFX_FOCUSATTACK);
          else
            PerformAction(me, 12);
            PlaySoundEffect(me, SFX_FEINT);
          endif
          Secs -= 500; //zmniejsza czas na zbicie specjala o 500 ms.
        endwhile
        
        if(Player.dead)
          SayUC(ToUnicode("Hahah! Myslal"+ggm(Player, 3)+" ze uciekniesz "+GetName(Player)+"?! hahah"));
        endif

        sleepms(500);
    endcase

  endif

  if(Player)
    UndoMod(Player, MOD_PARA);
  endif

  EraseInvulToAttack(me);

  SetUsedSepcs(me, NSPEC_MINDCONTROL);
  EraseNpcSpecPerforming(me, NSPEC_MINDCONTROL);
  return 1;

endfunction

//-----------------------------------------------------------------------------

function PerformMindControlAnimation(Secs, byref Player, PlayerSerial)

  var i;

  for(i:=1;i<=Secs;i+=1)
    case(i)
      1: SayUC(ToUnicode("*pr�buje przej�� kontrol� nad "+GetName(Player)+"*"));
      7: SayUC(ToUnicode("*prawie zaw�adn�� umys�em*"));
    endcase

    if(Modulo(i, 2) == 0)
      PerformAction(me, 16);
      PlaySoundEffect(me, SFX_FOCUSATTACK);
    else
      PerformAction(me, 12);
      PlaySoundEffect(me, SFX_FEINT);
    endif

    sleepms(500);
    
    if(!IsNpcSpecPerforming(me, NSPEC_MINDCONTROL))
      PerformNpcSpecialBreak();
      return 0;
    endif
    
    sleepms(500);
    
    if(!IsNpcSpecPerforming(me, NSPEC_MINDCONTROL))
      PerformNpcSpecialBreak();
      return 0;
    endif

    if(!Player)
      Player := FindPlayerBySerial(PlayerSerial);
      if(!Player)
        return 0;
      endif
    endif
    
  endfor

  DoMod(Player, MOD_PARA, 2, MODDUR_INFINITE);
  start_script(":magiceffects:undo_depend", array(Player, MOD_PARA, me));
  TurnToward(Player);

  return 1;

endfunction

//-----------------------------------------------------------------------------
