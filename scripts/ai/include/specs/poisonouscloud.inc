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
// przyklad: Plague 60 50 10 8 50 95 immolation
//-----------------------------------------------------------------------------

function CheckPoisonousCloud()

  var HpPerc := GetHpPercent(me);

  if(HpPerc > Specs[NSPEC_POISONOUSCLOUD][2])
    return 0;
  endif

  var Chance := Specs[NSPEC_POISONOUSCLOUD][3] + RandomInt(CInt((50-HpPerc*100)/5));

  if(IsSpecOn(NSPECF_DEBUG))
    syslog("[PoisonousCloud] "+GetName(me)+": Chance: "+Chance+"% Hp: "+CInt(HpPerc*100)+"% Limit: "+CInt(Specs[NSPEC_POISONOUSCLOUD][2]*100)+"%");
  endif

  if(!HaveChance(Chance))
    return 0;
  endif

  if(CheckNpcSpecDelay(NSPEC_POISONOUSCLOUD))
    return PerformPoisonousCloud();
  endif
  
  return 0;

endfunction

//-----------------------------------------------------------------------------

function PerformPoisonousCloud()

  var lSpec := Specs[NSPEC_POISONOUSCLOUD];
  var lAttackDelayed := SetDelayedAttack(me, lSpec[4]);

  if(IsSpecOn(NSPECF_DEBUG))
    syslog("[PoisonousCloud] "+GetName(me)+": Perform with delay: "+lSpec[3]);
  endif

  var Secs := lSpec[4];

  SetNpcSpecPerforming(me, NSPEC_POISONOUSCLOUD, 3);

  var Mobs := ListSpecialDamageMobs(me, lSpec[5], LSDMF_ONLY_PLAYERS, "PoisonousCloud");

  foreach Mob in Mobs
    BlockPortals(Mob, CInt(lSpec[4]*3) );
  endforeach

  if(PerformPoisonousCloudAnimation(Secs))
    case(Specs[NSPEC_POISONOUSCLOUD][7])
      "dragon":
        SayUC(ToUnicode("*pierdzi*"));
      "immolation":
        SayUC(ToUnicode("*pierdzi*"));
        PlaySoundEffect(me, SFX_IMMOLATION);
    endcase
    
    PerformAction(me, ANIM_CAST_AREA);
    PlaySoundEffect(me, SFX_FIRE1);
    PlaySoundEffect(me, SFX_FIRE2);

    Mobs := ListSpecialDamageMobs(me, lSpec[5], 0, "PoisonousCloud");

    start_script(":specattack:poisonouscloud", array(me, Mobs, lSpec));
  endif

  if(lAttackDelayed)
    EraseDelayedAttack(me);
  endif

  SetUsedSepcs(me, NSPEC_POISONOUSCLOUD);
  EraseNpcSpecPerforming(me, NSPEC_POISONOUSCLOUD);
  DoNpcSpecDelay(me, NSPEC_POISONOUSCLOUD);
  return 1;

endfunction

//-----------------------------------------------------------------------------

function PerformPoisonousCloudAnimation(Secs)
  
  var i;

  for(i:=1;i<=Secs;i+=1)
    if(i == 1)

      case(Specs[NSPEC_POISONOUSCLOUD][7])
        "dragon":
          SayUC(ToUnicode("*rozdziawia paszcze*"));
        "immolation":
          SayUC(ToUnicode("*przewraca mu siê w brzuchu*"));
      endcase

    elseif(i == 4)

      foreach Mob in ListSpecialDamageMobs(me, 2, 0, "PoisonousCloud")
        PlayPcEffect(Mob, FX_SMOKE, 68, 0x10, 0x10, 4);
        PlaySoundEffect(Mob, SFX_FART);
        //var SpecDmg := __IfElse(!Specs[NSPEC_POISONOUSCLOUD][6], CInt(GetHp(Mob)*0.3), CInt(PercentInterpolate(GetHp(Mob), Specs[NSPEC_POISONOUSCLOUD][6])));
        //var StamDmg := __IfElse(!Spec[9], 95, Spec[9]);
        //if(!Specs[NSPEC_POISONOUSCLOUD][8])
        DoMagicDamage(Mob, me, CInt(GetHp(Mob)*0.3), 14, DMGTYPE_POISON);
        //else
        //  ApplyRawDamage(Mob, SpecDmg);
        //endif
        //DealStaminaDamage(Mob, StamDmg, me);
      endforeach
    endif

    if(Modulo(i, 2) == 0)
      PerformAction(me, ANIM_CAST_NPC);
      PlaySoundEffect(me, SFX_GRUMBLE);
    else
      PlaySoundEffect(me, SFX_GRUMBLE);
    endif

    sleepms(500);
    
    if(!IsNpcSpecPerforming(me, NSPEC_POISONOUSCLOUD))
      PerformNpcSpecialBreak();
      return 0;
    endif
    
    sleepms(500);
    
    if(!IsNpcSpecPerforming(me, NSPEC_POISONOUSCLOUD))
      PerformNpcSpecialBreak();
      return 0;
    endif
    
  endfor

  return 1;

endfunction

//-----------------------------------------------------------------------------
