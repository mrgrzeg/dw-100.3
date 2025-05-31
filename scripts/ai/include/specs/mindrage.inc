//-----------------------------------------------------------------------------

include "include/portals";

//-----------------------------------------------------------------------------
// Dane w Data: 
// 1 - odstep czasu pomiedzy specjalem
// 2 - od jakiego % hp zaczyna rzucac
// 3 - szansa na rzucenie w loopie wyrazona w procentach
// 4 - czas trwania rzucania w sekundach
// 5 - zasieg specjala
// 6 - poziom obra¿en wyra¿ony w % hp przeciwnika
// 7 - 1 - bezposrednie obrazenia czyli nie sprawdza resow, ar itp. jesli 0 lub brak to sprawdza czyli po staremu
// 8 - zabiera podany procent staminy; jesli 0 lub brak to nie zabiera staminy
// przyklad: MindRage 60 50 10 8 50
//-----------------------------------------------------------------------------

function CheckMindRage()

  var HpPerc := GetHpPercent(me);

  if(HpPerc > Specs[NSPEC_MINDRAGE][2])
    return 0;
  endif

  var Chance := Specs[NSPEC_MINDRAGE][3] + RandomInt(CInt((50-HpPerc*100)/5));

  if(IsSpecOn(NSPECF_DEBUG))
    syslog("[MindRage] "+GetName(me)+": Chance: "+Chance+"% Hp: "+CInt(HpPerc*100)+"% Limit: "+CInt(Specs[NSPEC_MINDRAGE][2]*100)+"%");
  endif

  if(!HaveChance(Chance))
    return 0;
  endif

  if(CheckNpcSpecDelay(NSPEC_MINDRAGE))
    return PerformMindRage();
  endif
  
  return 0;

endfunction

//-----------------------------------------------------------------------------

function PerformMindRageAnimation(Secs)

  var i;

  for(i:=1;i<=Secs;i+=1)
    case(i)
      1: SayUC(ToUnicode("*skupia umys³*"));
      4: SayUC(ToUnicode("*wytê¿a swój zmys³*"));
    endcase

    if(Modulo(i, 2) == 0)
      PerformAction(me, 10);
      PlaySoundEffect(me, SFX_FOCUSATTACK);
    else
      PerformAction(me, 12);
      PlaySoundEffect(me, SFX_FEINT);
    endif

    sleepms(500);
    
    if(!IsNpcSpecPerforming(me, NSPEC_MINDRAGE))
      PerformNpcSpecialBreak();
      return 0;
    endif
    
    sleepms(500);
    
    if(!IsNpcSpecPerforming(me, NSPEC_MINDRAGE))
      PerformNpcSpecialBreak();
      return 0;
    endif
    
  endfor

  return 1;

endfunction

//-----------------------------------------------------------------------------

function PerformMindRage()

  var lAttackDelayed := SetDelayedAttack(me, Specs[NSPEC_MINDRAGE][4]+2);

  if(IsSpecOn(NSPECF_DEBUG))
    syslog("[MindRage] "+GetName(me)+": Perform with delay: "+Specs[NSPEC_MINDRAGE][3]);
  endif

  var Secs := Specs[NSPEC_MINDRAGE][4];

  SetNpcSpecPerforming(me, NSPEC_MINDRAGE, 2);

  var Mobs := ListSpecialDamageMobs(me, Specs[NSPEC_MINDRAGE][5], LSDMF_ONLY_PLAYERS, "MindRage");

  foreach Mob in Mobs
    BlockPortals(Mob, CInt(Specs[NSPEC_MINDRAGE][4]*1.5) );
  endforeach

  Mobs := ListSpecialDamageMobs(me, Specs[NSPEC_MINDRAGE][5], 0, "MindRage");

  if(PerformMindRageAnimation(Secs))
    SayUC(ToUnicode("*wytwarza falê zag³ady*"));

    PerformAction(me, 18);
    PlaySoundEffect(me, SFX_SMOKE_BOMB);

    DealMindRageDamage(Mobs);
    PlayMindRageEffect(Mobs, Specs[NSPEC_MINDRAGE][5], "MindRage");
  endif

  if(lAttackDelayed)
    EraseDelayedAttack(me);
  endif

  SetUsedSepcs(me, NSPEC_MINDRAGE);
  EraseNpcSpecPerforming(me, NSPEC_MINDRAGE);
  return 1;

endfunction

//-----------------------------------------------------------------------------

function DealMindRageDamage(byref Mobs)

  foreach Mob in Mobs
  
    if(IsSpecOn(NSPECF_DEBUG))
      syslog("[MindRage] "+GetName(me)+": Perform on mobile: "+GetName(Mob));
    endif
    
    var DmgPerc := Specs[NSPEC_MINDRAGE][6];
    var StamDmg := __IfElse(!Specs[NSPEC_MINDRAGE][8], 95, Specs[NSPEC_MINDRAGE][8]);

    if(IsInSpecArea(GetSpecAreaIdx(Mob), GetSpecAreaIdx(me)))
      if(!Specs[NSPEC_MINDRAGE][7])
        DealDamage(Mob, CInt(PercentInterpolate(GetHp(Mob),DmgPerc)), me);
      else
        ApplyRawDamage(Mob, CInt(PercentInterpolate(GetHp(Mob),DmgPerc)));
      endif
      DealManaDamage(Mob, CInt(PercentInterpolate(GetMana(Mob),DmgPerc)), me);
      if(Specs[NSPEC_MINDRAGE][8])
        DealStaminaDamage(Mob, StamDmg, me);
      endif
    else
      if(!Specs[NSPEC_MINDRAGE][7])
        DealDamage(Mob, CInt(PercentInterpolate(GetMaxHp(Mob),DmgPerc)), me); 
      else
        ApplyRawDamage(Mob, CInt(PercentInterpolate(GetHp(Mob),DmgPerc)));
      endif
    endif
    PlayPcEffect(Mob, FX_SPARK_EFFECT, 1244, 16, 16, 4);
    PlaySoundEffect(Mob, SFX_THUNDER_STORM);
  endforeach

endfunction

//-----------------------------------------------------------------------------

function PlayMindRageEffect(byref Mobs, Range, SpecName)

  var SpecArea := NpcInSpecArea(me, SpecName);
  var Locs := 0;
  
  foreach Mob in Mobs
    PlayPcEffect(Mob, FX_EXPLODE_2, 1321, 24, 24, 4);
    PlayLightningBoltEffect(Mob);
  endforeach

  if(!SpecArea)
    Locs := GetRandomLocsNear(me, Range);
  else
    Locs := GetSpecAreaRandomLocs(SpecArea, me.z);
  endif

  foreach Loc in Locs
    PlayPcStEffect(Loc.x, Loc.y, Loc.z, Loc.realm, FX_EXPLODE_2, 1321, 24, 24, 4);
  endforeach

endfunction

//-----------------------------------------------------------------------------
