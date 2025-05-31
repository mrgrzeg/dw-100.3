//-----------------------------------------------------------------------------

include "include/portals";

//-----------------------------------------------------------------------------
// Dane w Data: 
// 1 - odstep czasu pomiedzy specjalem
// 2 - od jakiego % hp zaczyna rzucac
// 3 - szansa na rzucenie w loopie wyrazona w procentach
// 4 - czas trwania rzucania w sekundach
// 5 - zasieg specjala
// 6 - poziom obra¿en wyra¿ony w % hp przeciwnika.
// 7 - type - dragon/immolation
// 8 - 1 - bezposrednie obrazenia czyli nie sprawdza resow, ar itp. jesli 0 lub brak to sprawdza czyli po staremu
// 9 - zabiera podany procent staminy; jesli 0 lub brak to nie zabiera staminy
// przyklad: Conflagration 60 50 10 8 50 95 dragon directdmg
//-----------------------------------------------------------------------------

function CheckConflagration()

  var HpPerc := GetHpPercent(me);

  if(HpPerc > Specs[NSPEC_CONFLAGRATION][2])
    return 0;
  endif

  var Chance := Specs[NSPEC_CONFLAGRATION][3] + RandomInt(CInt((50-HpPerc*100)/5));

  if(IsSpecOn(NSPECF_DEBUG))
    syslog("[Conflagration] "+GetName(me)+": Chance: "+Chance+"% Hp: "+CInt(HpPerc*100)+"% Limit: "+CInt(Specs[NSPEC_CONFLAGRATION][2]*100)+"%");
  endif

  if(!HaveChance(Chance))
    return 0;
  endif

  if(CheckNpcSpecDelay(NSPEC_CONFLAGRATION))
    return PerformConflagration();
  endif
  
  return 0;

endfunction

//-----------------------------------------------------------------------------

function PerformConflagration()

  var lSpec := Specs[NSPEC_CONFLAGRATION];
  var lAttackDelayed := SetDelayedAttack(me, lSpec[4]);

  if(IsSpecOn(NSPECF_DEBUG))
    syslog("[Conflagration] "+GetName(me)+": Perform with delay: "+lSpec[3]);
  endif

  var Secs := lSpec[4];

  SetNpcSpecPerforming(me, NSPEC_CONFLAGRATION, 150);

  var Mobs := ListSpecialDamageMobs(me, lSpec[5], LSDMF_ONLY_PLAYERS, "conflagration");

  foreach Mob in Mobs
    BlockPortals(Mob, CInt(lSpec[4]*3) );
  endforeach

  if(PerformConflagrtionAnimation(Secs))
    case(Specs[NSPEC_CONFLAGRATION][7])
      "dragon":
        SayUC(ToUnicode("*zionie smiertelnym ogniem*"));
      "immolation":
        SayUC(ToUnicode("*sprowadza zag³adê*"));
        PlaySoundEffect(me, SFX_IMMOLATION);
       "Cold":
        SayUC(ToUnicode("*sprowadza lodow± zag³adê*"));
        PlaySoundEffect(me, SFX_IMMOLATION);
       "Electric": 
        SayUC(ToUnicode("*sprowadza burzê*"));
        PlaySoundEffect(me, SFX_IMMOLATION);
        "Mrok": 
        SayUC(ToUnicode("*sprowadza mrok*"));
        PlaySoundEffect(me, SFX_IMMOLATION);
    endcase
    
    PerformAction(me, ANIM_CAST_AREA);
    PlaySoundEffect(me, SFX_FIRE1);
    PlaySoundEffect(me, SFX_FIRE2);

    Mobs := ListSpecialDamageMobs(me, lSpec[5], 0, "conflagration");

    start_script(":specattack:conflagration", array(me, Mobs, lSpec));
  endif

  if(lAttackDelayed)
    EraseDelayedAttack(me);
  endif

  SetUsedSepcs(me, NSPEC_CONFLAGRATION);
  EraseNpcSpecPerforming(me, NSPEC_CONFLAGRATION);
  DoNpcSpecDelay(me, NSPEC_CONFLAGRATION);
  return 1;

endfunction

//-----------------------------------------------------------------------------

function PerformConflagrtionAnimation(Secs)
  
  var i;

  for(i:=1;i<=Secs;i+=1)
    if(i == 1)

      case(Specs[NSPEC_CONFLAGRATION][7])
        "dragon":
          SayUC(ToUnicode("*rozdziawia paszcze*"));
        "immolation":
          SayUC(ToUnicode("*wytê¿a siê*"));
          "Cold":
          SayUC(ToUnicode("*Przywo³uje Zamieæ"));
          "Electric":
          SayUC(ToUnicode("*Przywo³uje burzê*"));
          "Mrok":
          SayUC(ToUnicode("*Przywo³uje mroczn± falê*"));
          
      endcase

    elseif(i == 4)

      case(Specs[NSPEC_CONFLAGRATION][7])
        "dragon":
          SayUC(ToUnicode("*z pyska buchaj¹ p³omienie*"));
        "immolation":
          SayUC(ToUnicode("*bucha ogniem*"));
      endcase 
      PlayStationaryEff(me.x, me.y-1, me.z, FX_FLAMESTRIKE, 0x10, 0x10, 1, me.realm);
      PlayStationaryEff(me.x, me.y+1, me.z, FX_FLAMESTRIKE, 0x10, 0x10, 1, me.realm);
      PlayStationaryEff(me.x-1, me.y, me.z, FX_FLAMESTRIKE, 0x10, 0x10, 1, me.realm);
      PlayStationaryEff(me.x+1, me.y, me.z, FX_FLAMESTRIKE, 0x10, 0x10, 1, me.realm);
      PlaySoundEffect(me, SFX_FIRE_STRIKE);
      
      foreach Mob in ListSpecialDamageMobs(me, 2, 0, "conflagration")
        PlayPcEffect(Mob, FX_FLAMESTRIKE, 0, 0x10, 0x10, 1);
        PlaySoundEffect(Mob, SFX_FIRE_STRIKE);
        DoMagicDamage(Mob, me, CInt(GetHp(Mob)*0.3), 9, DMGTYPE_FIRE);
      endforeach
    endif

    if(Modulo(i, 2) == 0)
      PerformAction(me, ANIM_CAST_NPC);
      PlaySoundEffect(me, SFX_FIRE1);
    else
      PlaySoundEffect(me, SFX_FIRE2);
    endif

    sleepms(500);
    
    if(!IsNpcSpecPerforming(me, NSPEC_CONFLAGRATION))
      PerformNpcSpecialBreak();
      return 0;
    endif
    
    sleepms(500);
    
    if(!IsNpcSpecPerforming(me, NSPEC_CONFLAGRATION))
      PerformNpcSpecialBreak();
      return 0;
    endif
    
  endfor

  return 1;

endfunction

//-----------------------------------------------------------------------------
