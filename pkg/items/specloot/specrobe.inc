
function EquipLegendaryRobe(who)

  PlayObjectCenteredEffect( who, FX_SPARK_EFFECT, 7, 0x10 );
  PlaySoundEffect( who, SFX_SPELL_GREATER_HEAL );
  var Duration := 86400; //24h
  DoMod(who, MOD_FIRERESIST, 1, Duration);
  DoMod(who, MOD_COLDRESIST, 1, Duration);
  DoMod(who, MOD_LIGHTRESIST, 1, Duration);

endfunction

//-----------------------------------------------------------------------------

function UnEquipLegendaryRobe(who)

  UnDoMod(who, MOD_FIRERESIST);
  UnDoMod(who, MOD_COLDRESIST);
  UnDoMod(who, MOD_LIGHTRESIST);

endfunction

//-----------------------------------------------------------------------------

function ApplyWeaponBonus(who, item, TimeToDestroy, ItemCfg)

  var ArtModArr := SplitWords(ItemCfg[item.objtype].ArtMod);
  var MagicItemArray := {};
  MagicItemArray.append(13);
  MagicItemArray.append(CInt(ArtModArr[1])); //numer skila pobiera z samej broni z artmoda
  MagicItemArray.append(CInt(ArtModArr[2])); //tak samo moc
  MagicItemArray.append(TimeToDestroy);
  MagicItemArray.append(TimeToDestroy);

  SetMagicMods(item, MagicItemArray);
  IdentifyMagicItem(who,item);

  SetItemPoisoned(who, item, 8, TimeToDestroy);

endfunction